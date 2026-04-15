// Fill out your copyright notice in the Description page of Project Settings.


#include "VehicleBase.h"

#include "AIControllerBase.h"
#include "GrenadeWidget.h"
#include "HealthComponent.h"
#include "WeaponBase.h"
#include "Components/CapsuleComponent.h"
#include "Core/CharacterBase.h"
#include "Engine/DamageEvents.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Net/UnrealNetwork.h"

// Sets default values
AVehicleBase::AVehicleBase()
{
 	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	VehicleSkeletalMesh = CreateDefaultSubobject<USkeletalMeshComponent>("Mesh");
	SetRootComponent(GetVehicleMesh());
	HealthComponent = CreateDefaultSubobject<UHealthComponent>("HealthComponent");
	ExitPoint = CreateDefaultSubobject<USceneComponent>("ExitPoint");
	ExitPoint->SetupAttachment(GetVehicleMesh());
}

// Called when the game starts or when spawned
void AVehicleBase::BeginPlay()
{
	Super::BeginPlay();
	HealthComponent->OnHealthUpdate.AddDynamic(this, &AVehicleBase::OnHealthUpdated);
	HealthComponent->OnHealthDepleted.AddDynamic(this, &AVehicleBase::OnHealthDepleted);
	UpdateDamageState();
}

// Called every frame
void AVehicleBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

// Called to bind functionality to input
void AVehicleBase::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}

void AVehicleBase::SpawnDefaultControllerWithTeam(uint8 TeamId)
{
	SpawnDefaultController();
	if (AAIControllerBase* AIC = Cast<AAIControllerBase>(GetController()))
	{
		AAIControllerBase* PilotAIC = Cast<AAIControllerBase>(PilotController);
		AIC->SetSmartObject(PilotAIC->SmartObject);
		AIC->SetGenericTeamId(TeamId);
	}
}

void AVehicleBase::OnHealthUpdated(UHealthComponent* HealthComp)
{
	UpdateDamageState();
}

void AVehicleBase::OnHealthDepleted(float Damage, FVector Force, FVector HitLocation, FName HitBoneName,
	AController* EventInstigator, AActor* DamageCauser)
{
	if (Pilot) Pilot->CustomTakeDamage(5000, FVector(0,0,0), EventInstigator, DamageCauser);
	SetIsDestroyed(true);
}

void AVehicleBase::UpdateDamageState()
{
	const float Health = HealthComponent->GetHealth();
	const float MaxHealth = HealthComponent->GetMaxHealth();
	if (Health <= MaxHealth && Health > MaxHealth * 0.66)
	{
		SetDamageState(Healthy);
	} else if (Health <= MaxHealth * 0.66 && Health > MaxHealth * 0.33)
	{
		SetDamageState(Damaged);
	} else if (Health > 0)
	{
		SetDamageState(Critical);
	} else
	{
		SetDamageState(EDamageState::Destroyed);
	}
}

void AVehicleBase::SetDamageState_Implementation(EDamageState NewDamageState)
{
	if (DamageState != NewDamageState)
	{
		OnDamageStateChanged.Broadcast(NewDamageState);
		DamageState = NewDamageState;
	}
}

void AVehicleBase::SetIsDestroyed(bool bNewIsDestroyed)
{
	bIsDestroyed = bNewIsDestroyed;
}

//This is a multicast function
void AVehicleBase::NotifyRestarted()
{
	Super::NotifyRestarted();
	// if (IsLocallyControlled())
	// {
	// 	SpawnHUD();
	// 	SpawnControls();
	// }
	Client_PossessedBy(nullptr);
}

//Server-only RPC
void AVehicleBase::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);
	//Client_PossessedBy(NewController);
}

void AVehicleBase::Client_PossessedBy_Implementation(AController* NewController)
{
	if (IsLocallyViewed())
	{
		SpawnHUD();
		SpawnControls();
	}
}

void AVehicleBase::UnPossessed()
{
	//Bugs out all clients
	Client_Unpossessed();
	if (IsLocallyViewed())
	{
		RemoveHUD();
		RemoveControls();
	}
	Super::UnPossessed();
}

void AVehicleBase::Client_Unpossessed_Implementation()
{
	if (IsLocallyViewed())
	{
		RemoveHUD();
		RemoveControls();
	}
}


void AVehicleBase::Enter_Implementation(ACharacterBase* NewPilot)
{
	if (bIsDestroyed || Pilot) return;
	SetPilotToPossess(NewPilot);
	if (IsPlayerControlled())
	{
		CL_Enter(NewPilot);
	}
	
	OnEntered.Broadcast();
}

void AVehicleBase::CL_Enter_Implementation(ACharacterBase* NewPilot)
{
	// if (IsLocallyControlled())
	// {
	// 	SpawnHUD();
	// 	SpawnControls();
	// }
}

void AVehicleBase::SetPilotToPossess_Implementation(ACharacterBase* NewPilot)
{
	Pilot = NewPilot;
	PilotController = Pilot->GetController();
	SetOwner(PilotController);
	Pilot->bIsInVehicle = true;
	Pilot->OccupiedVehicle = this;
	AttachPilot();
	Pilot->OnKilled.AddUniqueDynamic(this, &AVehicleBase::OnPilotKilled);
	PilotController->UnPossess();
	if (Cast<APlayerController>(PilotController))
	{
		PilotController->Possess(this);
	} else
	{
		SpawnDefaultControllerWithTeam(Pilot->TeamId);
	}
}

void AVehicleBase::AttachPilot_Implementation()
{
	if (IsValid(Pilot))
	{
		Pilot->GetCharacterMovement()->SetMovementMode(EMovementMode::MOVE_None);
		Pilot->GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
		Pilot->GetCapsuleComponent()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Vehicle, ECR_Ignore);
		FName Socket = NAME_None;
		if (GetVehicleMesh()->DoesSocketExist("Seat")) Socket = "Seat";
		if (Pilot->EquippedWeapon) Pilot->EquippedWeapon->SetActorHiddenInGame(true);
		Pilot->AttachToComponent(GetVehicleMesh(), FAttachmentTransformRules::SnapToTargetNotIncludingScale, Socket);
	}
}

void AVehicleBase::Exit_Implementation()
{
	if (IsPlayerControlled())
		CL_Exit();
	ResetPilot();
	OnExited.Broadcast();
}

void AVehicleBase::CL_Exit_Implementation()
{
	if (IsLocallyControlled())
	{
		RemoveControls();
		RemoveHUD();
	}
}

void AVehicleBase::ResetPilot_Implementation()
{
	if (!Pilot) return;
	if (GetController())
	{
		GetController()->UnPossess();
		if (Pilot->GetHealthComponent()->IsAlive())
			PilotController->Possess(Pilot);
	}
	Pilot->bIsInVehicle = false;
	Pilot->OccupiedVehicle = nullptr;
	Pilot->OnKilled.RemoveDynamic(this, &AVehicleBase::OnPilotKilled);
	DetachPilot();
	SetOwner(nullptr);
	Pilot = nullptr;
	PilotController = nullptr;
}

void AVehicleBase::DetachPilot_Implementation()
{
	if (IsValid(Pilot))
	{
		UE_LOG(LogTemp, Warning, TEXT("Called detach pilot on %d"), GetRemoteRole());
		Pilot->GetCharacterMovement()->SetMovementMode(EMovementMode::MOVE_NavWalking);
		Pilot->DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
		Pilot->SetActorTransform(ExitPoint->GetComponentTransform(), false);
		Pilot->GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		Pilot->GetCapsuleComponent()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Vehicle, ECR_Block);
		if (Pilot->EquippedWeapon) Pilot->EquippedWeapon->SetActorHiddenInGame(false);

	} else
	{
		UE_LOG(LogTemp, Warning, TEXT("Invalid pilot for %s"), *GetName());
	}
}




// void AVehicleBase::SpawnHUD_Implementation()
// {
// 	if (!VehicleHUD)
// 	{
// 		VehicleHUD = CreateWidget<UUserWidget>(GetController<APlayerController>(), VehicleHUDClass);
// 		if (VehicleHUD)
// 			VehicleHUD->AddToPlayerScreen();	
// 	}
// }
//
// void AVehicleBase::RemoveHUD_Implementation()
// {
// 	if (VehicleHUD)
// 	{
// 		UE_LOG(LogTemp, Warning, TEXT("Removing HUD"));
// 		VehicleHUD->RemoveFromRoot();
// 		VehicleHUD = nullptr;
// 	}
// }


FVector AVehicleBase::GetNavAgentLocation() const
{
	return GetActorLocation() - FVector(0.f, 0.f, AgentZOffset);
}

void AVehicleBase::OnPilotKilled(ACharacterBase* Character, AController* Killer, AActor* Causer)
{
	Exit();
}

void AVehicleBase::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(AVehicleBase, Pilot);
}

USkeletalMeshComponent* AVehicleBase::GetVehicleMesh()
{
	return VehicleSkeletalMesh;
}

float AVehicleBase::CustomTakeDamage_Implementation(float Damage, FVector Force, AController* EventInstigator, AActor* DamageCauser)
{
	HealthComponent->TakeDamage(Damage, Force, FVector(0,0,0), NAME_None, EventInstigator, DamageCauser);
	return Damage;
}

float AVehicleBase::CustomTakePointDamage_Implementation(float Damage, FVector Direction, const FHitResult& HitInfo, float Force, AController* EventInstigator, AActor* DamageCauser)
{
	HealthComponent->TakeDamage(Damage, Direction*Force, HitInfo.Location, HitInfo.BoneName, EventInstigator, DamageCauser);
	return Damage;
}

float AVehicleBase::CustomTakeRadialDamage_Implementation(FVector Origin, float Radius, float Force, const FHitResult& HitInfo, FRadialDamageEvent const& RadialDamageEvent, float MinimumRadius, AController* EventInstigator, AActor* DamageCauser)
{

	HealthComponent->TakeDamage(RadialDamageEvent.Params.BaseDamage, (GetActorLocation() - RadialDamageEvent.Origin).GetSafeNormal()*Force, this->GetActorLocation(), NAME_None, EventInstigator, DamageCauser);
	return RadialDamageEvent.Params.BaseDamage;
}

UHealthComponent* AVehicleBase::GetHealthComponent_Implementation()
{
	return HealthComponent;
}

void AVehicleBase::OnInteract_Implementation(ACharacterBase* Character)
{
	IInteractableInterface::OnInteract_Implementation(Character);

	if (bIsDestroyed) return;
}
