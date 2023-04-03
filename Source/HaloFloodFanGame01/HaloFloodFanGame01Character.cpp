// Copyright Epic Games, Inc. All Rights Reserved.

#include "HaloFloodFanGame01Character.h"
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
#include "Components/UniformGridPanel.h"
#include "Engine/DamageEvents.h"
#include "GameFramework/GameMode.h"
#include "GameFramework/SpectatorPawn.h"


//////////////////////////////////////////////////////////////////////////
// AHaloFloodFanGame01Character



AHaloFloodFanGame01Character::AHaloFloodFanGame01Character()
{
	// Character doesnt have a rifle at start
	bHasRifle = false;
	
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

	//Add Input Mapping Context
	if (APlayerController* PlayerController = Cast<APlayerController>(Controller))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
		{
			Subsystem->AddMappingContext(DefaultMappingContext, 0);
		}
	}

	if (IsLocallyControlled() && PlayerHUDClass) {
		APlayerController* PC = GetController<APlayerController>();
		check(PC);
		PlayerHUD = CreateWidget<UHaloHUDWidget>(PC, PlayerHUDClass);
		PlayerHUD->Character = this;
		PlayerHUD->AddToPlayerScreen();
	}
}

void AHaloFloodFanGame01Character::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	MeleeTimeline.TickTimeline(DeltaSeconds);
	FHitResult Hit;
	FVector TraceStart = FirstPersonCameraComponent->GetComponentLocation();
	FVector TraceEnd = FirstPersonCameraComponent->GetComponentLocation() + FirstPersonCameraComponent->GetForwardVector()*1000.0f;
	FCollisionQueryParams CollisionParameters;
	CollisionParameters.AddIgnoredActor(this);
	GetWorld()->LineTraceSingleByChannel(Hit, TraceStart, TraceEnd, ECollisionChannel::ECC_WorldDynamic, CollisionParameters);

	

	if (PlayerHUD)
	{
		PlayerHUD->SetCrosshairType(1);
		if (Hit.bBlockingHit && IsValid(Hit.GetActor()) && Cast<IInteractableInterface>(Hit.GetActor()))
		{
			PlayerHUD->SetCanInteract(true);
			PlayerHUD->SetCrosshairType(2);
		}
		else 
			PlayerHUD->SetCanInteract(false);
		
		PlayerHUD->SetCompassDirection(FirstPersonCameraComponent->GetComponentRotation().Yaw);
		PlayerHUD->SetShields(HealthComponent->GetShields(), HealthComponent->GetMaxShields());
		PlayerHUD->SetHealth(HealthComponent->GetHealth(), HealthComponent->GetMaxHealth());
	}
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

		EnhancedInputComponent->BindAction(PrimaryAttackAction, ETriggerEvent::Started, this, &ABaseCharacter::PrimaryAttack_Pull);

		EnhancedInputComponent->BindAction(PrimaryAttackAction, ETriggerEvent::Completed, this, &ABaseCharacter::PrimaryAttack_Release);

		// EnhancedInputComponent->BindAction(SecondaryAttackAction, ETriggerEvent::Triggered, this, &AHaloFloodFanGame01Character::SecondaryInput);

		EnhancedInputComponent->BindAction(SwitchWeaponAction, ETriggerEvent::Triggered, this, &AHaloFloodFanGame01Character::SwitchWeapon);

		EnhancedInputComponent->BindAction(ReloadAction, ETriggerEvent::Triggered, this, &AHaloFloodFanGame01Character::ReloadInput);
		
		EnhancedInputComponent->BindAction(MeleeAction, ETriggerEvent::Triggered, this, &ABaseCharacter::Melee);
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

void AHaloFloodFanGame01Character::Attack()
{
	// if (EquippedWep)
	// 	EquippedWep->PrimaryAttack();
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
	GetWorld()->LineTraceSingleByChannel(MeleeHit, TraceStart, TraceEnd, ECollisionChannel::ECC_WorldDynamic, CollisionParameters);
	DrawDebugLine(GetWorld(), TraceStart, TraceEnd, FColor(255, 0, 0), false, 3);
	
	if (MeleeHit.GetActor())
	{
		
		if (IDamageableInterface* HitActor = Cast<IDamageableInterface>(MeleeHit.GetActor()))
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
	PlayerHUD->RemoveFromParent();
	AController* PC = GetController();
	PC->UnPossess();
	FVector Loc = GetFirstPersonCameraComponent()->GetComponentLocation();
	FRotator Rot = GetFirstPersonCameraComponent()->GetComponentRotation();
	ASpectatorPawn* SpectatorPawn = Cast<ASpectatorPawn>(GetWorld()->SpawnActor(ASpectatorPawn::StaticClass(), &Loc, &Rot));
	PC->Possess(SpectatorPawn);
	FTimerHandle RestartTimer;
	// GetWorldTimerManager().SetTimer(RestartTimer, this, &AHaloFloodFanGame01Character::Attack, 5, false);
	// GetWorldTimerManager().SetTimer(RestartTimer, UGameplayStatics::GetGameMode(GetWorld())->RestartPlayer(PC), 5, false);
	Super::HealthDepleted(Damage, Force, HitLocation, HitBoneName);
}

void AHaloFloodFanGame01Character::SetHasRifle(bool bNewHasRifle)
{
	bHasRifle = bNewHasRifle;
}

bool AHaloFloodFanGame01Character::GetHasRifle()
{
	return bHasRifle;
}

void AHaloFloodFanGame01Character::ReloadInput()
{
	UE_LOG(LogTemp, Warning, TEXT("Reload"));
	if (EquippedWep) EquippedWep->Reload();
}

void AHaloFloodFanGame01Character::ThrowGrenade()
{
}

void AHaloFloodFanGame01Character::UseEquipment()
{
}

void AHaloFloodFanGame01Character::SwitchWeapon()
{
	
	if (!EquippedWep || !HolsteredWeapon)
		return;
	AGunBase* TempGun = EquippedWep;
	EquippedWep = HolsteredWeapon;
	HolsteredWeapon = TempGun;
	HolsteredWeapon->SetActorHiddenInGame(true);
	EquippedWep->SetActorHiddenInGame(false);
	EquippedWep->AttachToComponent(Mesh1P, FAttachmentTransformRules::SnapToTargetNotIncludingScale, "GripPoint");
	PlayerHUD->SetAmmoGridWeapon(EquippedWep);
	
}

void AHaloFloodFanGame01Character::Death()
{
}

void AHaloFloodFanGame01Character::Interact()
{
	FHitResult Hit;
	FVector TraceStart = FirstPersonCameraComponent->GetComponentLocation();
	FVector TraceEnd = FirstPersonCameraComponent->GetComponentLocation() + FirstPersonCameraComponent->GetForwardVector()*1000.0f;
	FCollisionQueryParams CollisionParameters;
	CollisionParameters.AddIgnoredActor(this);
	GetWorld()->LineTraceSingleByChannel(Hit, TraceStart, TraceEnd, ECollisionChannel::ECC_WorldDynamic, CollisionParameters);

	if (Hit.bBlockingHit)
	{
		if (IInteractableInterface* IntFace = Cast<IInteractableInterface>(Hit.GetActor()))
		{
			UE_LOG(LogTemp, Warning, TEXT("Implements Interactable Interface"));
			IntFace->Execute_OnInteract(Hit.GetActor(), this);
		}
	}
}

void AHaloFloodFanGame01Character::PickupWeapon(AGunBase* Gun)
{
	Super::PickupWeapon(Gun);
	PlayerHUD->SetAmmoGridWeapon(Gun);
}

void AHaloFloodFanGame01Character::DropWeapon()
{
	Super::DropWeapon();
	PlayerHUD->SetAmmoGridWeapon();
}