// Copyright Epic Games, Inc. All Rights Reserved.

#include "PlayerCharacter.h"

#include "GrenadeBase.h"
#include "GunBase.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "FrameTypes.h"
#include "HealthComponent.h"
#include "InteractableInterface.h"
#include "Blueprint/UserWidget.h"
#include "Components/SphereComponent.h"
#include "Components/TimelineComponent.h"
#include "Engine/DamageEvents.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "GameFramework/SpectatorPawn.h"
#include "Kismet/KismetMathLibrary.h"
#include "Net/UnrealNetwork.h"


//////////////////////////////////////////////////////////////////////////
// AHaloFloodFanGame01Character



APlayerCharacter::APlayerCharacter()
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

	InteractionSphere = CreateDefaultSubobject<USphereComponent>(TEXT("Interaction Sphere"));
	InteractionSphere->SetupAttachment(GetRootComponent());
	InteractionSphere->SetSphereRadius(500);
	InteractionSphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	//Mesh1P->SetRelativeRotation(FRotator(0.9f, -19.19f, 5.2f));
	Mesh1P->SetRelativeLocation(FVector(-30.f, 0.f, -150.f));
}

void APlayerCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{


	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	//DOREPLIFETIME(AHaloFloodFanGame01Character, EnhancedInputComponent);
}

void APlayerCharacter::OnInteractionSphereBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
	bool bFromSweep, const FHitResult& SweepResult)
{
	// if (Cast<IInteractableInterface>(OtherActor))
	// {
	// 	InteractableActor = OtherActor;
	// }
}

void APlayerCharacter::OnInteractionSphereEndOverlap(UPrimitiveComponent* OverlappedComponent,
	AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	//InteractableActor = nullptr;
}

void APlayerCharacter::BeginPlay()
{
	// Call the base class  
	Super::BeginPlay();

	InteractionSphere->OnComponentBeginOverlap.AddDynamic(this, &APlayerCharacter::OnInteractionSphereBeginOverlap);
	InteractionSphere->OnComponentEndOverlap.AddDynamic(this, &APlayerCharacter::OnInteractionSphereEndOverlap);
	
	if (HolsteredWeaponClass)
		PickupWeapon(Cast<AGunBase>(GetWorld()->SpawnActor(HolsteredWeaponClass)));

	//Add Input Mapping Context
	if (APlayerController* PC = Cast<APlayerController>(GetController()))
	{
		PlayerController = PC;
		//check(IsLocallyControlled());
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
		{
			Subsystem->AddMappingContext(DefaultMappingContext, 0);
		}

		if (IsLocallyControlled() && PlayerHUDClass) {
			PlayerHUD = CreateWidget<UUserWidget>(PlayerController, PlayerHUDClass);
			PlayerHUD->AddToPlayerScreen();
		}
	}
}

void APlayerCharacter::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	MeleeTimeline.TickTimeline(DeltaSeconds);

	TArray<AActor*> ActorsToIgnore;
	ActorsToIgnore.Add(this);
	UKismetSystemLibrary::SphereTraceSingle(GetWorld(), FirstPersonCameraComponent->GetComponentLocation(), FirstPersonCameraComponent->GetComponentLocation() + FirstPersonCameraComponent->GetForwardVector()*10000.0f, 20, UEngineTypes::ConvertToTraceType(ECollisionChannel::ECC_Visibility), false, ActorsToIgnore, EDrawDebugTrace::None, PlayerAim, true, FLinearColor::Red, FLinearColor::Green, 5);
	
	AActor* FoundActor = nullptr;

	if (Cast<IInteractableInterface>(GetPlayerAim().GetActor()) && GetPlayerAim().Distance < 250)
	{
		FoundActor = GetPlayerAim().GetActor();
	} else
	{
		TArray<AActor*> Actors;
		InteractionSphere->GetOverlappingActors(Actors);
		
		if (!Actors.IsEmpty())
		{
			float ClosestDist = 0;
			for (auto Actor : Actors)
			{
				if (Cast<IInteractableInterface>(Actor) && (ClosestDist == 0 || GetDistanceTo(Actor) < ClosestDist))
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
	
	// FRotator Rot = GetFirstPersonCameraComponent()->GetComponentRotation();
	// Rot.Pitch = UKismetMathLibrary::NormalizeAxis(RemoteViewPitch);
	// FirstPersonCameraComponent->SetWorldRotation(Rot);
}

FHitResult APlayerCharacter::GetPlayerAim()
{
	return PlayerAim;
}


//////////////////////////////////////////////////////////////////////////// Input

void APlayerCharacter::SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent)
{
	// Set up action bindings
	Super::SetupPlayerInputComponent(PlayerInputComponent);
	EnhancedInputComponent = CastChecked<UEnhancedInputComponent>(PlayerInputComponent);
	if (EnhancedInputComponent)
	{
		//Jumping
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Triggered, this, &ACharacter::Jump);
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Completed, this, &ACharacter::StopJumping);

		//Moving
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &APlayerCharacter::Move);

		//Looking
		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &APlayerCharacter::Look);

		EnhancedInputComponent->BindAction(InteractAction, ETriggerEvent::Triggered, this, &APlayerCharacter::Interact);

		EnhancedInputComponent->BindAction(PrimaryAttackAction, ETriggerEvent::Started, this, &APlayerCharacter::PrimaryAttack_Pull);

		EnhancedInputComponent->BindAction(PrimaryAttackAction, ETriggerEvent::Completed, this, &APlayerCharacter::PrimaryAttack_Release);
		
		EnhancedInputComponent->BindAction(SwitchWeaponAction, ETriggerEvent::Triggered, this, &APlayerCharacter::SwitchWeapon);

		EnhancedInputComponent->BindAction(ReloadAction, ETriggerEvent::Triggered, this, &APlayerCharacter::ReloadInput);
		
		EnhancedInputComponent->BindAction(MeleeAction, ETriggerEvent::Triggered, this, &APlayerCharacter::Melee);

		EnhancedInputComponent->BindAction(SwitchGrenadeAction, ETriggerEvent::Triggered, this, &APlayerCharacter::SwitchGrenadeType);
		
		EnhancedInputComponent->BindAction(ThrowGrenadeAction, ETriggerEvent::Triggered, this, &APlayerCharacter::ThrowEquippedGrenade);

		EnhancedInputComponent->BindAction(UseEquipmentAction, ETriggerEvent::Triggered, this, &APlayerCharacter::UseEquipment);
	}
}

void APlayerCharacter::Move(const FInputActionValue& Value)
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

void APlayerCharacter::Look(const FInputActionValue& Value)
{
	// input is a Vector2D
	
	FVector2D LookAxisVector = Value.Get<FVector2D>();
	Server_Look(LookAxisVector.Y);
	if (Controller != nullptr)
	{
		// add yaw and pitch input to controller
		AddControllerYawInput(LookAxisVector.X);
		AddControllerPitchInput(LookAxisVector.Y);
	}
}

void APlayerCharacter::Server_Look_Implementation(float Pitch)
{
	Multi_Look(Pitch);
}

FVector StartMeleeLoc;
FVector EndMeleeLoc;
FHitResult MeleeHit;

void APlayerCharacter::Multi_Look_Implementation(float Pitch)
{
	FRotator Rotation = GetFirstPersonCameraComponent()->GetComponentRotation();
	Rotation.Pitch = Pitch;
	GetFirstPersonCameraComponent()->SetWorldRotation(Rotation);
}

void APlayerCharacter::Melee_Implementation()
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

void APlayerCharacter::MeleeDamageCode()
{
	IDamageableInterface* HitActor = Cast<IDamageableInterface>(MeleeHit.GetActor());
	FPointDamageEvent PointDamageEvent;
	PointDamageEvent.Damage = MeleeDamage;
	PointDamageEvent.HitInfo = MeleeHit;
	FVector Dir = MeleeHit.Location - MeleeHit.TraceStart;
	Dir.Normalize();
	HitActor->CustomTakePointDamage(PointDamageEvent, MeleeForce);
}

void APlayerCharacter::MeleeUpdate(float Alpha)
{
	SetActorLocation(FMath::Lerp(StartMeleeLoc, EndMeleeLoc, Alpha));
}

void APlayerCharacter::OnHealthDepleted_Implementation(float Damage, FVector Force, FVector HitLocation, FName HitBoneName, AController* EventInstigator, AActor* DamageCauser)
{
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
	Super::OnHealthDepleted_Implementation(Damage, Force, HitLocation, HitBoneName, EventInstigator, DamageCauser);
}

void APlayerCharacter::ThrowEquippedGrenade_Implementation()
{
	if (GrenadeInventory.Num() <= 0) return;

	GrenadeInventory[CurGrenadeTypeI].GrenadeAmount -= 1;
	const FTransform SpawnTransform = FTransform(GetFirstPersonCameraComponent()->GetForwardVector().Rotation(), GetFirstPersonCameraComponent()->GetComponentLocation() + GetFirstPersonCameraComponent()->GetForwardVector()*300);
	FActorSpawnParameters ActorSpawnParameters;
	ActorSpawnParameters.Instigator = this;
	ActorSpawnParameters.Owner = this;

	if (AGrenadeBase* Grenade = Cast<AGrenadeBase>(GetWorld()->SpawnActorDeferred<AGrenadeBase>(GrenadeInventory[CurGrenadeTypeI].GrenadeClass, SpawnTransform, this, this, ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButDontSpawnIfColliding)))
	{
		Grenade->SetInstigator(this);
		Grenade->SetArmed(true);
		Grenade->FinishSpawning(SpawnTransform);
		FVector Direction = GetFirstPersonCameraComponent()->GetForwardVector() + FVector(0,0,0.15);
		Direction.Normalize();
		Grenade->Mesh->AddImpulse(Direction*2000.0f, NAME_None, true);
		Grenade->Mesh->AddAngularImpulseInDegrees(Grenade->GetActorRightVector().GetSafeNormal()*1000 , NAME_None, true);
		//Grenade->ProjectileMovementComponent->Velocity = (Direction*2000.0f);
		
		if (GrenadeInventory[CurGrenadeTypeI].GrenadeAmount <= 0)
		{
			GrenadeInventory.RemoveAt(CurGrenadeTypeI);
			SwitchGrenadeType(CurGrenadeTypeI);
		}
		OnGrenadeInvetoryUpdated.Broadcast(GrenadeInventory);
	}
}

void APlayerCharacter::SwitchWeapon()
{
	
	if (!EquippedWeapon || !HolsteredWeapon)
		return;
	EquippedWeapon->ReleaseTrigger();
	GetWorldTimerManager().ClearTimer(EquippedWeapon->ReloadTimer);
	EquippedWeapon->bReloading = false;
	AGunBase* TempGun = EquippedWeapon;
	EquippedWeapon = HolsteredWeapon;
	HolsteredWeapon = TempGun;
	HolsteredWeapon->SetActorHiddenInGame(true);
	EquippedWeapon->SetActorHiddenInGame(false);
	EquippedWeapon->AttachToComponent(Mesh1P, FAttachmentTransformRules::SnapToTargetNotIncludingScale, "GripPoint");
	WeaponsUpdated.Broadcast(EquippedWeapon, HolsteredWeapon);
}

void APlayerCharacter::Interact()
{
	if (InteractableActor)
	{
		if (UKismetSystemLibrary::DoesImplementInterface(InteractableActor, UInteractableInterface::StaticClass()))
		{
			IInteractableInterface::Execute_OnInteract(InteractableActor, this);
		}
	}
	
}

void APlayerCharacter::SwitchGrenadeType()
{
	SwitchGrenadeType(CurGrenadeTypeI+1);
}

void APlayerCharacter::SwitchGrenadeType(int Index = 0)
{
	if (GrenadeInventory.Num() <= 0) return;
	
	CurGrenadeTypeI = Index;
	CurGrenadeTypeI = CurGrenadeTypeI % (GrenadeInventory.Num());
	OnGrenadeTypeSwitched.Broadcast(GrenadeInventory[CurGrenadeTypeI].GrenadeClass);
	UE_LOG(LogTemp, Warning, TEXT("Switched grenade to: %s"), *GrenadeInventory[CurGrenadeTypeI].GrenadeClass->GetDefaultObjectName().ToString());
}

void APlayerCharacter::ControllerChanged(AController* OldController, AController* NewController)
{
	
}

void APlayerCharacter::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);
	
}

void APlayerCharacter::UnPossessed()
{
	Super::UnPossessed();
	if (APlayerController* PC = Cast<APlayerController>(Controller))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PC->GetLocalPlayer()))
		{
			Subsystem->RemoveMappingContext(DefaultMappingContext);
		}
	}
}

void APlayerCharacter::PickupWeapon(AGunBase* Gun)
{
	Super::PickupWeapon(Gun);
	if (EquippedWeapon)
	{
		if (IsLocallyControlled())
		{
			EquippedWeapon->AttachToComponent(Mesh1P, FAttachmentTransformRules::SnapToTargetNotIncludingScale, "GripPoint");
		}
		WeaponsUpdated.Broadcast(EquippedWeapon, HolsteredWeapon);
	} else
	{
		UE_LOG(LogTemp, Warning, TEXT("Invalid pickup for player"));
	}
}

void APlayerCharacter::DropWeapon()
{
	Super::DropWeapon();
	WeaponsUpdated.Broadcast(EquippedWeapon, HolsteredWeapon);
}

void APlayerCharacter::SetFragCount(int32 NewFragCount)
{
	FragCount = NewFragCount;
}