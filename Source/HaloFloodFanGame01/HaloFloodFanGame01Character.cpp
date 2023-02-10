// Copyright Epic Games, Inc. All Rights Reserved.

#include "HaloFloodFanGame01Character.h"
#include "HaloFloodFanGame01Projectile.h"
#include "Animation/AnimInstance.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "HaloHUDWidget.h"
#include "Blueprint/UserWidget.h"


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
		PlayerHUD->SetShields(25, 100);
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

void AHaloFloodFanGame01Character::SetHasRifle(bool bNewHasRifle)
{
	bHasRifle = bNewHasRifle;
}

bool AHaloFloodFanGame01Character::GetHasRifle()
{
	return bHasRifle;
}

void AHaloFloodFanGame01Character::PickupWeapon(AGunBase* gun)
{

	gun->AttachToComponent(Mesh1P, FAttachmentTransformRules::SnapToTargetNotIncludingScale);
}

TSubclassOf<AGunBase>* AHaloFloodFanGame01Character::GetCurrentWeapon()
{
	if (curSlot == 1) {
		return PrimWep;
	}
	else if (curSlot == 2) {
		return SecWep;
	}
	else if (curSlot == 3) {
		return ObjWep;
	}
}

void AHaloFloodFanGame01Character::DropWeapon(int slot)
{
}
