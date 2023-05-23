// Copyright Epic Games, Inc. All Rights Reserved.

#include "HaloFloodFanGame01Character.h"

#include "BaseGrenade.h"
#include "GunBase.h"
#include "Animation/AnimInstance.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "FrameTypes.h"
#include "HaloHUDWidget.h"
#include "HealthComponent.h"
#include "InteractableInterface.h"
#include "Blueprint/UserWidget.h"
#include "Components/TextBlock.h"
#include "Components/TimelineComponent.h"
#include "Engine/DamageEvents.h"
#include "GameFramework/SpectatorPawn.h"


//////////////////////////////////////////////////////////////////////////
// AHaloFloodFanGame01Character



AHaloFloodFanGame01Character::AHaloFloodFanGame01Character()
{
	// Set size for collision capsule
	//GetCapsuleComponent()->InitCapsuleSize(55.f, 96.0f);
		
	// Create a CameraComponent	
	FirstPersonCameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("FirstPersonCamera"));
	FirstPersonCameraComponent->SetupAttachment(GetCapsuleComponent());
	FirstPersonCameraComponent->SetRelativeLocation(FVector(-10.f, 0.f, 60.f)); // Position the camera
	FirstPersonCameraComponent->bUsePawnControlRotation = true;

	// Create a mesh component that will be used when being viewed from a '1st person' view (when controlling this pawn)
	Mesh1P = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("CharacterMesh1P"));
	Mesh1P->SetOnlyOwnerSee(true);
	Mesh1P->SetupAttachment(FirstPersonCameraComponent);
	Mesh1P->bCastDynamicShadow = false;
	Mesh1P->CastShadow = false;
	//Mesh1P->SetRelativeRotation(FRotator(0.9f, -19.19f, 5.2f));
	Mesh1P->SetRelativeLocation(FVector(-30.f, 0.f, -150.f));
}

void AHaloFloodFanGame01Character::BeginPlay()
{
	// Call the base class  
	Super::BeginPlay();

	if (HolsteredGunClass)
		PickupWeapon(Cast<AGunBase>(GetWorld()->SpawnActor(HolsteredGunClass)));

	//Add Input Mapping Context
	
}

FHitResult PlayerAim;
void AHaloFloodFanGame01Character::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	MeleeTimeline.TickTimeline(DeltaSeconds);
	FVector TraceStart = FirstPersonCameraComponent->GetComponentLocation();
	FVector TraceEnd = FirstPersonCameraComponent->GetComponentLocation() + FirstPersonCameraComponent->GetForwardVector()*1000.0f;
	FCollisionQueryParams CollisionParameters;
	CollisionParameters.AddIgnoredActor(this);
	GetWorld()->LineTraceSingleByChannel(PlayerAim, TraceStart, TraceEnd, ECollisionChannel::ECC_Visibility, CollisionParameters);

	

	if (PlayerHUD)
	{
		

		
		PlayerHUD->SetCompassDirection(FirstPersonCameraComponent->GetComponentRotation().Yaw);
		PlayerHUD->SetShields(HealthComponent->GetShields(), HealthComponent->GetMaxShields());
		PlayerHUD->SetHealth(HealthComponent->GetHealth(), HealthComponent->GetMaxHealth());
	}
}

FHitResult AHaloFloodFanGame01Character::GetPlayerAim()
{
	return PlayerAim;
}


//////////////////////////////////////////////////////////////////////////// Input

void AHaloFloodFanGame01Character::SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent)
{
	// Set up action bindings
	if (UEnhancedInputComponent* EnhancedInputComponent = CastChecked<UEnhancedInputComponent>(PlayerInputComponent))
	{
		//Jumping
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Triggered, this, &ACharacter::Jump);
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Completed, this, &ACharacter::StopJumping);

		//Moving
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &AHaloFloodFanGame01Character::Move);

		//Looking
		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &AHaloFloodFanGame01Character::Look);

		EnhancedInputComponent->BindAction(InteractAction, ETriggerEvent::Triggered, this, &AHaloFloodFanGame01Character::Interact);

		EnhancedInputComponent->BindAction(PrimaryAttackAction, ETriggerEvent::Started, this, &AHaloFloodFanGame01Character::PrimaryAttack_Pull);

		EnhancedInputComponent->BindAction(PrimaryAttackAction, ETriggerEvent::Completed, this, &AHaloFloodFanGame01Character::PrimaryAttack_Release);
		
		EnhancedInputComponent->BindAction(SwitchWeaponAction, ETriggerEvent::Triggered, this, &AHaloFloodFanGame01Character::SwitchWeapon);

		EnhancedInputComponent->BindAction(ReloadAction, ETriggerEvent::Triggered, this, &AHaloFloodFanGame01Character::ReloadInput);
		
		EnhancedInputComponent->BindAction(MeleeAction, ETriggerEvent::Triggered, this, &AHaloFloodFanGame01Character::Melee);
		
		EnhancedInputComponent->BindAction(ThrowGrenadeAction, ETriggerEvent::Triggered, this, &AHaloFloodFanGame01Character::ThrowEquippedGrenade);

		EnhancedInputComponent->BindAction(UseEquipmentAction, ETriggerEvent::Triggered, this, &AHaloFloodFanGame01Character::UseEquipment);
	}
}


void AHaloFloodFanGame01Character::Move(const FInputActionValue& Value)
{
	// input is a Vector2D
	FVector2D MovementVector = Value.Get<FVector2D>();
	if (Controller != nullptr)
	{
		// add movement 
		AddMovementInput(GetActorForwardVector(), MovementVector.Y);
		AddMovementInput(GetActorRightVector(), MovementVector.X);
	}
}

void AHaloFloodFanGame01Character::Look(const FInputActionValue& Value)
{
	// input is a Vector2D
	FVector2D LookAxisVector = Value.Get<FVector2D>();

	if (Controller != nullptr)
	{
		// add yaw and pitch input to controller
		AddControllerYawInput(LookAxisVector.X);
		AddControllerPitchInput(LookAxisVector.Y);
	}
}

FVector StartMeleeLoc;
FVector EndMeleeLoc;
FHitResult MeleeHit;
void AHaloFloodFanGame01Character::Melee_Implementation()
{
	FVector TraceStart = GetFirstPersonCameraComponent()->GetComponentLocation();
	FVector TraceEnd = GetFirstPersonCameraComponent()->GetComponentLocation() + GetFirstPersonCameraComponent()->GetForwardVector()*500;
	FCollisionQueryParams CollisionParameters;
	CollisionParameters.AddIgnoredActor(this);
	CollisionParameters.AddIgnoredActor(GetAttachParentActor());
	GetWorld()->LineTraceSingleByChannel(MeleeHit, TraceStart, TraceEnd, ECollisionChannel::ECC_Visibility, CollisionParameters);
	DrawDebugLine(GetWorld(), TraceStart, TraceEnd, FColor(255, 0, 0), false, 3);
	
	if (MeleeHit.GetActor())
	{
		IDamageableInterface* DamageableActor = Cast<IDamageableInterface>(MeleeHit.GetActor());
		if (DamageableActor && DamageableActor->GetHealthComponent()->GetHealth() > 0)
		{
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
		UPrimitiveComponent* HitComp = MeleeHit.GetComponent();
		
		if (HitComp && HitComp->IsSimulatingPhysics())
		{
			FVector ForceVector = (HitComp->GetComponentLocation() - GetActorLocation());
			ForceVector.Normalize();
			HitComp->AddImpulse(ForceVector*MeleeForce);
		}
	}
}

void AHaloFloodFanGame01Character::MeleeDamageCode()
{
	IDamageableInterface* HitActor = Cast<IDamageableInterface>(MeleeHit.GetActor());
	FPointDamageEvent PointDamageEvent;
	PointDamageEvent.Damage = MeleeDamage;
	PointDamageEvent.HitInfo = MeleeHit;
	FVector Dir = MeleeHit.Location - MeleeHit.TraceStart;
	Dir.Normalize();
	HitActor->TakePointDamage(MeleeDamage, Dir*MeleeForce, PointDamageEvent);
}

void AHaloFloodFanGame01Character::MeleeUpdate(float Alpha)
{
	SetActorLocation(FMath::Lerp(StartMeleeLoc, EndMeleeLoc, Alpha));
}

void AHaloFloodFanGame01Character::HealthDepleted(float Damage, FVector Force, FVector HitLocation, FName HitBoneName)
{
	//PlayerHUD->RemoveFromParent();
	if (PlayerController)
	{
		PlayerController->UnPossess();
		if (PlayerHUD)
			PlayerHUD->RemoveFromParent();
		FVector Loc = GetFirstPersonCameraComponent()->GetComponentLocation();
		FRotator Rot = GetFirstPersonCameraComponent()->GetComponentRotation();
		ASpectatorPawn* SpectatorPawn = Cast<ASpectatorPawn>(GetWorld()->SpawnActor(ASpectatorPawn::StaticClass(), &Loc, &Rot));
		PlayerController->Possess(SpectatorPawn);
	}
		
	
	// FTimerHandle RestartTimer;
	// GetWorldTimerManager().SetTimer(RestartTimer, this, &AHaloFloodFanGame01Character::Attack, 5, false);
	// GetWorldTimerManager().SetTimer(RestartTimer, UGameplayStatics::GetGameMode(GetWorld())->RestartPlayer(PC), 5, false);
	Super::HealthDepleted(Damage, Force, HitLocation, HitBoneName);
}

void AHaloFloodFanGame01Character::ThrowEquippedGrenade_Implementation()
{
	if (!EquippedGrenadeClass) return;
	if (FragCount<=0) return;
	SetFragCount(FragCount-1);
	FVector SpawnLoc = GetFirstPersonCameraComponent()->GetComponentLocation() + GetFirstPersonCameraComponent()->GetForwardVector()*200;
	ABaseGrenade* Grenade = Cast<ABaseGrenade>(GetWorld()->SpawnActor(EquippedGrenadeClass, &SpawnLoc));
	Grenade->SetArmed(true);
	FVector Force = GetFirstPersonCameraComponent()->GetForwardVector() + FVector(0,0,0.1);
	Grenade->Mesh->AddImpulse(Force*20000);
}

void AHaloFloodFanGame01Character::SwitchWeapon()
{
	
	if (!EquippedWep || !HolsteredWeapon)
		return;
	EquippedWep->ReleaseTrigger();
	AGunBase* TempGun = EquippedWep;
	EquippedWep = HolsteredWeapon;
	HolsteredWeapon = TempGun;
	HolsteredWeapon->SetActorHiddenInGame(true);
	EquippedWep->SetActorHiddenInGame(false);
	EquippedWep->AttachToComponent(Mesh1P, FAttachmentTransformRules::SnapToTargetNotIncludingScale, "GripPoint");
	//if (PlayerHUD) PlayerHUD->UpdateHUDWeaponData(EquippedWep, HolsteredWeapon);
	WeaponsUpdated.Broadcast(EquippedWep, HolsteredWeapon);
}

void AHaloFloodFanGame01Character::Interact()
{
	FHitResult Hit;
	FVector TraceStart = FirstPersonCameraComponent->GetComponentLocation();
	FVector TraceEnd = FirstPersonCameraComponent->GetComponentLocation() + FirstPersonCameraComponent->GetForwardVector()*1000.0f;
	FCollisionQueryParams CollisionParameters;
	CollisionParameters.AddIgnoredActor(this);
	GetWorld()->LineTraceSingleByChannel(Hit, TraceStart, TraceEnd, ECollisionChannel::ECC_Visibility, CollisionParameters);

	if (Hit.bBlockingHit)
	{
		UE_LOG(LogTemp, Warning, TEXT("Trace Hit %s"), *Hit.GetActor()->GetActorLabel());
		if (UKismetSystemLibrary::DoesImplementInterface(Hit.GetActor(), UInteractableInterface::StaticClass()))
		{
			UE_LOG(LogTemp, Warning, TEXT("Interacted w/ %s"), *Hit.GetActor()->GetActorLabel());
			IInteractableInterface::Execute_OnInteract(Hit.GetActor(), this);
			//IntFace->Execute_OnInteract(Hit.GetActor(), this);
		}
	}
}

void AHaloFloodFanGame01Character::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);
	if (APlayerController* PC = Cast<APlayerController>(NewController))
	{
		PlayerController = PC;
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
		{
			Subsystem->AddMappingContext(DefaultMappingContext, 0);
		}

		if (IsLocallyControlled() && PlayerHUDClass) {
			check(PlayerController);
			PlayerHUD = CreateWidget<UHaloHUDWidget>(PlayerController, PlayerHUDClass);
			PlayerHUD->PlayerCharacter = this;
			PlayerHUD->AddToPlayerScreen();
		}
	}
	
}

void AHaloFloodFanGame01Character::UnPossessed()
{
	Super::UnPossessed();

	//if (PlayerHUD) PlayerHUD->RemoveFromParent();

	if (APlayerController* PC = Cast<APlayerController>(Controller))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PC->GetLocalPlayer()))
		{
			Subsystem->RemoveMappingContext(DefaultMappingContext);
		}
	}
}

void AHaloFloodFanGame01Character::PickupWeapon(AGunBase* Gun)
{
	Super::PickupWeapon(Gun);
	EquippedWep->AttachToComponent(Mesh1P, FAttachmentTransformRules::SnapToTargetNotIncludingScale, "GripPoint");
	WeaponsUpdated.Broadcast(EquippedWep, HolsteredWeapon);
}

void AHaloFloodFanGame01Character::DropWeapon()
{
	Super::DropWeapon();
	WeaponsUpdated.Broadcast(EquippedWep, HolsteredWeapon);
}

void AHaloFloodFanGame01Character::SetFragCount(int32 NewFragCount)
{
	FragCount = NewFragCount;
	if (PlayerHUD) PlayerHUD->SetFragCounter(FragCount);
}