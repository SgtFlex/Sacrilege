// Fill out your copyright notice in the Description page of Project Settings.


#include "Core/VehicleBase.h"

#include "Core/AIControllerBase.h"
#include "UI/GrenadeWidget.h"
#include "Components/HealthComponent.h"
#include "Core/WeaponBase.h"
#include "Camera/CameraComponent.h"
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
	GetHealthComponent()->OnHealthUpdate.AddDynamic(this, &AVehicleBase::OnHealthUpdated);
	GetHealthComponent()->OnHealthDepleted.AddDynamic(this, &AVehicleBase::OnHealthDepleted);
	UpdateDamageState();
}

// Called every frame
void AVehicleBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (GetLocalRole() == ENetRole::ROLE_Authority) ServerUpdateAimRotation();
}

void AVehicleBase::ServerUpdateAimRotation_Implementation()
{
	AimRotation = GetBaseAimRotation();
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
	if (Pilot) Pilot->CustomTakeDamage(5000, Force, EventInstigator, DamageCauser);
	SetIsDestroyed(true);
}

void AVehicleBase::UpdateDamageState()
{
	const float Health = GetHealthComponent()->GetHealth();
	const float MaxHealth = GetHealthComponent()->GetMaxHealth();
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
	ServerSetPilotToPossess(NewPilot);
	if (IsPlayerControlled())
	{
		CL_Enter(NewPilot);
	}
	for (auto ChildVehicle : ChildVehicles)
	{
		if (ChildVehicle) ChildVehicle->AIDesired = true;
	}
	//TArray<AActor*> IgnoreActors;
	//GetIgnoreActors(IgnoreActors);
	//Pilot->EquippedWeapon->ActorsToIgnore.Append(IgnoreActors);
	OnEntered.Broadcast();
	IgnoreListUpdated.Broadcast();
}

void AVehicleBase::CL_Enter_Implementation(ACharacterBase* NewPilot)
{
	// if (IsLocallyControlled())
	// {
	// 	SpawnHUD();
	// 	SpawnControls();
	// }
}

void AVehicleBase::ServerSetPilotToPossess_Implementation(ACharacterBase* NewPilot)
{
	Pilot = NewPilot;
	PilotController = Pilot->GetController();
	SetOwner(PilotController);
	Pilot->bIsInVehicle = true;
	Pilot->OccupiedVehicle = this;
	MulticastAttachPilot();
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

void AVehicleBase::MulticastAttachPilot_Implementation()
{
	if (IsValid(Pilot))
	{
		FVector StartingPoint = Pilot->GetActorLocation();
		if (Pilot->VehicleAnimEnterMontages.Contains(AnimType)) Pilot->GetMesh()->GetAnimInstance()->Montage_Play(*Pilot->VehicleAnimEnterMontages.Find(AnimType.GetValue()));
		Pilot->GetCharacterMovement()->SetMovementMode(EMovementMode::MOVE_None);
		Pilot->GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
		Pilot->GetCapsuleComponent()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Vehicle, ECR_Ignore);
		FName Socket = NAME_None;
		if (GetVehicleMesh()->DoesSocketExist("Seat")) Socket = "Seat";
		if (AnimType!=Passenger && Pilot->EquippedWeapon) Pilot->EquippedWeapon->SetActorHiddenInGame(true);
		Pilot->AttachToComponent(GetVehicleMesh(), FAttachmentTransformRules::SnapToTargetIncludingScale, Socket);
		LerpToSeat(StartingPoint);
	}
}

void AVehicleBase::Exit_Implementation()
{
	if (!Pilot) return;
	LerpToExit();
	if (Pilot->VehicleAnimExitMontages.Contains(AnimType.GetValue()))
	{
		UAnimMontage* ExitMontage = *Pilot->VehicleAnimExitMontages.Find(AnimType.GetValue());
		Pilot->GetMesh()->GetAnimInstance()->Montage_Play(ExitMontage, ExitMontage->GetPlayLength()/ExitTime);
	}
	GetWorldTimerManager().SetTimer(ExitDelayHandle, this, &AVehicleBase::DoVehicleUnpossession, ExitTime, false);
}

void AVehicleBase::DoVehicleUnpossession_Implementation()
{
	if (!Pilot) return;
	if (IsPlayerControlled())
		CL_Exit();
	ServerResetPilot();
	OnExited.Broadcast();
	IgnoreListUpdated.Broadcast();
}

void AVehicleBase::CL_Exit_Implementation()
{
	if (IsLocallyControlled())
	{
		RemoveControls();
		RemoveHUD();
	}
}

void AVehicleBase::ServerResetPilot_Implementation()
{
	if (!Pilot) return;
	if (GetController())
	{
		Pilot->SetActorRotation(GetControlRotation());
		GetController()->UnPossess();
		if (Pilot->GetHealthComponent()->IsAlive())
		{
			
			PilotController->Possess(Pilot);
			
		}
	}
	Pilot->bIsInVehicle = false;
	Pilot->OccupiedVehicle = nullptr;
	Pilot->OnKilled.RemoveDynamic(this, &AVehicleBase::OnPilotKilled);
	MulticastDetachPilot();
	SetOwner(nullptr);
	Pilot = nullptr;
	PilotController = nullptr;
}

void AVehicleBase::MulticastDetachPilot_Implementation()
{
	if (IsValid(Pilot))
	{
		Pilot->GetCharacterMovement()->SetMovementMode(EMovementMode::MOVE_NavWalking);
		Pilot->DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
		
		//below should be on client only
		Pilot->LerpCamera(GetCamera()->GetComponentTransform().GetRelativeTransform(Pilot->GetFirstPersonCameraComponent()->GetComponentTransform()), Pilot->GetFirstPersonCameraComponent()->GetRelativeTransform());
		//Pilot->SetActorRotation(FRotator(0, 0, 0));
		//PilotController->SetControlRotation(GetControlRotation());
		if (Pilot->GetCapsuleComponent())
		{
			Pilot->GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
			Pilot->GetCapsuleComponent()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Vehicle, ECR_Block);
		}
		if (Pilot->VehicleAnimExitMontages.Contains(AnimType)) Pilot->GetMesh()->GetAnimInstance()->Montage_Stop(0, *Pilot->VehicleAnimExitMontages.Find(AnimType.GetValue()));
		//Pilot->SetActorLocation(ExitPoint->GetComponentLocation(), false);
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

void AVehicleBase::AddChildVehicleToIgnoreList(AVehicleBase* Vehicle)
{
	ChildVehicles.Add(Vehicle);
	Vehicle->OnEntered.AddUniqueDynamic(this, &AVehicleBase::UpdateIgnoreList);
	Vehicle->OnExited.AddUniqueDynamic(this, &AVehicleBase::UpdateIgnoreList);
	IgnoreListUpdated.Broadcast();
}

void AVehicleBase::UpdateIgnoreList()
{
	IgnoreListUpdated.Broadcast();
}

void AVehicleBase::GetIgnoreActors(TArray<AActor*>& IgnoreActors)
{
	if (IsChildActor())
	{
		if (AVehicleBase* ParentVehicle = Cast<AVehicleBase>(GetParentActor())) ParentVehicle->GetIgnoreActors(IgnoreActors);
	} else
	{
		for (const auto Vehicle : ChildVehicles)
		{
			IgnoreActors.Add(Vehicle);
			if (Vehicle && Vehicle->Pilot) IgnoreActors.Add(Vehicle->Pilot);
		}
	}
	IgnoreActors.Add(this);
	if (Pilot) IgnoreActors.Add(Pilot);
}

float AVehicleBase::CustomTakeDamage_Implementation(float Damage, FVector Force, AController* EventInstigator, AActor* DamageCauser)
{
	GetHealthComponent()->TakeDamage(Damage, Force, FVector(0,0,0), NAME_None, EventInstigator, DamageCauser);
	return Damage;
}

float AVehicleBase::CustomTakePointDamage_Implementation(float Damage, FVector Direction, const FHitResult& HitInfo, float Force, AController* EventInstigator, AActor* DamageCauser)
{
	GetHealthComponent()->TakeDamage(Damage, Direction*Force, HitInfo.Location, HitInfo.BoneName, EventInstigator, DamageCauser);
	return Damage;
}

float AVehicleBase::CustomTakeRadialDamage_Implementation(FVector Origin, float Radius, float Force, const FHitResult& HitInfo, FRadialDamageEvent const& RadialDamageEvent, float MinimumRadius, AController* EventInstigator, AActor* DamageCauser)
{
	if (IsChildActor()) return 0;
	GetHealthComponent()->TakeDamage(RadialDamageEvent.Params.BaseDamage, (GetActorLocation() - RadialDamageEvent.Origin).GetSafeNormal()*Force, this->GetActorLocation(), NAME_None, EventInstigator, DamageCauser);
	return RadialDamageEvent.Params.BaseDamage;
}

UHealthComponent* AVehicleBase::GetHealthComponent_Implementation()
{
	if (AVehicleBase* ParentVehicle = Cast<AVehicleBase>(GetParentActor())) return ParentVehicle->GetHealthComponent();
	return HealthComponent;
}

void AVehicleBase::OnInteract_Implementation(ACharacterBase* Character)
{
	if (!Character->GameplayTags.HasTag(FGameplayTag::RequestGameplayTag(FName("Character.CanUseVehicles")))) return;
	IInteractableInterface::OnInteract_Implementation(Character);

	if (bIsDestroyed) return;
}
