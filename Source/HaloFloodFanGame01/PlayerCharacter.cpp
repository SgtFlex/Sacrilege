// Copyright Epic Games, Inc. All Rights Reserved.

#include "PlayerCharacter.h"

#include "HealthComponent.h"
#include "GrenadeBase.h"
#include "GunBase.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InteractableInterface.h"
#include "Blueprint/UserWidget.h"
#include "Components/SphereComponent.h"
#include "Components/TimelineComponent.h"
#include "Engine/DamageEvents.h"
#include "GameFramework/SpectatorPawn.h"
#include "Kismet/KismetSystemLibrary.h"



APlayerCharacter::APlayerCharacter()
{
	// Set size for collision capsule
	//GetCapsuleComponent()->InitCapsuleSize(55.f, 96.0f);
		
	

	// Create a mesh component that will be used when being viewed from a '1st person' view (when controlling this pawn)
	Mesh1P = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("CharacterMesh1P"));
	Mesh1P->SetOnlyOwnerSee(true);
	Mesh1P->SetupAttachment(GetCapsuleComponent());
	Mesh1P->bCastDynamicShadow = false;
	Mesh1P->CastShadow = false;

	// Create a CameraComponent	
	FirstPersonCameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("FirstPersonCamera"));
	FirstPersonCameraComponent->SetupAttachment(GetMesh1P(), "HeadSocket");
	//FirstPersonCameraComponent->SetRelativeLocation(FVector(-10.f, 0.f, 60.f)); // Position the camera
	FirstPersonCameraComponent->bUsePawnControlRotation = true;

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
	//if (GetController())
		//GetMesh1P()->SetWorldRotation(GetControlRotation());

	SetCurrentInteractable();
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

		EnhancedInputComponent->BindAction(ReloadAction, ETriggerEvent::Triggered, this, &APlayerCharacter::ReloadWeapon);
		
		EnhancedInputComponent->BindAction(MeleeAction, ETriggerEvent::Triggered, this, &APlayerCharacter::Melee);

		EnhancedInputComponent->BindAction(SwitchGrenadeAction, ETriggerEvent::Triggered, this, &APlayerCharacter::SwitchGrenadeType);
		
		EnhancedInputComponent->BindAction(ThrowGrenadeAction, ETriggerEvent::Triggered, this, &APlayerCharacter::ThrowEquippedGrenade);

		EnhancedInputComponent->BindAction(UseEquipmentAction, ETriggerEvent::Triggered, this, &APlayerCharacter::UseEquipment);
	}
}

void APlayerCharacter::Move(const FInputActionValue& Value)
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

void APlayerCharacter::Look(const FInputActionValue& Value)
{
	// input is a Vector2D
	
	const FVector2D LookAxisVector = Value.Get<FVector2D>();

	// Disabled for now due to incorrect pitch values in standalone
	// Server_Look(LookAxisVector.Y);
	if (Controller != nullptr)
	{
		// add yaw and pitch input to controller
		AddControllerYawInput(LookAxisVector.X);
		AddControllerPitchInput(LookAxisVector.Y);
		Mesh1P->AddLocalRotation(FRotator(0, 0, -LookAxisVector.Y));
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

//@TODO Simplify by removing InteractionSphere. Interaction radius and casting should be done by interactables, not the player character.
//We should simply find the closest Interactable actor within an array of interactables
void APlayerCharacter::SetCurrentInteractable()
{
	TArray<AActor*> ActorsToIgnore;
	ActorsToIgnore.Add(this);
	UKismetSystemLibrary::SphereTraceSingle(GetWorld(), FirstPersonCameraComponent->GetComponentLocation(), FirstPersonCameraComponent->GetComponentLocation() + FirstPersonCameraComponent->GetForwardVector()*10000.0f, 20, UEngineTypes::ConvertToTraceType(ECollisionChannel::ECC_Visibility), false, ActorsToIgnore, EDrawDebugTrace::None, PlayerAim, true, FLinearColor::Red, FLinearColor::Green, 5);
	AActor* FoundActor = nullptr;
	
	if (GetPlayerAim().GetActor() && PlayerAim.Distance < 250 && GetPlayerAim().GetActor()->Implements<UInteractableInterface>())
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

void APlayerCharacter::Melee_Implementation()
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

	if (AGrenadeBase* Grenade = Cast<AGrenadeBase>(GetWorld()->SpawnActorDeferred<AGrenadeBase>(GrenadeInventory[CurGrenadeTypeI].GrenadeClass, SpawnTransform, this, this, ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn)))
	{
		Grenade->SetInstigator(this);
		Grenade->SetArmed(true);
		Grenade->FinishSpawning(SpawnTransform);
		FVector Direction = GetFirstPersonCameraComponent()->GetForwardVector() + FVector(0,0,0.15);
		Direction.Normalize();
		Grenade->Mesh->AddImpulse(Direction*2000.0f, NAME_None, true);
		Grenade->Mesh->AddAngularImpulseInDegrees(Grenade->GetActorRightVector().GetSafeNormal()*1000 , NAME_None, true);
		
		if (GrenadeInventory[CurGrenadeTypeI].GrenadeAmount <= 0)
		{
			GrenadeInventory.RemoveAt(CurGrenadeTypeI);
			SwitchGrenadeType(CurGrenadeTypeI);
		}
		OnGrenadeInventoryUpdated.Broadcast(GrenadeInventory);
	}
}

void APlayerCharacter::Interact()
{
	if (InteractableActor && InteractableActor->Implements<UInteractableInterface>())
	{
		IInteractableInterface::Execute_OnInteract(InteractableActor, this);
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
}

void APlayerCharacter::EquipWeapon(AGunBase* Gun)
{
	Super::EquipWeapon(Gun);
	
	if (IsLocallyControlled())
	{
		GetMesh1P()->GetAnimInstance()->Montage_Play(DrawAnimation1P);
		Gun->AttachToComponent(Mesh1P, FAttachmentTransformRules::SnapToTargetNotIncludingScale, "GripPoint");
		//Gun->Mesh->PlayAnimation(Gun->DrawAnimation1P, false);
	}
}

void APlayerCharacter::HolsterWeapon(AGunBase* Gun)
{
	Super::HolsterWeapon(Gun);

	GetMesh1P()->GetAnimInstance()->Montage_Play(HolsterAnimation1P);
	//Gun->Mesh->PlayAnimation(Gun->HolsterAnimation1P, false);
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