// Fill out your copyright notice in the Description page of Project Settings.


#include "Core/CharacterBase.h"

#include "AIControllerBase.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "GrenadeBase.h"
#include "GunBase.h"
#include "HaloGameState.h"
#include "HealthComponent.h"
#include "Components/CapsuleComponent.h"
#include "Engine/DamageEvents.h"
#include "Kismet/GameplayStatics.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraComponent.h"
#include "PlayerControllerBase.h"
#include "TimerManager.h"
#include "WorldCleanupManager.h"
#include "Animation/AnimInstance.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Blueprint/UserWidget.h"
#include "Camera/CameraComponent.h"
#include "Components/DecalComponent.h"
#include "Components/SphereComponent.h"
#include "GameFramework/InputDeviceSubsystem.h"
#include "HaloFloodFanGame01/FirefightGamemode.h"
#include "Net/UnrealNetwork.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISense_Damage.h"
#include "Perception/AISense_Sight.h"
#include "Perception/AISense_Touch.h"

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

	InteractionSphere = CreateDefaultSubobject<USphereComponent>(TEXT("Interaction Sphere"));
	InteractionSphere->SetupAttachment(GetRootComponent());
	InteractionSphere->SetSphereRadius(500);
	InteractionSphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
}

// Called when the game starts or when spawned
void ACharacterBase::BeginPlay()
{
	Super::BeginPlay();
	GetCapsuleComponent()->OnComponentHit.AddDynamic(this, &ACharacterBase::OnHit);

	GetMesh()->OnComponentSleep.AddDynamic(this, &ACharacterBase::RagdollSettled);
	SpawnWeapons();
	//UE_LOG(LogTemp, Warning, TEXT("Char: %s %f"), *GetActorLabel(), GetHealthComponent()->GetHealth());
	//if (GetHealthComponent()) UE_LOG(LogTemp, Warning, TEXT("%s's Health component is owned by %s (Should be %s)"), *GetActorLabel(), *GetHealthComponent()->GetOwner()->GetActorLabel(), *GetActorLabel());
	if (GetHealthComponent()) GetHealthComponent()->OnHealthDepleted.AddDynamic(this, &ACharacterBase::OnHealthDepleted);

	InputDeviceSubsystem = GetGameInstance()->GetEngine()->GetEngineSubsystem<UInputDeviceSubsystem>();
}

void ACharacterBase::Restart()
{
	Super::Restart();

	// SpawnWeapons();
	Cast<IGenericTeamAgentInterface>(GetController())->SetGenericTeamId(FGenericTeamId(TeamId));
}

void ACharacterBase::SpawnWeapons()
{
	Server_SpawnWeapons();
}

void ACharacterBase::Server_SpawnWeapons_Implementation()
{
	if (EquippedWeaponClass)
	{
		AGunBase* Gun = GetWorld()->SpawnActor<AGunBase>(EquippedWeaponClass);
		if (Gun) UE_LOG(LogTemp, Warning, TEXT("Sucessfully spawned equipped weapon"));
		PickupWeapon(Gun);
		
	}
	if (HolsteredWeaponClass)
	{
		AGunBase* Gun = GetWorld()->SpawnActor<AGunBase>(HolsteredWeaponClass);
		if (Gun) UE_LOG(LogTemp, Warning, TEXT("Sucessfully spawned holstered weapon"));
		PickupWeapon(Gun);
	}
}

void ACharacterBase::Multi_SpawnWeapons_Implementation()
{
	
}

// Called every frame
void ACharacterBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	MeleeTimeline.TickTimeline(DeltaTime);
	if (GetController())
		GetMesh1P()->SetWorldRotation(FRotator(0, GetViewRotation().Yaw + 90, GetViewRotation().Pitch));

	SetCurrentInteractable();
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


	const float AimAssistMultiplier = AimAssist();
	float ScopeSenseMultiplier;
	if (EquippedWeapon && EquippedWeapon->ScopeActive)
	{
		ScopeSenseMultiplier = (EquippedWeapon->ZoomFOV/90);
	} else
	{
		ScopeSenseMultiplier = 1;
	}
	
	// add yaw and pitch input to controller
	AddControllerYawInput(LookAxisVector.X * AimAssistMultiplier * ScopeSenseMultiplier);
	AddControllerPitchInput(LookAxisVector.Y * AimAssistMultiplier * ScopeSenseMultiplier);
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
	DOREPLIFETIME(ACharacterBase, EquippedWeaponClass);
	DOREPLIFETIME(ACharacterBase, HolsteredWeaponClass);
	DOREPLIFETIME(ACharacterBase, CurGrenadeTypeI);
	DOREPLIFETIME(ACharacterBase, GrenadeInventory);
	//DOREPLIFETIME(ACharacterBase, HealthComponent);
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

		EnhancedInputComponent->BindAction(SwitchGrenadeAction, ETriggerEvent::Triggered, this, &ACharacterBase::SwitchGrenadeType);
		
		EnhancedInputComponent->BindAction(ThrowGrenadeAction, ETriggerEvent::Triggered, this, &ACharacterBase::ThrowEquippedGrenade);

		EnhancedInputComponent->BindAction(UseEquipmentAction, ETriggerEvent::Triggered, this, &ACharacterBase::UseEquipment);
		
		EnhancedInputComponent->BindAction(ScopeAction, ETriggerEvent::Triggered, this, &ACharacterBase::ScopeWeapon);
	}

}

float ACharacterBase::CustomTakeRadialDamage_Implementation(float Force, FRadialDamageEvent const& RadialDamageEvent,
	AController* EventInstigator, AActor* DamageCauser)
{
	return ChangeHealth(this, RadialDamageEvent.Params.BaseDamage, (Cast<AActor>(this)->GetActorLocation() - RadialDamageEvent.Origin).GetSafeNormal() * Force, FVector(0,0,0), FName(""), EventInstigator, DamageCauser);
}

float ACharacterBase::CustomTakeDamage_Implementation(float DamageAmount, FVector Force,
                                                      FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
	return ChangeHealth(this, DamageAmount, Force, FVector(0,0,0), FName(""), EventInstigator, DamageCauser);
}

// float ACharacterBase::CustomOnTakeAnyDamage_Implementation(float DamageAmount, FVector Force,
// 	AController* EventInstigator, AActor* DamageCauser)
// {
// 	return IDamageableInterface::CustomOnTakeAnyDamage(DamageAmount, Force, EventInstigator, DamageCauser);
// }

float ACharacterBase::CustomTakePointDamage_Implementation(FPointDamageEvent const& PointDamageEvent, float Force, AController* EventInstigator, AActor* DamageCauser)
{
	if (!HasAuthority()) return 0;
	float x = IDamageableInterface::ChangeHealth(this, PointDamageEvent.Damage, PointDamageEvent.ShotDirection * Force, PointDamageEvent.HitInfo.Location, PointDamageEvent.HitInfo.BoneName, EventInstigator, DamageCauser);
	//float x = IDamageableInterface::CustomTakePointDamage(PointDamageEvent, Force, EventInstigator, DamageCauser);
	if (EventInstigator && Cast<AAIControllerBase>(GetController()))
	{
		UAISense_Damage::ReportDamageEvent(GetWorld(), this, EventInstigator->GetPawn(), PointDamageEvent.Damage, Cast<AActor>(EventInstigator)->GetActorLocation(), PointDamageEvent.HitInfo.Location);
	}
	
	if (IDamageableInterface::Execute_GetHealthComponent(this)->GetShields() <= 0)
	{
		if (HurtAnim)
		{
			StunAmount = StunAmount + (Force/50);
			if (StunAmount >= 100  && !GetMesh()->GetAnimInstance()->Montage_IsPlaying(HurtAnim))
			{
				Stun();
			}
		}
		if (BloodPFX)
		{
			UNiagaraComponent* BloodNiagaraComponent = UNiagaraFunctionLibrary::SpawnSystemAttached(BloodPFX, GetMesh(), PointDamageEvent.HitInfo.BoneName, PointDamageEvent.HitInfo.ImpactPoint, PointDamageEvent.HitInfo.Normal.Rotation(), EAttachLocation::KeepWorldPosition, true);
			//BloodNiagaraComponent->SetNiagaraVariableActor("Character", this);
			BloodNiagaraComponent->SetVariableActor("Character", this);
		}
		if (BloodSplatterMat)
		{
			UGameplayStatics::SpawnDecalAttached(BloodSplatterMat, FVector(10,10,10), GetMesh(),
			PointDamageEvent.HitInfo.BoneName, PointDamageEvent.HitInfo.Location, PointDamageEvent.HitInfo.Normal.Rotation() + FRotator(-90, 0, FMath::RandRange(-180, 180)), EAttachLocation::KeepWorldPosition, 0);
		}
		if (BloodDecalMaterial)
		{
			float DecalSize = FMath::RandRange(10, 130);

			FHitResult HitResult;
			FCollisionQueryParams QueryParams;
			QueryParams.AddIgnoredActor(this);
			GetWorld()->LineTraceSingleByChannel(HitResult, PointDamageEvent.HitInfo.Location, PointDamageEvent.HitInfo.Location + (PointDamageEvent.ShotDirection * 4000),ECollisionChannel::ECC_Visibility, QueryParams);
			if (HitResult.bBlockingHit)
			{
				GetWorld()->GetSubsystem<UWorldCleanupManager>()->ManageDecal(UGameplayStatics::SpawnDecalAttached(BloodDecalMaterial, FVector(DecalSize,DecalSize,DecalSize), HitResult.GetComponent(), HitResult.BoneName, HitResult.Location, HitResult.Normal.Rotation() + FRotator(-180,0,FMath::RandRange(-180, 180)), EAttachLocation::KeepWorldPosition));
				//Cast<AHaloGameState>(GetWorld()->GetGameState())->ManageDecal(UGameplayStatics::SpawnDecalAttached(BloodDecalMaterial, FVector(DecalSize,DecalSize,DecalSize), HitResult.GetComponent(), HitResult.BoneName, HitResult.Location, HitResult.Normal.Rotation() + FRotator(-180,0,FMath::RandRange(-180, 180)), EAttachLocation::KeepWorldPosition));
			}
		}
	}
	return x;
}


UHealthComponent* ACharacterBase::GetHealthComponent_Implementation()
{
	return HealthComponent;
}


void ACharacterBase::OnHealthDepleted_Implementation(float Damage, FVector DamageForce, FVector HitLocation, FName HitBoneName, AController* EventInstigator, AActor* DamageCauser)
{
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
			APlayerController* PC = PlayerController;
			PlayerController->UnPossess();
			Cast<AFirefightGameMode>(GetWorld()->GetAuthGameMode())->OnPlayerCharDied.Broadcast(this, Cast<APlayerControllerBase>(PC));
		}
	}
	MC_OnHealthDepleted(Damage, Force, HitLocation, HitBoneName, EventInstigator, DamageCauser);
}

void ACharacterBase::MC_OnHealthDepleted_Implementation(float Damage, FVector Force, FVector HitLocation,
	FName HitBoneName, AController* EventInstigator, AActor* DamageCauser)
{
	if (EquippedWeapon)
		DropWeapon();
	if (BloodDecalMaterial)
		GetWorld()->GetSubsystem<UWorldCleanupManager>()->ManageDecal(UGameplayStatics::SpawnDecalAtLocation(GetWorld(), BloodDecalMaterial, FVector(100, 100, 100), GetActorLocation(), FRotator(-90,0,0)));
		//Cast<AHaloGameState>(GetWorld()->GetGameState())->ManageDecal(UGameplayStatics::SpawnDecalAtLocation(GetWorld(), BloodDecalMaterial, FVector(100, 100, 100), GetActorLocation(), FRotator(-90,0,0)));
	if (DeathSound)
		UGameplayStatics::PlaySoundAtLocation(GetWorld(), DeathSound, GetActorLocation());
	GetMesh()->GetAnimInstance()->Montage_Play(DeathAnim);
	GetCapsuleComponent()->DestroyComponent();
	SetRootComponent(GetMesh());
	GetMesh()->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	GetMesh()->SetSimulatePhysics(true);
	GetMesh()->AddImpulseAtLocation(Force, HitLocation, HitBoneName);
	OnKilled.Broadcast(this, EventInstigator, DamageCauser);
	DropGrenades();
	GetWorld()->GetSubsystem<UWorldCleanupManager>()->ManageRagdoll(this);
	//Cast<AHaloGameState>(GetWorld()->GetGameState())->ManageRagdoll(this);
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
	float DamageCalculation;
	
	if (OtherComp)
	{
		UAISense_Touch::ReportTouchEvent(GetWorld(), this, OtherActor, Hit.Location);
		const float VelocityDifference = FMath::Abs(OtherComp->GetComponentVelocity().Length() - this->GetVelocity().Length());
		const float Mass = OtherComp->IsSimulatingPhysics() ? (OtherComp->GetMass()) : 1;
		DamageCalculation = FMath::Pow(VelocityDifference, 1.0f/3.0f) * (Mass/300);
		if (DamageCalculation > 5)
		{
			FDamageEvent DamageEvent = FDamageEvent(UDamageType::StaticClass());
			CustomTakeDamage(DamageCalculation, NormalImpulse, DamageEvent, nullptr, nullptr);
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



void ACharacterBase::MeleeDamageCode()
{
	FPointDamageEvent PointDamageEvent;
	PointDamageEvent.Damage = MeleeDamage;
	PointDamageEvent.HitInfo = MeleeHit;
	FVector Dir = MeleeHit.Location - MeleeHit.TraceStart;
	Dir.Normalize();
	IDamageableInterface::Execute_CustomTakePointDamage(MeleeHit.GetActor(), PointDamageEvent, MeleeForce, GetInstigatorController(), this);
	//HitActor->CustomTakePointDamage(PointDamageEvent, MeleeForce);
}

void ACharacterBase::MeleeUpdate(float Alpha)
{
	SetActorLocation(FMath::Lerp(StartMeleeLoc, EndMeleeLoc, Alpha));
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
	if (GetWorld()->GetTimerManager().TimerExists(MeleeTimer)) return;
	GetWorld()->GetTimerManager().SetTimer(MeleeTimer, 1, false);
	
	FVector TraceStart = GetFirstPersonCameraComponent()->GetComponentLocation();
	FVector TraceEnd = GetFirstPersonCameraComponent()->GetComponentLocation() + GetFirstPersonCameraComponent()->GetForwardVector()*500;
	FCollisionQueryParams CollisionParameters;
	CollisionParameters.AddIgnoredActor(this);
	CollisionParameters.AddIgnoredActor(GetAttachParentActor());
	GetWorld()->LineTraceSingleByChannel(MeleeHit, TraceStart, TraceEnd, ECollisionChannel::ECC_Visibility, CollisionParameters);
	DrawDebugLine(GetWorld(), TraceStart, TraceEnd, FColor(255, 0, 0), false, 3);
	
	if (MeleeHit.GetActor())
	{
		if (MeleeHit.GetActor()->Implements<UDamageableInterface>())
		{
			if (IDamageableInterface::Execute_GetHealthComponent(MeleeHit.GetActor())->GetHealth() > 0) {
				if (MeleeCurve)
				{
					StartMeleeLoc = GetActorLocation();
					EndMeleeLoc = MeleeHit.GetActor()->GetActorLocation();
					
					FOnTimelineFloat TimelineCallback;
					FOnTimelineEventStatic TimelineFinishedCallback;
					
					TimelineCallback.BindUFunction(this, FName("MeleeUpdate"));
					TimelineFinishedCallback.BindUFunction(this, FName("MeleeDamageCode"));
					
					MeleeTimeline.AddInterpFloat(MeleeCurve, TimelineCallback);
					MeleeTimeline.SetTimelineFinishedFunc(TimelineFinishedCallback);
					MeleeTimeline.SetPlayRate(10);
					MeleeTimeline.PlayFromStart();
				}
			}
		}
		UPrimitiveComponent* HitComp = MeleeHit.GetComponent();
		
		if (HitComp && HitComp->IsSimulatingPhysics())
		{
			FVector ForceVector = (HitComp->GetComponentLocation() - GetActorLocation());
			ForceVector.Normalize();
			HitComp->AddImpulse(ForceVector*MeleeForce);
		}
	}
}

void ACharacterBase::NPCMelee_Implementation()
{
	//GetMesh()->GetAnimInstance()->Montage_Play(MeleeAnim);
	if (!CanMelee()) return;
	GetWorld()->GetTimerManager().SetTimer(MeleeTimer, 1, false);
	UE_LOG(LogTemp, Warning, TEXT("Melee"));
	FCollisionShape BoxShape = FCollisionShape::MakeBox(FVector(50, 50, 50));
	TArray<FHitResult> SweepResult;
	FCollisionQueryParams CollisionParams;
	CollisionParams.AddIgnoredActor(this);
	//GetWorld()->SweepMultiByChannel(SweepResult, GetActorLocation(), GetActorLocation() + GetActorForwardVector()*100, FQuat(0,0,0,0), ECollisionChannel::ECC_Pawn, BoxShape, CollisionParams);
	//GetWorld()->SweepMultiByChannel(SweepResult, GetActorLocation(), GetActorLocation() + GetActorForwardVector()*100, FQuat(0,0,0,0), ECollisionChannel::ECC_Pawn, BoxShape, CollisionParams);
	GetWorld()->SweepMultiByObjectType(SweepResult, GetActorLocation(), GetActorLocation() + GetActorForwardVector()*100, FQuat(0,0,0,0), ECollisionChannel::ECC_Pawn, BoxShape, CollisionParams);
	for (auto Result : SweepResult)
	{
		
		if (Result.GetActor() && Result.GetActor()->Implements<UDamageableInterface>())
		{
			UE_LOG(LogTemp, Warning, TEXT("Melee'd Actor: %s"), *Result.GetComponent()->GetName());
			FDamageEvent DamageEvent;
			Cast<IDamageableInterface>(Result.GetActor())->CustomTakeDamage(MeleeDamage, FVector(0,0,0), DamageEvent, nullptr, this);
			//Result.GetActor()->TakeDamage(MeleeDamage, DamageEvent, nullptr, this);
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

void ACharacterBase::ThrowEquippedGrenade_Implementation()
{
	if (IsPlayerControlled())
	{
		if (GrenadeInventory.Num() <= 0) return;
		if (ThrowGrenadeAnimation1P)
			GetMesh1P()->GetAnimInstance()->Montage_Play(ThrowGrenadeAnimation1P);
		GrenadeInventory[CurGrenadeTypeI].GrenadeAmount -= 1;
		const FTransform SpawnTransform = FTransform(GetFirstPersonCameraComponent()->GetForwardVector().Rotation(), GetFirstPersonCameraComponent()->GetComponentLocation() + GetFirstPersonCameraComponent()->GetForwardVector()*300);
		FActorSpawnParameters ActorSpawnParameters;
		ActorSpawnParameters.Instigator = this;
		ActorSpawnParameters.Owner = this;

		if (AGrenadeBase* Grenade = Cast<AGrenadeBase>(GetWorld()->SpawnActorDeferred<AGrenadeBase>(GrenadeInventory[CurGrenadeTypeI].GrenadeClass, SpawnTransform, this, this, ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn)))
		{
			Grenade->SetInstigator(this);
			Grenade->SetArmed(true);
			Grenade->FinishSpawning(SpawnTransform);
			UGameplayStatics::SpawnSoundAtLocation(GetWorld(), Grenade->ThrowSFX, Grenade->GetActorLocation());
			FVector Direction = GetFirstPersonCameraComponent()->GetForwardVector() + FVector(0,0,0.15);
			Direction.Normalize();
			Grenade->Mesh->AddImpulse(Direction*2000.0f, NAME_None, true);
			Grenade->Mesh->AddAngularImpulseInDegrees(Grenade->GetActorRightVector().GetSafeNormal()*1000 , NAME_None, true);
	
			if (GrenadeInventory[CurGrenadeTypeI].GrenadeAmount <= 0)
			{
				GrenadeInventory.RemoveAt(CurGrenadeTypeI);
				SwitchToGrenadeType(CurGrenadeTypeI);
			}
			OnGrenadeInventoryUpdated.Broadcast(GrenadeInventory);
		}
	} else
	{
		if (GrenadeInventory[CurGrenadeTypeI].GrenadeAmount <= 0) return;
		OnGrenadeInventoryUpdated.Broadcast(GrenadeInventory);
		FVector EyesLoc;
		FRotator EyesRot;
		GetActorEyesViewPoint(EyesLoc, EyesRot);
		FActorSpawnParameters ActorSpawnParameters;
		ActorSpawnParameters.Owner = this;
		AGrenadeBase* Grenade = Cast<AGrenadeBase>(GetWorld()->SpawnActor(GrenadeInventory[CurGrenadeTypeI].GrenadeClass, &EyesLoc, &EyesRot, ActorSpawnParameters));
		Grenade->SetInstigator(this);
		Grenade->SetArmed(true);
		UGameplayStatics::SpawnSoundAtLocation(GetWorld(), Grenade->ThrowSFX, Grenade->GetActorLocation());
		FVector Force = GetBaseAimRotation().Vector() + FVector(0,0,0.1);
		Grenade->Mesh->AddImpulse(Force*20000);
	}
}


void ACharacterBase::SwitchGrenadeType()
{
	SwitchToGrenadeType(CurGrenadeTypeI+1);
}

void ACharacterBase::SwitchToGrenadeType_Implementation(int Index = 0)
{
	if (GrenadeInventory.Num() <= 0) return;
	
	CurGrenadeTypeI = Index;
	CurGrenadeTypeI = CurGrenadeTypeI % (GrenadeInventory.Num());
	OnGrenadeTypeSwitched.Broadcast(GrenadeInventory[CurGrenadeTypeI].GrenadeClass);
}

void ACharacterBase::UseEquipment()
{
	UE_LOG(LogTemp, Warning, TEXT("Used Equipment"));
}

void ACharacterBase::PrimaryAttack_Pull()
{

	Server_PrimaryAttack_Pull();
	
}

void ACharacterBase::Server_PrimaryAttack_Pull_Implementation()
{
	Multi_PrimaryAttack_Pull();
}

void ACharacterBase::Multi_PrimaryAttack_Pull_Implementation()
{

	if (EquippedWeapon)
		EquippedWeapon->PullTrigger();
	
}

void ACharacterBase::PrimaryAttack_Release()
{
	Server_PrimaryAttack_Release();
}

void ACharacterBase::Server_PrimaryAttack_Release_Implementation()
{
	Multi_PrimaryAttack_Release();
}

void ACharacterBase::Multi_PrimaryAttack_Release_Implementation()
{
	if (EquippedWeapon)
		EquippedWeapon->ReleaseTrigger();
}

void ACharacterBase::ReloadWeapon()
{
	Server_ReloadWeapon();
}

void ACharacterBase::Server_ReloadWeapon_Implementation()
{
	Multi_ReloadWeapon();
}

void ACharacterBase::Multi_ReloadWeapon_Implementation()
{
	if (EquippedWeapon)
	{
		EquippedWeapon->StartReload();
	}
}

void ACharacterBase::SwitchWeapon()
{
	Server_SwitchWeapon();
}

void ACharacterBase::Server_SwitchWeapon_Implementation()
{
	Multi_SwitchWeapon();
}

void ACharacterBase::Multi_SwitchWeapon_Implementation()
{
	if (!(EquippedWeapon && HolsteredWeapon))
		return;
	HolsterWeapon(EquippedWeapon);
	
	AGunBase* TempGun = EquippedWeapon;
	EquippedWeapon = HolsteredWeapon;
	HolsteredWeapon = TempGun;
	
	// EquipWeapon(EquippedWeapon);
	// WeaponsUpdated.Broadcast(EquippedWeapon, HolsteredWeapon);

	GetWorld()->GetTimerManager().SetTimer(HolsterHandle, FTimerDelegate::CreateUObject(this, &ACharacterBase::EquipWeapon, EquippedWeapon), HolsteredWeapon->HolsterSpeed, false);
	
}

void ACharacterBase::ScopeWeapon()
{
	if (EquippedWeapon)
	{
		if (EquippedWeapon->ScopeActive)
		{
			EquippedWeapon->ScopeOut();
		} else
		{
			EquippedWeapon->ScopeIn();
		}
	}
}

void ACharacterBase::EquipWeapon(AGunBase* Gun)
{
	//EquippedWeapon = Gun;
	UE_LOG(LogTemp, Warning, TEXT("EquipWeapon called"));
	EquippedWeapon->Mesh->SetSimulatePhysics(false);
	EquippedWeapon->SetActorHiddenInGame(false);
	EquippedWeapon->AttachToComponent(GetMesh(), FAttachmentTransformRules::SnapToTargetNotIncludingScale, "GripPoint");
	EquippedWeapon->OnEquipped();

	if (HolsteredWeapon)
		HolsteredWeapon->SetActorHiddenInGame(true);
	//PlayerCharacter

	if (IsPlayerControlled() && IsLocallyControlled())
	{
		if (EquippedWeapon->DrawAnimation1P)
			GetMesh1P()->GetAnimInstance()->Montage_Play(EquippedWeapon->DrawAnimation1P, EquippedWeapon->DrawAnimation1P->GetPlayLength() / EquippedWeapon->DrawSpeed);
	}
	SetupViewmodel(true);
	
	WeaponsUpdated.Broadcast(EquippedWeapon, HolsteredWeapon);
}

void ACharacterBase::SetupViewmodel_Implementation(bool FirstPerson)
{
	if (EquippedWeapon)
	{
		if (IsPlayerControlled() && IsLocallyControlled() && FirstPerson)
		{
			EquippedWeapon->AttachToComponent(Mesh1P, FAttachmentTransformRules::SnapToTargetNotIncludingScale, "GripPoint");
		} else
		{
			EquippedWeapon->AttachToComponent(GetMesh(), FAttachmentTransformRules::SnapToTargetNotIncludingScale, "GripPoint");
		}
	}
	
}

void ACharacterBase::HolsterWeapon(AGunBase* Gun)
{
	//EquippedWeapon = nullptr;
	HolsteredWeapon->ReleaseTrigger();
	GetWorldTimerManager().ClearTimer(HolsteredWeapon->ReloadTimer);
	HolsteredWeapon->bReloading = false;
	//Gun->SetActorHiddenInGame(true);

	//PlayerCharacter
	if (IsLocallyControlled())
	{
		if (HolsteredWeapon->HolsterAnimation1P)
			GetMesh1P()->GetAnimInstance()->Montage_Play(EquippedWeapon->HolsterAnimation1P, EquippedWeapon->HolsterAnimation1P->GetPlayLength() / EquippedWeapon->HolsterSpeed);
	}
}

void ACharacterBase::PickupWeapon(AGunBase* Gun)
{
	Server_PickupWeapon(Gun);
}

void ACharacterBase::Server_PickupWeapon_Implementation(AGunBase* Gun)
{
	// Server_PickupWeapon(Gun);
	Gun->SetOwner(this);
	Gun->Mesh->SetSimulatePhysics(false);
	Gun->SetActorEnableCollision(false);
	Gun->OnPickup(this);
	
	if (!EquippedWeapon)
	{
		EquippedWeapon = Gun;
	} else if (!HolsteredWeapon)
	{
		HolsteredWeapon = Gun;
		Gun->SetActorHiddenInGame(true);
	} else
	{
		DropWeapon();
		EquippedWeapon = Gun;
	}
	// EquipWeapon(EquippedWeapon);
	if (EquippedWeapon)
	{
		WeaponsUpdated.Broadcast(EquippedWeapon, HolsteredWeapon);
	}
	// EquipWeapon(EquippedWeapon);
	// Multi_PickupWeapon(Gun);
}

void ACharacterBase::Multi_PickupWeapon_Implementation(AGunBase* Gun)
{
	Gun->Mesh->SetSimulatePhysics(false);
	Gun->SetActorEnableCollision(false);
	Gun->OnPickup(this);
	
	if (!EquippedWeapon)
	{
		EquippedWeapon = Gun;
	} else if (!HolsteredWeapon)
	{
		HolsteredWeapon = Gun;
		Gun->SetActorHiddenInGame(true);
	} else
	{
		DropWeapon();
		EquippedWeapon = Gun;
	}
	EquipWeapon(EquippedWeapon);
}

void ACharacterBase::DropWeapon()
{
	EquippedWeapon->ReleaseTrigger();
	EquippedWeapon->DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
	EquippedWeapon->SetActorEnableCollision(true);
	EquippedWeapon->Mesh->SetSimulatePhysics(true);
	EquippedWeapon->Mesh->AddImpulse(GetControlRotation().Vector() * 300, NAME_None, true);
	EquippedWeapon->OnDropped();
	EquippedWeapon = nullptr;
	WeaponsUpdated.Broadcast(EquippedWeapon, HolsteredWeapon);
}

void ACharacterBase::RagdollSettled(UPrimitiveComponent* Component, FName Name)
{

	GetMesh()->PutAllRigidBodiesToSleep();
	//GetMesh()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Ignore);
	
}

void ACharacterBase::Stun(float StunTime)
{
	StunAmount = 0;
	float StunDuration = HurtAnim->CalculateSequenceLength();
	GetMesh()->GetAnimInstance()->Montage_Play(HurtAnim);
	if (AAIControllerBase* AIC = Cast<AAIControllerBase>(GetController()))
	{
		AIC->BehaviorTreeComp->PauseLogic(FString("Stunned"));
		AIC->StopMovement();
		AIC->BlackboardComp->SetValueAsBool("IsStunned", true);
		AIC->ClearFocus(EAIFocusPriority::Gameplay);
		GetWorld()->GetTimerManager().SetTimer(StunTimer, this, &ACharacterBase::Unstun, StunDuration, false);
	}
}

void ACharacterBase::Unstun()
{
	if (AAIControllerBase* AIC = Cast<AAIControllerBase>(GetController()))
	{
		AIC->BehaviorTreeComp->ResumeLogic(FString("Unstunned"));
		AIC->BlackboardComp->SetValueAsBool("IsStunned", false);
	}
}

void ACharacterBase::SetCurrentInteractable()
{
	if (!IsPlayerControlled() || !CanInteract) return;
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
		InteractionSphere->GetOverlappingActors(Actors);
		
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
		OnInteractableChanged.Broadcast(InteractableActor);
	}
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

void ACharacterBase::Multi_Interact_Implementation()
{
	
}

void ACharacterBase::NotifyRestarted()
{
	Super::NotifyRestarted();
	if (APlayerController* PC = Cast<APlayerController>(GetController()))
	{
		PlayerController = PC;
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PC->GetLocalPlayer()))
		{
			Subsystem->AddMappingContext(DefaultMappingContext, 0);
		}
		if (IsLocallyControlled())
		{
			if (PlayerHUDClass && !PlayerHUD) {
				PlayerHUD = CreateWidget<UUserWidget>(PC, PlayerHUDClass);
				PlayerHUD->AddToPlayerScreen();
			}
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

