// Copyright Epic Games, Inc. All Rights Reserved.

#include "HaloFloodFanGame01Character.h"
#include "GunBase.h"
#include "HaloFloodFanGame01Projectile.h"
#include "Animation/AnimInstance.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "HaloHUDWidget.h"
#include "InteractableInterface.h"
#include "Blueprint/UserWidget.h"
#include "Components/TextBlock.h"


//////////////////////////////////////////////////////////////////////////
// AHaloFloodFanGame01Character



AHaloFloodFanGame01Character::AHaloFloodFanGame01Character()
{
	// Character doesnt have a rifle at start
	bHasRifle = false;
	
	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(55.f, 96.0f);
		
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

	PlayerHUDClass = nullptr;
	PlayerHUD = nullptr;

	
	//PlayerHUD->PlasmaCounter->SetText(FText::AsNumber(PlasmaCount));
	//PlayerHUD->SpikeCounter->SetText(FText::AsNumber(SpikeCount));
	//PlayerHUD->IncenCounter->SetText(FText::AsNumber(IncenCount));
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
		PlayerHUD->AddToPlayerScreen();
		PlayerHUD->SetFragCounter(FragCount);
		PlayerHUD->SetPlasmaCounter(PlasmaCount);
		PlayerHUD->SetSpikeCounter(SpikeCount);
		PlayerHUD->SetIncenCounter(IncenCount);
	}

}

void AHaloFloodFanGame01Character::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	
	if (Shields < MaxShields && CanShieldsRecharge)
	{
		FMath::Clamp(Shields += ShieldRegenRate, 0, MaxShields);
	}

	FHitResult Hit;
	FVector TraceStart = FirstPersonCameraComponent->GetComponentLocation();
	FVector TraceEnd = FirstPersonCameraComponent->GetComponentLocation() + FirstPersonCameraComponent->GetForwardVector()*1000.0f;
	FCollisionQueryParams CollisionParameters;
	CollisionParameters.AddIgnoredActor(this);
	GetWorld()->LineTraceSingleByChannel(Hit, TraceStart, TraceEnd, ECollisionChannel::ECC_WorldDynamic, CollisionParameters);

	if (Hit.bBlockingHit && IsValid(Hit.GetActor()) && Cast<IInteractableInterface>(Hit.GetActor()))
		PlayerHUD->SetCanInteract(true);
	else 
		PlayerHUD->SetCanInteract(false);

	PlayerHUD->SetCompassDirection(GetFirstPersonCameraComponent()->GetComponentRotation().Yaw);
	if (EquippedWep) PlayerHUD->SetAmmoReserveCounter(EquippedWep->CurReserve);
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

		EnhancedInputComponent->BindAction(PrimaryAttackAction, ETriggerEvent::Triggered, this, &AHaloFloodFanGame01Character::PrimaryInput);

		EnhancedInputComponent->BindAction(SecondaryAttackAction, ETriggerEvent::Triggered, this, &AHaloFloodFanGame01Character::SecondaryInput);

		EnhancedInputComponent->BindAction(SwitchWeaponAction, ETriggerEvent::Triggered, this, &AHaloFloodFanGame01Character::SwitchWeapon);
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
	if (EquippedWep)
		EquippedWep->PrimaryAttack();
}

void AHaloFloodFanGame01Character::SetHasRifle(bool bNewHasRifle)
{
	bHasRifle = bNewHasRifle;
}

bool AHaloFloodFanGame01Character::GetHasRifle()
{
	return bHasRifle;
}

void AHaloFloodFanGame01Character::PickupWeapon(AGunBase* Gun)
{
	if (!EquippedWep)
	{
		EquippedWep = Gun;
	} else if (!HolsteredWeapon)
	{
		HolsteredWeapon = Gun;
		Gun->SetHidden(true);
	} else
	{
		DropWeapon();
		EquippedWep = Gun;
	}
	Gun->Mesh->SetSimulatePhysics(false);
	Gun->SetActorEnableCollision(false);
	Gun->AttachToComponent(Mesh1P, FAttachmentTransformRules::SnapToTargetNotIncludingScale, "GripPoint");
	Gun->Camera = Cast<USceneComponent>(GetFirstPersonCameraComponent());
}


void AHaloFloodFanGame01Character::DropWeapon()
{
	EquippedWep->DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
	EquippedWep->SetActorLocation(this->Mesh1P->GetSocketLocation("GripPoint"));
	EquippedWep->SetActorEnableCollision(true);
	EquippedWep->Mesh->SetSimulatePhysics(true);
	EquippedWep->Mesh->AddImpulse(FirstPersonCameraComponent->GetForwardVector()*5000);
	EquippedWep = nullptr;
}

void AHaloFloodFanGame01Character::PrimaryInput()
{
	if (EquippedWep)
		EquippedWep->PrimaryAttack();
}

void AHaloFloodFanGame01Character::SecondaryInput()
{
	if (EquippedWep)
		EquippedWep->SecondaryAttack();
}

void AHaloFloodFanGame01Character::ThrowGrenade()
{
}

void AHaloFloodFanGame01Character::UseEquipment()
{
}

void AHaloFloodFanGame01Character::SwitchWeapon()
{
	UE_LOG(LogTemp, Warning, TEXT("Switched weapon"));
	if (!EquippedWep || !HolsteredWeapon)
		return;
	AGunBase* TempGun = EquippedWep;
	EquippedWep = HolsteredWeapon;
	HolsteredWeapon = TempGun;

	HolsteredWeapon->SetActorHiddenInGame(true);
	EquippedWep->SetActorHiddenInGame(false);
	EquippedWep->AttachToComponent(Mesh1P, FAttachmentTransformRules::SnapToTargetNotIncludingScale, "GripPoint");
	PlayerHUD->ConstructAmmoGrid(EquippedWep);
}

void AHaloFloodFanGame01Character::Death()
{
}

void AHaloFloodFanGame01Character::StartShieldRegen()
{
	CanShieldsRecharge = true;
}

void AHaloFloodFanGame01Character::RegenShield()
{
}

void AHaloFloodFanGame01Character::BreakShield()
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

float AHaloFloodFanGame01Character::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent,
                                               AController* EventInstigator, AActor* DamageCauser)
{
	float DamageLeft = DamageAmount;
	if (Shields > 0)
	{
		DamageLeft = DamageAmount - Shields;
		Shields -= DamageAmount;
		PlayerHUD->SetShields(Shields, MaxShields);
		UE_LOG(LogTemp, Warning, TEXT("Shields: %f"), Shields);
		if (Shields <= 0)
		{
			UE_LOG(LogTemp, Warning, TEXT("Shields broken!"));
		}
	} 
	if (Shields <= 0 && Health > 0)
	{
		Health -= DamageLeft;
		PlayerHUD->SetHealth(Health, MaxHealth);
		UE_LOG(LogTemp, Warning, TEXT("Health: %f"), Health);
		if (Health <= 0)
		{
			UE_LOG(LogTemp, Warning, TEXT("Health depleted!"));
			this->Death();
		}
	}
	return DamageAmount;
}

