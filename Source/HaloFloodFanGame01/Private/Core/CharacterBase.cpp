// Fill out your copyright notice in the Description page of Project Settings.


#include "Core/CharacterBase.h"

#include "Core/AIControllerBase.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "EnhancedInputSubsystemInterface.h"
#include "Core/EquipmentBase.h"
#include "Core/GrenadeBase.h"
#include "Core/GunBase.h"
#include "Components/HealthComponent.h"
#include "Components/CapsuleComponent.h"
#include "Engine/DamageEvents.h"
#include "Kismet/GameplayStatics.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraComponent.h"
#include "Core/PlayerControllerBase.h"
#include "TimerManager.h"
#include "Subsystems/WorldCleanupManager.h"
#include "Animation/AnimInstance.h"
#include "Animation/AnimSingleNodeInstance.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Blueprint/UserWidget.h"
#include "Camera/CameraComponent.h"
#include "Components/DecalComponent.h"
#include "Engine/DecalActor.h"
#include "GameFramework/InputDeviceSubsystem.h"
#include "Net/UnrealNetwork.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISense_Damage.h"
#include "Perception/AISense_Touch.h"
#include "ProfilingDebugging/CookStats.h"

// Sets default values
ACharacterBase::ACharacterBase()
{
 	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	HealthComponent = CreateDefaultSubobject<UHealthComponent>(TEXT("HealthComponent"));
	
	SetReplicates(true);
	SetReplicateMovement(true);

	// Set size for collision capsule
	//GetCapsuleComponent()->InitCapsuleSize(55.f, 96.0f);
		
	

	// Create a mesh component that will be used when being viewed from a '1st person' view (when controlling this pawn)
	Mesh1P = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("CharacterMesh1P"));
	Mesh1P->SetOnlyOwnerSee(true);
	Mesh1P->SetupAttachment(GetCapsuleComponent());
	Mesh1P->bCastDynamicShadow = false;
	Mesh1P->CastShadow = false;
	//Mesh1P->SetRelativeRotation(FRotator(0.9f, -19.19f, 5.2f));
	//Mesh1P->SetRelativeLocation(FVector(-30.f, 0.f, -150.f));

	// Create a CameraComponent	
	FirstPersonCameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("FirstPersonCamera"));
	FirstPersonCameraComponent->SetupAttachment(GetMesh1P(), "HeadSocket");
	//FirstPersonCameraComponent->SetRelativeLocation(FVector(-10.f, 0.f, 60.f)); // Position the camera
	FirstPersonCameraComponent->bUsePawnControlRotation = true;
}

void ACharacterBase::ApplyPhysicsImpulse_Implementation(FVector Force, FVector Location, FName BoneName)
{
	if (GetHealthComponent()->IsAlive())
	{
		LaunchCharacter(Force*.01f, false, false);
	} else
	{
		GetMesh()->AddImpulseAtLocation(Force, Location, BoneName);
	}
}

// Called when the game starts or when spawned
void ACharacterBase::BeginPlay()
{
	Super::BeginPlay();
	
	GetCapsuleComponent()->OnComponentHit.AddDynamic(this, &ACharacterBase::OnHit);

	GetMesh()->OnComponentSleep.AddDynamic(this, &ACharacterBase::RagdollSettled);
	SpawnLoadout();

	
	if (GetHealthComponent()) GetHealthComponent()->OnHealthDepleted.AddDynamic(this, &ACharacterBase::OnHealthDepleted);

	InputDeviceSubsystem = GetGameInstance()->GetEngine()->GetEngineSubsystem<UInputDeviceSubsystem>();
}

void ACharacterBase::Restart()
{
	Cast<IGenericTeamAgentInterface>(GetController())->SetGenericTeamId(FGenericTeamId(TeamId));
	if (AAIControllerBase* AIC = Cast<AAIControllerBase>(GetController()))
		AIC->TeamNumber = TeamId;

	Super::Restart();
}

void ACharacterBase::SpawnLoadout()
{
	if (!bCanUseWeapons) return;
	UE_LOG(LogTemp, Warning, TEXT("Spawning weapons for %s"), *GetName());
	if (!HasAuthority())
		return;
	else
	{
		if (Loadouts.Num() > 0)
		{
			FLoadoutStruct ChosenLoadoutStruct = Loadouts[FMath::RandRange(0, Loadouts.Num() - 1)];
			if (ChosenLoadoutStruct.PrimaryWeaponClass)
			{
				AWeaponBase* Gun = GetWorld()->SpawnActor<AWeaponBase>(ChosenLoadoutStruct.PrimaryWeaponClass);
				PickupWeapon(Gun);
		
			}
			if (ChosenLoadoutStruct.SecondaryWeaponClass)
			{
				AWeaponBase* Gun = GetWorld()->SpawnActor<AWeaponBase>(ChosenLoadoutStruct.SecondaryWeaponClass);
				PickupWeapon(Gun);
			}
			if (!ChosenLoadoutStruct.Grenades.IsEmpty())
			{
				GrenadeInventory = ChosenLoadoutStruct.Grenades;
				OnGrenadeInventoryUpdated.Broadcast(GrenadeInventory);
			}
			if (ChosenLoadoutStruct.EquipmentClass)
			{
				EquipmentClass = ChosenLoadoutStruct.EquipmentClass;
			}
		}
	}
}

void ACharacterBase::Server_SpawnLoadout_Implementation()
{
	
	SpawnLoadout();
}

void ACharacterBase::Multi_SpawnLoadout_Implementation()
{
	
}

// Called every frame
void ACharacterBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	MeleeTimeline.TickTimeline(DeltaTime);
	if (GetController())
		GetMesh1P()->SetWorldRotation(FRotator(0, GetViewRotation().Yaw + 90, GetViewRotation().Pitch));

	if (HasAuthority())
		ServerSetCurrentInteractable();
}

void ACharacterBase::Move(const FInputActionValue& Value)
{
	// input is a Vector2D
	const FVector2D MovementVector = Value.Get<FVector2D>();
	if (Controller != nullptr)
	{
		// add movement
		AddMovementInput(GetActorForwardVector(), MovementVector.Y);
		AddMovementInput(GetActorRightVector(), MovementVector.X);
	}
}



void ACharacterBase::Look(const FInputActionValue& Value)
{
	// input is a Vector2D
	
	const FVector2D LookAxisVector = Value.Get<FVector2D>();
	// Disabled for now due to incorrect pitch values in standalone
	//Server_Look(LookAxisVector.Y);


	//const float AimAssistMultiplier = AimAssist();
	const float AimAssistMultiplier = 1;
	
	// add yaw and pitch input to controller
	AddControllerYawInput(LookAxisVector.X * AimAssistMultiplier * ScopeSensitivityMultiplier);
	AddControllerPitchInput(LookAxisVector.Y * AimAssistMultiplier * ScopeSensitivityMultiplier);
	//Mesh1P->AddLocalRotation(FRotator(0, 0, -LookAxisVector.Y));
}

void ACharacterBase::Server_Look_Implementation(const float Pitch)
{
	Multi_Look(Pitch);
}

void ACharacterBase::Multi_Look_Implementation(const float Pitch)
{
	FRotator Rotation = GetFirstPersonCameraComponent()->GetComponentRotation();
	Rotation.Pitch = Pitch;
	GetFirstPersonCameraComponent()->SetWorldRotation(Rotation);
}

void ACharacterBase::GetPlayerAim(FHitResult& HitResult) const
{
	HitResult = PlayerAim;
}

float ACharacterBase::AimAssist() const
{
	if (Controller != nullptr && InputDeviceSubsystem->GetMostRecentlyUsedHardwareDevice(GetPlatformUserId()).PrimaryDeviceType == EHardwareDevicePrimaryType::Gamepad)
	{
		FHitResult Aim;
		GetPlayerAim(Aim);
		if (Aim.GetActor())
		{
			ACharacterBase* AimedAtCharacter = Cast<ACharacterBase>(Aim.GetActor());
			if (AimedAtCharacter && AimedAtCharacter->GetHealthComponent()->IsAlive())
			{
				return 0.25;
			} else
			{
				return 1;
			}
		}
		
	}
	return 1;
}

//@TODO Potentially reduce replicated variables
void ACharacterBase::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(ACharacterBase, EquippedWeapon);
	DOREPLIFETIME(ACharacterBase, HolsteredWeapon);
	DOREPLIFETIME(ACharacterBase, Emotion);
	DOREPLIFETIME(ACharacterBase, AlertState);
	DOREPLIFETIME(ACharacterBase, TeamId);
	DOREPLIFETIME(ACharacterBase, GrenadeInventory);
	DOREPLIFETIME(ACharacterBase, InteractableActor);
	DOREPLIFETIME(ACharacterBase, HealthComponent);
}

// Called to bind functionality to input
void ACharacterBase::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	EnhancedInputComponent = CastChecked<UEnhancedInputComponent>(PlayerInputComponent);
	
	if (EnhancedInputComponent)
	{
		//Jumping
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Triggered, this, &ACharacter::Jump);
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Completed, this, &ACharacter::StopJumping);

		//Moving
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &ACharacterBase::Move);

		//Looking
		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &ACharacterBase::Look);

		EnhancedInputComponent->BindAction(InteractAction, ETriggerEvent::Triggered, this, &ACharacterBase::Interact);

		EnhancedInputComponent->BindAction(PrimaryAttackAction, ETriggerEvent::Started, this, &ACharacterBase::PrimaryAttack_Pull);

		EnhancedInputComponent->BindAction(PrimaryAttackAction, ETriggerEvent::Completed, this, &ACharacterBase::PrimaryAttack_Release);
		
		EnhancedInputComponent->BindAction(SwitchWeaponAction, ETriggerEvent::Triggered, this, &ACharacterBase::SwitchWeapon);

		EnhancedInputComponent->BindAction(ReloadAction, ETriggerEvent::Triggered, this, &ACharacterBase::ReloadWeapon);
		
		EnhancedInputComponent->BindAction(MeleeAction, ETriggerEvent::Triggered, this, &ACharacterBase::Melee);

		EnhancedInputComponent->BindAction(SwitchGrenadeAction, ETriggerEvent::Triggered, this, &ACharacterBase::CycleGrenadeType);
		
		EnhancedInputComponent->BindAction(ThrowGrenadeAction, ETriggerEvent::Triggered, this, &ACharacterBase::ThrowEquippedGrenade);

		EnhancedInputComponent->BindAction(UseEquipmentAction, ETriggerEvent::Triggered, this, &ACharacterBase::UseEquipment);
		
		EnhancedInputComponent->BindAction(ScopeAction, ETriggerEvent::Started, this, &ACharacterBase::SecondaryAttack_Start);

		EnhancedInputComponent->BindAction(ScopeAction, ETriggerEvent::Completed, this, &ACharacterBase::SecondaryAttack_End);
	}

}

void ACharacterBase::MulticastSpawnBloodFX_Implementation(FVector Direction, const FHitResult& HitInfo)
{
	if (GetHealthComponent()->GetShields() > 0)
	{
		
	} else
	{
		if (HitReactionBS)
		{
			GetMesh()->PlayAnimation(HitReactionBS, false);
			FVector BlendParams(50, 0.0, 0.0);
			GetMesh()->GetSingleNodeInstance()->SetBlendSpacePosition(BlendParams);
		}
		if (BloodPFX)
		{
			UNiagaraComponent* BloodNiagaraComponent = UNiagaraFunctionLibrary::SpawnSystemAttached(BloodPFX, GetMesh(), HitInfo.BoneName, HitInfo.ImpactPoint, HitInfo.Normal.Rotation(), EAttachLocation::KeepWorldPosition, true);
			//BloodNiagaraComponent->SetNiagaraVariableActor("Character", this);
			BloodNiagaraComponent->SetVariableActor("Character", this);
		}
		if (BloodSplatterMat)
		{			
			UGameplayStatics::SpawnDecalAttached(BloodSplatterMat, FVector(10,10,10), GetMesh(),
			HitInfo.BoneName, HitInfo.Location, HitInfo.Normal.Rotation() + FRotator(-90, 0, FMath::RandRange(-180, 180)), EAttachLocation::KeepWorldPosition, 0);
		}
		if (BloodDecalMaterial)
		{
			float DecalSize = FMath::RandRange(10, 130);
			FHitResult HitResult;
			FCollisionQueryParams QueryParams;
			QueryParams.AddIgnoredActor(this);
			GetWorld()->LineTraceSingleByChannel(HitResult, HitInfo.Location, HitInfo.Location + (Direction * 4000),ECollisionChannel::ECC_Visibility, QueryParams);
			if (HitResult.bBlockingHit)
			{
				UDecalComponent* BloodDecal = UGameplayStatics::SpawnDecalAttached(BloodDecalMaterial, FVector(20,DecalSize,DecalSize), HitResult.GetComponent(), HitResult.BoneName, HitResult.Location, HitResult.Normal.Rotation() + FRotator(-180,0,FMath::RandRange(-180, 180)), EAttachLocation::KeepWorldPosition);
				// UMaterialInstanceDynamic* DynamicMat = BloodDecal->CreateDynamicMaterialInstance();
				// DynamicMat->SetScalarParameterValue("Mask", 3);
				GetWorld()->GetSubsystem<UWorldCleanupManager>()->ManageDecal(BloodDecal);
				//Cast<AHaloGameState>(GetWorld()->GetGameState())->ManageDecal(UGameplayStatics::SpawnDecalAttached(BloodDecalMaterial, FVector(DecalSize,DecalSize,DecalSize), HitResult.GetComponent(), HitResult.BoneName, HitResult.Location, HitResult.Normal.Rotation() + FRotator(-180,0,FMath::RandRange(-180, 180)), EAttachLocation::KeepWorldPosition));
			}
		}
	}
	
}

float ACharacterBase::CustomTakeRadialDamage_Implementation(FVector Origin, float Radius, float Force, const FHitResult& HitInfo, FRadialDamageEvent const& RadialDamageEvent, float MinimumRadius, AController* EventInstigator, AActor* DamageCauser)
{
	//(MaxRange - HitDist)/(MaxRange - MinRange)
	const float Alpha = FMath::Pow( (Radius - FMath::Max((HitInfo.ImpactPoint - Origin).Length(), MinimumRadius))/(Radius - MinimumRadius), RadialDamageEvent.Params.DamageFalloff);
	return ChangeHealth(this,
		FMath::Lerp(RadialDamageEvent.Params.MinimumDamage, RadialDamageEvent.Params.BaseDamage, Alpha),
		(GetActorLocation() - RadialDamageEvent.Origin).GetSafeNormal() * Force,
		FVector(0,0,0), FName(""), EventInstigator, DamageCauser);
}

float ACharacterBase::CustomTakeDamage_Implementation(float Damage, FVector Force, AController* EventInstigator, AActor* DamageCauser)
{
	return ChangeHealth(this, Damage, Force, FVector(0,0,0), FName(""), EventInstigator, DamageCauser);
}

// float ACharacterBase::CustomOnTakeAnyDamage_Implementation(float DamageAmount, FVector Force,
// 	AController* EventInstigator, AActor* DamageCauser)
// {
// 	return IDamageableInterface::CustomOnTakeAnyDamage(DamageAmount, Force, EventInstigator, DamageCauser);
// }

float ACharacterBase::CustomTakePointDamage_Implementation(float Damage, FVector Direction, const FHitResult& HitInfo, float Force, AController* EventInstigator, AActor* DamageCauser)
{
	if (!HasAuthority()) return 0;
	if (const UHealthComponent* HealthComp = GetHealthComponent())
	{
		if (HealthComp->GetShields() <= 0)
		{
			if (HitBoxNameMap.Contains(HitInfo.BoneName))
				if (HitBoxDamageMultipliers.Contains(HitBoxNameMap[HitInfo.BoneName]))
					Damage = Damage * HitBoxDamageMultipliers[HitBoxNameMap[HitInfo.BoneName]];
			CurrentStunBuildup = CurrentStunBuildup + (Force/50);
			if (CurrentStunBuildup >= StunThreshold)
			{
				Stun();
			}
				
			MulticastSpawnBloodFX(Direction, HitInfo);
		}
	}
	
	float x = IDamageableInterface::ChangeHealth(this, Damage, Direction * Force, HitInfo.Location, HitInfo.BoneName, EventInstigator, DamageCauser);
	//float x = IDamageableInterface::CustomTakePointDamage(PointDamageEvent, Force, EventInstigator, DamageCauser);
	if (EventInstigator && Cast<AAIControllerBase>(GetController()))
	{
		UAISense_Damage::ReportDamageEvent(GetWorld(), this, EventInstigator->GetPawn(), Damage, Cast<AActor>(EventInstigator)->GetActorLocation(), HitInfo.Location);
	}
	
	
	OnTakeCustomPointDamage.Broadcast(Damage);
	return x;
}


UHealthComponent* ACharacterBase::GetHealthComponent_Implementation()
{
	return HealthComponent;
}


void ACharacterBase::OnHealthDepleted_Implementation(float Damage, FVector DamageForce, FVector HitLocation, FName HitBoneName, AController* EventInstigator, AActor* DamageCauser)
{
	OnKilled.Broadcast(this, EventInstigator, DamageCauser);
	if (EventInstigator)
		UE_LOG(LogTemp, Warning, TEXT("%s killed %s"), *EventInstigator->GetName(), *GetName())
	SV_OnHealthDepleted(Damage, DamageForce, HitLocation, HitBoneName, EventInstigator, DamageCauser);
	// if (!HasAuthority()) return;
	//GetHealthComponent()->Deactivate();
	// OnKilled.Broadcast(this, EventInstigator, DamageCauser);
	// DropGrenades();
	
	// if (BloodDecalMaterial) Cast<AHaloGameState>(GetWorld()->GetGameState())->ManageDecal(UGameplayStatics::SpawnDecalAtLocation(GetWorld(), BloodDecalMaterial, FVector(100, 100, 100), GetActorLocation(), FRotator(-90,0,0)));
	// if (DeathSound) UGameplayStatics::PlaySoundAtLocation(GetWorld(), DeathSound, GetActorLocation());
	// GetMesh()->GetAnimInstance()->Montage_Play(DeathAnim);
	// GetCapsuleComponent()->DestroyComponent();
	// SetRootComponent(GetMesh());
	// GetMesh()->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	// GetMesh()->SetSimulatePhysics(true);
	// GetMesh()->AddImpulseAtLocation(DamageForce, HitLocation, HitBoneName);
	
	
	// if (Cast<AAIController>(GetController())) GetController()->Destroy();
	
	// if (EquippedWeapon)
	// 	DropWeapon();

	// Cast<AHaloGameState>(GetWorld()->GetGameState())->ManageRagdoll(this);

	// if (Cast<APlayerControllerBase>(GetController()))
	// {
	// 	APlayerController* PC = PlayerController;
	// 	PlayerController->UnPossess();
	// 	if (PlayerHUD)
	// 		PlayerHUD->RemoveFromParent();
	//
	// 	Cast<AFirefightGameMode>(GetWorld()->GetAuthGameMode())->OnPlayerCharDied.Broadcast(this, Cast<APlayerControllerBase>(PC));
	// }
}

void ACharacterBase::SV_OnHealthDepleted_Implementation(float Damage, FVector Force, FVector HitLocation,
	FName HitBoneName, AController* EventInstigator, AActor* DamageCauser)
{
	
	if (GetController())
	{
		if (Cast<AAIController>(GetController())) GetController()->Destroy();
		UAIPerceptionSystem::GetCurrent( GetWorld() )->UnregisterSource(*this);
		if (Cast<APlayerControllerBase>(GetController()))
		{
			APlayerControllerBase* PC = PlayerController;
			PlayerController->UnPossess();
			//PlayerController->OnPlayerDeath.Broadcast(this, PC);
			//@TODO Hard reference to game mode, needs to be removed

		}
	}
	MC_OnHealthDepleted(Damage, Force, HitLocation, HitBoneName, EventInstigator, DamageCauser);
}

void ACharacterBase::MC_OnHealthDepleted_Implementation(float Damage, FVector Force, FVector HitLocation,
	FName HitBoneName, AController* EventInstigator, AActor* DamageCauser)
{
	
	if (EquippedWeapon)
		DropEquippedWeapon();
	//TODO Holsterd weapon is destroyed, but should we drop it perhaps? Something to think about
	if (HolsteredWeapon)
		HolsteredWeapon->Destroy();
	if (BloodDecalMaterial)
		GetWorld()->GetSubsystem<UWorldCleanupManager>()->ManageDecal(UGameplayStatics::SpawnDecalAtLocation(GetWorld(), BloodDecalMaterial, FVector(100, 100, 100), GetActorLocation(), FRotator(-90,0,0)));
		//Cast<AHaloGameState>(GetWorld()->GetGameState())->ManageDecal(UGameplayStatics::SpawnDecalAtLocation(GetWorld(), BloodDecalMaterial, FVector(100, 100, 100), GetActorLocation(), FRotator(-90,0,0)));
	if (DeathSound)
		UGameplayStatics::PlaySoundAtLocation(GetWorld(), DeathSound, GetActorLocation());
	
	GetCapsuleComponent()->DestroyComponent();
	SetRootComponent(GetMesh());
	if (Force.Length() > 50000 || !DeathAnim || bIsInVehicle || (HitBoxNameMap.Contains(HitBoneName) && HitBoxNameMap[HitBoneName]=="Head"))
	{
		GetMesh()->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		GetMesh()->SetSimulatePhysics(true);
		if (HitBoxNameMap.Contains(HitBoneName) && HitBoxNameMap[HitBoneName]=="Head")
			GetMesh()->AddImpulseAtLocation(((Force*7 + FVector(0,0,1)*Force.Length()*10) * GetMesh()->GetMass())*0.005, HitLocation, HitBoneName);
		else
			GetMesh()->AddImpulseAtLocation(Force, HitLocation, HitBoneName);
	} else
	{
		GetMesh()->GetAnimInstance()->Montage_Play(DeathAnim);
		FTimerDelegate RagdollDelegate = FTimerDelegate::CreateUObject(this, &ACharacterBase::CreateRagdollCorpse);
		GetWorldTimerManager().SetTimer(RagdollTimer, RagdollDelegate, DeathAnim->GetPlayLength(), false);
	}
	
	
	
	
	
	//OnKilled.Broadcast(this, EventInstigator, DamageCauser);
	DropGrenades();
	GetWorld()->GetSubsystem<UWorldCleanupManager>()->ManageCorpse(this);
	//Cast<AHaloGameState>(GetWorld()->GetGameState())->ManageRagdoll(this);
}

void ACharacterBase::CreateRagdollCorpse() const
{
	GetMesh()->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	GetMesh()->SetSimulatePhysics(true);
	//GetMesh()->SetComponentTickEnabled(false);
}


void ACharacterBase::DropGrenades_Implementation()
{
	for (auto GrenadeStruct : GrenadeInventory)
	{
		for (int i = 0; i < FMath::RandRange(0, GrenadeStruct.GrenadeAmount); i++)
		{
			FActorSpawnParameters ActorSpawnParameters = FActorSpawnParameters();
			ActorSpawnParameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
			FVector Loc = GetActorLocation();
			if (AGrenadeBase* Grenade = Cast<AGrenadeBase>(GetWorld()->SpawnActor(GrenadeStruct.GrenadeClass, &Loc)))
			{
				GetWorld()->GetSubsystem<UWorldCleanupManager>()->ManageGrenade(Grenade);
				//GetWorld()->GetSubsystem<UWorldCleanupManager>()->ManageActor("Grenades", Grenade);
				Grenade->Mesh->SetSimulatePhysics(true);
				//Grenade->Mesh->AddImpulse(Get * 0.025);
			}
				
		}
	}
}

void ACharacterBase::OnHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp,
                           FVector NormalImpulse, const FHitResult& Hit)
{
	LaunchCharacter(NormalImpulse/100, true, true);

	if (OtherComp)
	{
		UAISense_Touch::ReportTouchEvent(GetWorld(), this, OtherActor, Hit.Location);
		const float VelocityDifference = FMath::Abs(OtherComp->GetComponentVelocity().Length() - this->GetVelocity().Length());
		const FVector ForceVector = OtherComp->GetComponentVelocity() - this->GetVelocity();
		//Dotproduct doesnt work correctly if we're standing still
		const float DotProduct = FMath::Abs(Hit.Normal.Dot(ForceVector.GetSafeNormal()));
		
		//const float OtherCompMass = OtherComp->GetMass();
		//UE_LOG(LogTemp, Warning, TEXT("Dot: %f"), DotProduct);
		//const float DamageCalculation = FMath::Pow(VelocityDifference, 1.0f / 3.0f) * (OtherCompMass/300);
		const float DamageCalculation = (VelocityDifference/25) * DotProduct;
		if (DamageCalculation > 5)
		{
			//@TODO Review collision damage
			//CustomTakeDamage(DamageCalculation, NormalImpulse, nullptr, nullptr);
			//float DecalSize = 100;
			//UGameplayStatics::SpawnDecalAtLocation(GetWorld(), BloodDecalMaterial, FVector(DecalSize, DecalSize, DecalSize), GetMesh()->GetComponentLocation() + FVector(FMath::RandRange(-50, 50), FMath::RandRange(-50, 50), 0), FRotator(-90,0,FMath::RandRange(-180, 180)));
		}
	}
}

void ACharacterBase::SetSmartObject(ASmartObject* NewSmartObject)
{
	SmartObject = NewSmartObject;
	if (AAIControllerBase* AIController = Cast<AAIControllerBase>(GetController()))
	{
		AIController->SetSmartObject(NewSmartObject);
	}
}

void ACharacterBase::MeleeActor(AActor* Actor)
{
	// if (Actor->Implements<UDamageableInterface>())
	// {
	// 	const FVector Dir = (Actor->GetActorLocation() - GetActorLocation()).GetSafeNormal();
	// 	IDamageableInterface::Execute_CustomTakePointDamage(Actor, MeleeDamage, Dir, MeleeHit, MeleeForce, GetInstigatorController(), this);
	// }
	// UPrimitiveComponent* HitComp = MeleeHit.GetComponent();
	// if (HitComp && HitComp->IsSimulatingPhysics())
	// {
	// 	FVector ForceVector = (HitComp->GetComponentLocation() - GetActorLocation());
	// 	ForceVector.Normalize();
	// 	HitComp->AddImpulse(ForceVector*MeleeForce);
	// }
	// if (MeleeImpactFX.Contains(MeleeHit.PhysMaterial->SurfaceType))
	// {
	// 	if (TSubclassOf<ADecalActor> MeleeImpactClass = *MeleeImpactFX.Find(MeleeHit.PhysMaterial->SurfaceType))
	// 	{
	// 		const FVector Loc = MeleeHit.Location;
	// 		const FRotator Rot =  MeleeHit.ImpactNormal.Rotation() + FRotator(-90,0,0);
	// 		GetWorld()->SpawnActor(MeleeImpactClass, &Loc, &Rot);
	// 	}
	// }
}

void ACharacterBase::MeleeUpdate(float Alpha)
{
	// SetActorLocation(FMath::Lerp(StartMeleeLocation, EndMeleeLocation, Alpha));
	// if (Controller)
	// {
	// 	Controller->SetControlRotation(FMath::Lerp(StartMeleeRotation, (EndMeleeLocation - StartMeleeLocation).Rotation(), Alpha));
	// }
}


bool ACharacterBase::CanMelee_Implementation()
{
	if (GetWorld()->GetTimerManager().TimerExists(MeleeTimer)) return false;
	return true;
}

void ACharacterBase::EquipGrenadeType_Implementation(TSubclassOf<AGrenadeBase> Grenade)
{
	// EquippedGrenadeClass = Grenade;
}

void ACharacterBase::Melee_Implementation()
{
	if (!bCanMelee) return;
	SV_Melee();
}

void ACharacterBase::SV_Melee_Implementation()
{
	if (IsPlayerControlled())
	{
		PlayerMelee();
	} else
	{
		NPCMelee();
	}
}

void ACharacterBase::PlayerMelee_Implementation()
{
	EquippedWeapon->WeaponMelee();
	// if (GetWorld()->GetTimerManager().TimerExists(MeleeTimer)) return;
	// GetWorld()->GetTimerManager().SetTimer(MeleeTimer, 1, false);
	// MulticastPlayMeleeFX();
	// FVector TraceStart = GetFirstPersonCameraComponent()->GetComponentLocation();
	// FVector TraceEnd = GetFirstPersonCameraComponent()->GetComponentLocation() + GetFirstPersonCameraComponent()->GetForwardVector()*500;
	// FCollisionQueryParams CollisionParameters;
	// CollisionParameters.AddIgnoredActor(this);
	// CollisionParameters.AddIgnoredActor(GetAttachParentActor());
	// CollisionParameters.bReturnPhysicalMaterial = true;
	// GetWorld()->LineTraceSingleByChannel(MeleeHit, TraceStart, TraceEnd, ECollisionChannel::ECC_Visibility, CollisionParameters);
	// //DrawDebugLine(GetWorld(), TraceStart, TraceEnd, FColor(255, 0, 0), false, 3);
	//
	// if (MeleeHit.GetActor())
	// {
	// 	
	// 	ACharacterBase* MeleeChar = Cast<ACharacterBase>(MeleeHit.GetActor());
	// 	if (MeleeChar && MeleeChar->GetHealthComponent()->IsAlive())
	// 	{
	// 		Lunge(MeleeHit.GetActor());
	// 		// StartMeleeLocation = GetActorLocation();
	// 		// StartMeleeRotation = GetController()->GetControlRotation();
	// 		// EndMeleeLocation = MeleeHit.GetActor()->GetActorLocation();
	// 		// SlideMelee(MeleeChar);
	// 	} else
	// 	{
	// 		MeleeActor(MeleeHit.GetActor());
	// 	}
	// }
}

void ACharacterBase::NPCMelee_Implementation()
{
	
	if (!CanMelee()) return;
	MulticastPlayMeleeFX();
	GetWorld()->GetTimerManager().SetTimer(MeleeTimer, 1, false);
	FCollisionShape BoxShape = FCollisionShape::MakeBox(FVector(250, 250, 50));
	TArray<FHitResult> SweepResult;
	FCollisionQueryParams CollisionParams;
	CollisionParams.AddIgnoredActor(this);
	GetWorld()->SweepMultiByChannel(SweepResult, GetActorLocation(), GetActorLocation() + GetActorForwardVector()*100, FQuat(0,0,0,0), ECollisionChannel::ECC_Pawn, BoxShape, CollisionParams);
	for (auto Result : SweepResult)
	{
		if (Result.GetActor() && Result.GetActor()->Implements<UDamageableInterface>() && Result.GetActor()!=this)
		{
			FDamageEvent DamageEvent;
			IDamageableInterface::Execute_CustomTakeDamage(Result.GetActor(), MeleeDamage, FVector(0,0,0), nullptr, this);

		}
		UPrimitiveComponent* HitComp = Result.GetComponent();
		
		if (HitComp && HitComp->IsSimulatingPhysics())	
		{
			FVector ForceVector = (HitComp->GetComponentLocation() - GetActorLocation());
			ForceVector.Normalize();
			HitComp->AddImpulse(ForceVector*MeleeForce);
		}
	}
}

void ACharacterBase::MulticastPlayMeleeFX_Implementation()
{
	if (MeleeAnim)
		GetMesh()->GetAnimInstance()->Montage_Play(MeleeAnim);
}

bool ACharacterBase::AddGrenade_Implementation(const TSubclassOf<AGrenadeBase> GrenadeType, int Amount)
{
	bool FoundGrenade = false;
	
	for (int i = 0; i < GrenadeInventory.Num(); i++)
	{
		if (GrenadeInventory[i].GrenadeClass == GrenadeType)
		{
			FoundGrenade = true;
			if (GrenadeInventory[i].GrenadeAmount < 4)
			{
				GrenadeInventory[i].GrenadeAmount++;
				if (IsLocallyControlled() && GrenadeType.GetDefaultObject()->PickupSFX) UGameplayStatics::PlaySound2D(GetWorld(), GrenadeType.GetDefaultObject()->PickupSFX);
			} else
			{
				return false;
			}
		}
	}

	if (!FoundGrenade)
	{
		FGrenadeStruct Grenade;
		Grenade.GrenadeClass = GrenadeType;
		Grenade.GrenadeAmount = 1;
		GrenadeInventory.Add(Grenade);
		if (GrenadeType.GetDefaultObject()->PickupSFX) UGameplayStatics::PlaySound2D(GetWorld(), GrenadeType.GetDefaultObject()->PickupSFX);
	}
	
	OnGrenadeInventoryUpdated.Broadcast(GetGrenadeInventory());
	return true;
}

void ACharacterBase::ThrowEquippedGrenade()
{
	if (!bCanUseGrenades) return;
	if (GetGrenadeInventory().IsEmpty()) return;
	ThrowGrenade(GetGrenadeTypeIndex());
}

void ACharacterBase::ThrowGrenade(const int GrenadeTypeIndex)
{
	if (GetGrenadeInventory().IsEmpty()) return;
	ThrowGrenadeFX(GetSelectedGrenadeType());
	if (GetLocalRole()!=ROLE_Authority)
	{
		//If we are a client, play the throw grenade FX and animation and send an RPC to the server to throw the actual grenade
		ServerThrowGrenade(GrenadeTypeIndex);
		return;
	}
	// FTimerDelegate ThrowGrenadeTimerDelegate;
	// ThrowGrenadeTimerDelegate.BindUObject(this, &ACharacterBase::SpawnGrenade, GrenadeInventory[GrenadeTypeIndex].GrenadeClass);
	// GetWorldTimerManager().SetTimer(ThrowGrenadeDelayHandle, ThrowGrenadeTimerDelegate, 0.5f, false);
	MulticastThrowGrenade(GrenadeInventory[GrenadeTypeIndex].GrenadeClass);
	SpawnGrenade(GrenadeInventory[GrenadeTypeIndex].GrenadeClass);
	SubtractGrenade(GrenadeTypeIndex);
	
}

void ACharacterBase::SubtractGrenade(const int GrenadeTypeIndex)
{
	GrenadeInventory[GrenadeTypeIndex].GrenadeAmount -= 1;
	if (GrenadeInventory[GrenadeTypeIndex].GrenadeAmount <= 0)
	{
		RemoveGrenadeStruct(GrenadeTypeIndex);
	}
	OnGrenadeInventoryUpdated.Broadcast(GrenadeInventory);
}

void ACharacterBase::RemoveGrenadeStruct(int GrenadeTypeIndex)
{
	GrenadeInventory.RemoveAt(GrenadeTypeIndex);
	if (!GrenadeInventory.IsEmpty())
		SetGrenadeTypeIndex(GrenadeTypeIndex % GrenadeInventory.Num());
}

void ACharacterBase::SpawnGrenade(const TSubclassOf<AGrenadeBase>& GrenadeType)
{
	const FTransform SpawnTransform = FTransform(GetFirstPersonCameraComponent()->GetForwardVector().Rotation(), GetFirstPersonCameraComponent()->GetComponentLocation() + GetFirstPersonCameraComponent()->GetForwardVector()*300);
	FActorSpawnParameters ActorSpawnParameters;
	ActorSpawnParameters.Instigator = this;
	ActorSpawnParameters.Owner = this;

	if (AGrenadeBase* Grenade = Cast<AGrenadeBase>(GetWorld()->SpawnActorDeferred<AGrenadeBase>(GrenadeType, SpawnTransform, this, this, ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn)))
	{
		Grenade->SetInstigator(this);
		Grenade->FinishSpawning(SpawnTransform);
		Grenade->SetArmed(true);
		//UGameplayStatics::SpawnSoundAtLocation(GetWorld(), Grenade->ThrowSFX, Grenade->GetActorLocation());
		FVector Direction = GetFirstPersonCameraComponent()->GetForwardVector() + FVector(0,0,0.15);
		Direction.Normalize();
		Grenade->Mesh->AddImpulse(Direction*2000.0f, NAME_None, true);
		Grenade->Mesh->AddAngularImpulseInDegrees(Grenade->GetActorRightVector().GetSafeNormal()*1000 , NAME_None, true);
	}
}


void ACharacterBase::ServerThrowGrenade_Implementation(const int GrenadeTypeIndex)
{
	ThrowGrenade(GrenadeTypeIndex);
}

void ACharacterBase::MulticastThrowGrenade_Implementation(const TSubclassOf<AGrenadeBase> GrenadeType)
{
	//Ignore self from multicast, already called these locally in ThrowGrenade
	if (!IsLocallyControlled())
	{
		ThrowGrenadeFX(GrenadeType);
	}
}

void ACharacterBase::ThrowGrenadeFX(const TSubclassOf<AGrenadeBase> GrenadeType)
{
	UGameplayStatics::SpawnSoundAtLocation(GetWorld(), GrenadeType.GetDefaultObject()->ThrowSFX, GetActorLocation());
	if (IsLocallyControlled())
	{
		if (ThrowGrenadeAnimation1P) GetMesh1P()->GetAnimInstance()->Montage_Play(ThrowGrenadeAnimation1P);
	}
	if (ThrowGrenadeAnimation) GetMesh()->GetAnimInstance()->Montage_Play(ThrowGrenadeAnimation);
}

TArray<FGrenadeStruct>& ACharacterBase::GetGrenadeInventory()
{
	return GrenadeInventory;
}

TSubclassOf<AGrenadeBase> ACharacterBase::GetSelectedGrenadeType() const
{
	if (GrenadeInventory.IsEmpty()) return nullptr;
	return GrenadeInventory[GetGrenadeTypeIndex()].GrenadeClass;
}

bool ACharacterBase::SelectGrenadeType(const TSubclassOf<AGrenadeBase> GrenadeType) 
{
	for (int i = 0; i < GrenadeInventory.Num(); i++)
	{
		if (GrenadeInventory[i].GrenadeClass == GrenadeType)
		{
			SetGrenadeTypeIndex(i);
			return true;
		}
	}
	return false;
}

void ACharacterBase::SetGrenadeTypeIndex(const int Index)
{
	CurGrenadeTypeI = FMath::Clamp(Index, 0, GrenadeInventory.Num() - 1);
}

int ACharacterBase::GetGrenadeTypeIndex() const
{
	return CurGrenadeTypeI;
}

void ACharacterBase::CycleGrenadeType()
{
	if (GrenadeInventory.Num() <= 0) return;

	SetGrenadeTypeIndex((GetGrenadeTypeIndex() + 1) % GrenadeInventory.Num());
	ClientCycleGrenadeType();
	OnGrenadeTypeSwitched.Broadcast(GetSelectedGrenadeType(), GetGrenadeTypeIndex());
	//SwitchToGrenadeType(CurGrenadeTypeI+1);
}

void ACharacterBase::ClientCycleGrenadeType_Implementation() const
{
	UGameplayStatics::PlaySound2D(GetWorld(), GetSelectedGrenadeType().GetDefaultObject()->PickupSFX);
}

void ACharacterBase::UseEquipment_Implementation()
{
}

void ACharacterBase::PrimaryAttack_Pull()
{
	
	Server_PrimaryAttack_Pull();
	
}

void ACharacterBase::Server_PrimaryAttack_Pull_Implementation()
{
	if (EquippedWeapon)
		EquippedWeapon->PrimaryFire_Start();
	// if (EquippedWeapon)
	// 	EquippedWeapon->PullTrigger();
	//Multi_PrimaryAttack_Pull();
}

//DEPRECATED
void ACharacterBase::Multi_PrimaryAttack_Pull_Implementation()
{

	if (EquippedWeapon)
		EquippedWeapon->PrimaryFire_Start();
		// EquippedWeapon->PullTrigger();
	
}

void ACharacterBase::PrimaryAttack_Release()
{
	Server_PrimaryAttack_Release();
}

void ACharacterBase::Server_PrimaryAttack_Release_Implementation()
{
	if (EquippedWeapon)
		EquippedWeapon->PrimaryFire_End();
		//EquippedWeapon->ReleaseTrigger();
	//Multi_PrimaryAttack_Release();
}

//DEPRECATED
void ACharacterBase::Multi_PrimaryAttack_Release_Implementation()
{
	if (EquippedWeapon)
		EquippedWeapon->PrimaryFire_End();
}

void ACharacterBase::ReloadWeapon()
{
	Server_ReloadWeapon();
}

void ACharacterBase::Server_ReloadWeapon_Implementation()
{
	if (EquippedWeapon)
		EquippedWeapon->Reload();
	//Multi_ReloadWeapon();
}

//DEPRECATED
void ACharacterBase::Multi_ReloadWeapon_Implementation()
{
	if (EquippedWeapon)
	{
		EquippedWeapon->Reload();
	}
}

void ACharacterBase::SwitchWeapon()
{
	Server_SwitchWeapon();
}

void ACharacterBase::Server_SwitchWeapon_Implementation()
{
	if (!(EquippedWeapon && HolsteredWeapon))
		return;
	
	Multi_SwitchWeapon();
	
	GetWorld()->GetTimerManager().SetTimer(HolsterHandle, FTimerDelegate::CreateUObject(this, &ACharacterBase::ServerFinishSwitchingWeapons), HolsteredWeapon->HolsterSpeed, false);
}

void ACharacterBase::Multi_SwitchWeapon_Implementation()
{
	MulticastHolsterEquippedWeapon();
}

void ACharacterBase::ServerFinishSwitchingWeapons_Implementation()
{
	AWeaponBase* TempGun = EquippedWeapon;
	EquippedWeapon = HolsteredWeapon;
	HolsteredWeapon = TempGun;
	if (HasAuthority())
	{
		DrawEquippedWeapon();
	}
}

void ACharacterBase::ScopeWeapon()
{
	if (EquippedWeapon)
	{
		EquippedWeapon->SecondaryFire_Start();
	}
	// if (EquippedWeapon)
	// {
	// 	if (EquippedWeapon->ScopeActive)
	// 	{
	// 		EquippedWeapon->ScopeOut();
	// 	} else if (!EquippedWeapon->ScopeActive && EquippedWeapon->ZoomFOV != 0.0f)
	// 	{
	// 		EquippedWeapon->ScopeIn();
	// 	}
	// }
}

void ACharacterBase::SecondaryAttack_Start()
{
	if (EquippedWeapon)
	{
		EquippedWeapon->SecondaryFire_Start();
	}
}

void ACharacterBase::SecondaryAttack_End()
{
	if (EquippedWeapon)
	{
		EquippedWeapon->SecondaryFire_End();
	}
}

void ACharacterBase::DrawEquippedWeapon()
{
	if (!EquippedWeapon)
	{
		UE_LOG(LogTemp, Warning, TEXT("No equipped weapon for: %s"), *GetName());
		return;
	}
	
	//EquippedWeapon->SetReplicateMovement(false);
	EquippedWeapon->Mesh->SetSimulatePhysics(false);
	EquippedWeapon->SetActorEnableCollision(false);
	EquippedWeapon->SetActorHiddenInGame(false);
	
	EquippedWeapon->Equip();

	if (HolsteredWeapon)
		HolsteredWeapon->SetActorHiddenInGame(true);


	SetupViewmodel(true);
	
	WeaponsUpdated.Broadcast(EquippedWeapon, HolsteredWeapon);
}

void ACharacterBase::SetupViewmodel(const bool bFirstPerson)
{
	if (!EquippedWeapon) return;
 	EquippedWeapon->ForceNetUpdate();
	if (bFirstPerson && IsLocallyViewed())
	{
		GetMesh1P()->bPauseAnims = false;
		if (EquippedWeapon->DrawSFX) UGameplayStatics::PlaySoundAtLocation(GetWorld(), EquippedWeapon->DrawSFX, GetActorLocation());
		EquippedWeapon->AttachToComponent(GetMesh1P(), FAttachmentTransformRules::SnapToTargetNotIncludingScale, "GripPoint");
		if (EquippedWeapon->DrawAnimation1P)
		{
			GetMesh1P()->GetAnimInstance()->Montage_Play(EquippedWeapon->DrawAnimation1P, EquippedWeapon->DrawAnimation1P->GetPlayLength() / EquippedWeapon->DrawSpeed);
		}
	}
	else
	{
		GetMesh1P()->bPauseAnims = true;
		UE_LOG(LogTemp, Warning, TEXT("Attached to main mesh 3P"));
		EquippedWeapon->AttachToComponent(GetMesh(), FAttachmentTransformRules::SnapToTargetNotIncludingScale, "GripPoint");
	}
}

void ACharacterBase::MulticastHolsterEquippedWeapon_Implementation()
{
	if (!EquippedWeapon) return;
	EquippedWeapon->Holster();
	UE_LOG(LogTemp, Warning, TEXT("%s holstered %s"), *GetName(), *EquippedWeapon->GetName());
	//Gun->SetActorHiddenInGame(true);

	if (IsLocallyControlled())
	{
		if (EquippedWeapon->HolsterAnimation1P)
			GetMesh1P()->GetAnimInstance()->Montage_Play(EquippedWeapon->HolsterAnimation1P, EquippedWeapon->HolsterAnimation1P->GetPlayLength() / EquippedWeapon->HolsterSpeed);
	}
	// if (!EquippedWeapon) return;
	// UE_LOG(LogTemp, Warning, TEXT("%s holstered %s"), *GetName(), *EquippedWeapon->GetName());
	// if (EquippedWeapon->ScopeActive) EquippedWeapon->ScopeOut();
	// EquippedWeapon->ReleaseTrigger();
	// GetWorldTimerManager().ClearTimer(EquippedWeapon->ReloadTimer);
	// EquippedWeapon->bReloading = false;
	// //Gun->SetActorHiddenInGame(true);
	//
	// if (IsLocallyControlled())
	// {
	// 	if (EquippedWeapon->HolsterAnimation1P)
	// 		GetMesh1P()->GetAnimInstance()->Montage_Play(EquippedWeapon->HolsterAnimation1P, EquippedWeapon->HolsterAnimation1P->GetPlayLength() / EquippedWeapon->HolsterSpeed);
	// }
}

void ACharacterBase::PickupWeapon(AWeaponBase* Gun)
{
	if (!bCanUseWeapons) return;
	if (HasAuthority())
	{
		Server_PickupWeapon(Gun);
	}
}

void ACharacterBase::Server_PickupWeapon_Implementation(AWeaponBase* Gun)
{
	// Server_PickupWeapon(Gun);
	Gun->SetOwner(this);
	Gun->ForceNetUpdate();
	//Gun->SetReplicateMovement(false);
	Gun->Mesh->SetSimulatePhysics(false);
	Gun->SetActorEnableCollision(false);
	
	UE_LOG(LogTemp, Warning, TEXT("%s picked up %s"), *GetName(), *Gun->GetName());

	Gun->AttachToComponent(GetMesh(), FAttachmentTransformRules::SnapToTargetNotIncludingScale, "GripPoint");
	Gun->Pickup(this);
	
	if (!EquippedWeapon)
	{
		EquippedWeapon = Gun;
	} else if (!HolsteredWeapon)
	{
		HolsteredWeapon = Gun;
		Gun->SetActorHiddenInGame(true);
	} else
	{
		DropEquippedWeapon();
		EquippedWeapon = Gun;
	}
	// EquipWeapon(EquippedWeapon);
	if (EquippedWeapon)
	{
		WeaponsUpdated.Broadcast(EquippedWeapon, HolsteredWeapon);
	}
	//If we're the server, then run EquipWeapon since we don't get RepNotifies
	if (HasAuthority())
		DrawEquippedWeapon();

	// Multi_PickupWeapon(Gun);
}
//
// void ACharacterBase::Multi_PickupWeapon_Implementation(AWeaponBase* Gun)
// {
// 	Gun->Mesh->SetSimulatePhysics(false);
// 	Gun->SetActorEnableCollision(false);
// 	Gun->OnPickup(this);
// 	
// 	if (!EquippedWeapon)
// 	{
// 		EquippedWeapon = Gun;
// 	} else if (!HolsteredWeapon)
// 	{
// 		HolsteredWeapon = Gun;
// 		Gun->SetActorHiddenInGame(true);
// 	} else
// 	{
// 		DropWeapon();
// 		EquippedWeapon = Gun;
// 	}
// 	//EquipWeapon(EquippedWeapon);
// }

void ACharacterBase::OnRep_EquippedWeapon()
{
	if (!EquippedWeapon) return;
	DrawEquippedWeapon();
}

void ACharacterBase::OnRep_HolsteredWeapon()
{
	if (!HolsteredWeapon) return;
	HolsteredWeapon->SetOwner(this);
	//HolsteredWeapon->SetReplicateMovement(false);
	HolsteredWeapon->AttachToComponent(GetMesh(), FAttachmentTransformRules::SnapToTargetNotIncludingScale, "Holster");
	HolsteredWeapon->SetActorHiddenInGame(true);
	HolsteredWeapon->Mesh->SetSimulatePhysics(false);
	HolsteredWeapon->SetActorEnableCollision(false);
}

void ACharacterBase::DropEquippedWeapon()
{
	if (!EquippedWeapon) return;
	DropWeapon(EquippedWeapon);
}

void ACharacterBase::DropWeapon(AWeaponBase* Gun)
{
	//Gun->SetReplicateMovement(true);
	if (!Gun) return;
	Gun->Drop();
	Gun->Mesh->AddImpulse(GetControlRotation().Vector() * 300, NAME_None, true);
	WeaponsUpdated.Broadcast(EquippedWeapon, HolsteredWeapon);
	EquippedWeapon = nullptr;
}

void ACharacterBase::RagdollSettled(UPrimitiveComponent* Component, FName Name)
{

	GetMesh()->PutAllRigidBodiesToSleep();
	//GetMesh()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Ignore);
	
}

void ACharacterBase::Stun(float StunTime)
{
	if (GetWorldTimerManager().TimerExists(StunTimer)) return;
	CurrentStunBuildup = 0;

	if (AAIControllerBase* AIC = Cast<AAIControllerBase>(GetController()))
	{
		constexpr float StunDuration = 1.5f;

		AIC->BehaviorTreeComp->PauseLogic(FString("Stunned"));
		AIC->StopMovement();
		AIC->BlackboardComp->SetValueAsBool("IsStunned", true);
		AIC->ClearFocus(EAIFocusPriority::Gameplay);
		MulticastPlayStunAnimation(StunDuration);
		FTimerDelegate UnstunDelegate = FTimerDelegate::CreateUObject(this, &ACharacterBase::Unstun);
		// GetWorld()->GetTimerManager().SetTimer(StunTimer, UnstunDelegate, StunDuration, false);
		GetWorld()->GetTimerManager().SetTimer(StunTimer, this, &ACharacterBase::Unstun, StunDuration, false);
	}
}

void ACharacterBase::MulticastPlayStunAnimation_Implementation(float StunTime)
{
	if (HurtAnim)
		GetMesh()->GetAnimInstance()->Montage_Play(HurtAnim);
}

void ACharacterBase::Unstun() const
{
	if (AAIControllerBase* AIC = Cast<AAIControllerBase>(GetController()))
	{
		AIC->BehaviorTreeComp->ResumeLogic(FString("Unstunned"));
		AIC->BlackboardComp->SetValueAsBool("IsStunned", false);
	}
}

void ACharacterBase::ServerSetCurrentInteractable_Implementation()
{
	if (!IsPlayerControlled() || !CanInteract || !GameplayTags.HasTag(FGameplayTag::RequestGameplayTag(FName("Character.CanInteract")))) return;
	TArray<AActor*> ActorsToIgnore;
	ActorsToIgnore.Add(this);
	if (IsPlayerControlled())
	UKismetSystemLibrary::SphereTraceSingle(GetWorld(), FirstPersonCameraComponent->GetComponentLocation(), FirstPersonCameraComponent->GetComponentLocation() + FirstPersonCameraComponent->GetForwardVector()*10000.0f, 20, UEngineTypes::ConvertToTraceType(ECollisionChannel::ECC_Visibility), false, ActorsToIgnore, EDrawDebugTrace::None, PlayerAim, true, FLinearColor::Red, FLinearColor::Green, 5);
	AActor* FoundActor = nullptr;


	FHitResult HitResult;
	GetPlayerAim(HitResult);
	if (HitResult.GetActor() && PlayerAim.Distance < 250 && HitResult.GetActor()->Implements<UInteractableInterface>())
	{
		FoundActor = HitResult.GetActor();
	} else
	{
		TArray<AActor*> Actors;
		TArray<TEnumAsByte<EObjectTypeQuery>> Objects;
		UKismetSystemLibrary::SphereOverlapActors(GetWorld(), GetActorLocation(), 300, Objects, AActor::StaticClass(), ActorsToIgnore, Actors);
		//InteractionSphere->GetOverlappingActors(Actors);
		
		if (!Actors.IsEmpty())
		{
			float ClosestDist = 0;
			for (const auto Actor : Actors)
			{
				if (Actor->Implements<UInteractableInterface>() && (ClosestDist == 0 || GetDistanceTo(Actor) < ClosestDist))
				{
					ClosestDist = GetDistanceTo(Actor);
					FoundActor = Actor;
				}
			}
		}
	}

	if (InteractableActor != FoundActor)
	{
		InteractableActor = FoundActor;
		MulticastUpdateInteractInfo();
		OnInteractableChanged.Broadcast(InteractableActor);
	}
}

void ACharacterBase::MulticastUpdateInteractInfo_Implementation()
{
	OnInteractableChanged.Broadcast(InteractableActor);
}

void ACharacterBase::Interact()
{
	if (!CanInteract) return;
	Server_Interact();
}

void ACharacterBase::Server_Interact_Implementation()
{
	if (InteractableActor && InteractableActor->Implements<UInteractableInterface>())
	{
		IInteractableInterface::Execute_OnInteract(InteractableActor, this);
	}
}

void ACharacterBase::NotifyRestarted()
{
	Super::NotifyRestarted();
	bUseControllerRotationYaw = IsPlayerControlled();
	if (APlayerControllerBase* PC = Cast<APlayerControllerBase>(GetController()))
	{
		PlayerController = PC;
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PC->GetLocalPlayer()))
		{
			Subsystem->AddMappingContext(DefaultMappingContext, 0);
		}
		UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PC->GetLocalPlayer());
		if (IsLocallyControlled())
		{
			if (PlayerHUDClass && !PlayerHUD) {
				PlayerHUD = CreateWidget<UUserWidget>(PC, PlayerHUDClass);
				PlayerHUD->AddToPlayerScreen();
			}
			PC->SetShowMouseCursor(false);
			FInputModeGameOnly InputModeGameOnly;
			PC->SetInputMode(InputModeGameOnly);
			SetupViewmodel(true);
		}
	}
}

void ACharacterBase::UnPossessed()
{
	CL_Unpossessed();
	PlayerController = nullptr;
	SetupViewmodel(false);
	Super::UnPossessed();
}

void ACharacterBase::SpawnDefaultController()
{
	Super::SpawnDefaultController();
	// if (AAIControllerBase* AICBase = Cast<AAIControllerBase>(GetController()))
	// {
	// 	AICBase->SetGenericTeamId(TeamId);
	// }
}

void ACharacterBase::CL_Unpossessed_Implementation()
{
	if (const APlayerController* PC = Cast<APlayerController>(Controller))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PC->GetLocalPlayer()))
		{
			Subsystem->RemoveMappingContext(DefaultMappingContext);
		}
		if (PlayerHUD)
		{
			PlayerHUD->RemoveFromParent();
			PlayerHUD = nullptr;
		}
	}
}

