// Fill out your copyright notice in the Description page of Project Settings.


#include "VehicleBase.h"

#include "AIControllerBase.h"
#include "GrenadeWidget.h"
#include "HealthComponent.h"
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

	VehicleMesh = CreateDefaultSubobject<UStaticMeshComponent>("VehicleMesh");
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
	SetIsDestroyed(true);
}

void AVehicleBase::UpdateDamageState()
{
	const float Health = HealthComponent->GetHealth();
	const float MaxHealth = HealthComponent->GetMaxHealth();
	if (Health <= MaxHealth && Health > MaxHealth * 0.75)
	{
		SetDamageState(Healthy);
	} else if (Health <= MaxHealth * 0.75 && Health > MaxHealth * 0.5)
	{
		SetDamageState(Damaged);
	} else
	{
		SetDamageState(Critical);
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

void AVehicleBase::NotifyRestarted()
{
	Super::NotifyRestarted();
	// if (IsLocallyControlled())
	// {
	// 	SpawnHUD();
	// 	SpawnControls();
	// }
}

void AVehicleBase::UnPossessed()
{
	
	// if (IsLocallyControlled())
	// {
	// 	RemoveHUD();
	// 	RemoveControls();
	// }
	Super::UnPossessed();
}



void AVehicleBase::Enter_Implementation(ACharacterBase* NewPilot)
{
	if (bIsDestroyed || Pilot) return;
	SetPilotToPossess(NewPilot);
	if (IsPlayerControlled())
	{
		CL_Enter(NewPilot);
	}
	
	
}

void AVehicleBase::CL_Enter_Implementation(ACharacterBase* NewPilot)
{
	if (IsLocallyControlled())
	{
		SpawnHUD();
		SpawnControls();
	}
}

void AVehicleBase::SetPilotToPossess_Implementation(ACharacterBase* NewPilot)
{
	Pilot = NewPilot;
	PilotController = Pilot->GetController();
	SetOwner(PilotController);
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
		Pilot->AttachToComponent(GetVehicleMesh(), FAttachmentTransformRules::SnapToTargetNotIncludingScale, FName("Seat"));
		Pilot->GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	}
}

void AVehicleBase::Exit_Implementation()
{
	if (IsPlayerControlled())
		CL_Exit();
	ResetPilot();
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
	GetController()->UnPossess();
	PilotController->Possess(Pilot);
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
		Pilot->GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		Pilot->SetActorTransform(ExitPoint->GetComponentTransform());
	} else
	{
		UE_LOG(LogTemp, Warning, TEXT("Invalid pilot"));
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


void AVehicleBase::OnPilotKilled(ACharacterBase* Character, AController* Killer, AActor* Causer)
{
	Exit();
}

void AVehicleBase::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(AVehicleBase, Pilot);
}

UStaticMeshComponent* AVehicleBase::GetVehicleMesh()
{
	return VehicleMesh;
}

float AVehicleBase::CustomTakeDamage_Implementation(float DamageAmount, FVector Force, FDamageEvent const& DamageEvent,
                                                    AController* EventInstigator, AActor* DamageCauser)
{
	HealthComponent->TakeDamage(DamageAmount, Force, FVector(0,0,0), NAME_None, EventInstigator, DamageCauser);
	return DamageAmount;
}

float AVehicleBase::CustomTakePointDamage_Implementation(FPointDamageEvent const& PointDamageEvent, float Force,
                                                         AController* EventInstigator, AActor* DamageCauser)
{
	HealthComponent->TakeDamage(PointDamageEvent.Damage, PointDamageEvent.ShotDirection*Force, PointDamageEvent.HitInfo.Location, PointDamageEvent.HitInfo.BoneName, EventInstigator, DamageCauser);
	return PointDamageEvent.Damage;
}

float AVehicleBase::CustomTakeRadialDamage_Implementation(float Force, FRadialDamageEvent const& RadialDamageEvent,
	AController* EventInstigator, AActor* DamageCauser)
{

	HealthComponent->TakeDamage(RadialDamageEvent.Params.BaseDamage, (GetActorLocation() - RadialDamageEvent.Origin).GetSafeNormal()*Force, this->GetActorLocation(), NAME_None, EventInstigator, DamageCauser);
	return RadialDamageEvent.Params.BaseDamage;
}

void AVehicleBase::OnInteract_Implementation(ACharacterBase* Character)
{
	IInteractableInterface::OnInteract_Implementation(Character);

	if (bIsDestroyed) return;
}
