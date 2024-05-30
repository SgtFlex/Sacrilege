// Copyright Epic Games, Inc. All Rights Reserved.

#include "PlayerCharacter.h"

#include "HealthComponent.h"
#include "GrenadeBase.h"
#include "GunBase.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "FirefightGamemode.h"
#include "InteractableInterface.h"
#include "PlayerControllerBase.h"
#include "Blueprint/UserWidget.h"
#include "Components/SphereComponent.h"
#include "Components/TimelineComponent.h"
#include "Engine/DamageEvents.h"
#include "GameFramework/InputDeviceSubsystem.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetSystemLibrary.h"



APlayerCharacter::APlayerCharacter()
{
	// Create a mesh component that will be used when being viewed from a '1st person' view (when controlling this pawn)
	Mesh1P = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("CharacterMesh1P"));
	Mesh1P->SetOnlyOwnerSee(true);
	Mesh1P->SetupAttachment(GetCapsuleComponent());
	Mesh1P->bCastDynamicShadow = false;
	Mesh1P->CastShadow = false;

	// Create a CameraComponent	
	FirstPersonCameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("FirstPersonCamera"));
	FirstPersonCameraComponent->SetupAttachment(GetMesh1P(), "HeadSocket");
	FirstPersonCameraComponent->bUsePawnControlRotation = true;

	InteractionSphere = CreateDefaultSubobject<USphereComponent>(TEXT("Interaction Sphere"));
	InteractionSphere->SetupAttachment(GetRootComponent());
	InteractionSphere->SetSphereRadius(500);
	InteractionSphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	Mesh1P->SetRelativeLocation(FVector(-30.f, 0.f, -150.f));
}

void APlayerCharacter::BeginPlay()
{
	// Call the base class  
	Super::BeginPlay();
	
	InputDeviceSubsystem = GetGameInstance()->GetEngine()->GetEngineSubsystem<UInputDeviceSubsystem>();
}

void APlayerCharacter::Tick(const float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	MeleeTimeline.TickTimeline(DeltaSeconds);
	if (GetController())
		GetMesh1P()->SetWorldRotation(FRotator(0, GetViewRotation().Yaw + 90, GetViewRotation().Pitch));

	SetCurrentInteractable();
}

void APlayerCharacter::GetPlayerAim(FHitResult& HitResult) const
{
	HitResult = PlayerAim;
}

//////////////////////////////////////////////////////////////////////////// Input

void APlayerCharacter::SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent)
{
	// Set up action bindings
	Super::SetupPlayerInputComponent(PlayerInputComponent);
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

float APlayerCharacter::AimAssist() const
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

void APlayerCharacter::Look(const FInputActionValue& Value)
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

void APlayerCharacter::Server_Look_Implementation(const float Pitch)
{
	Multi_Look(Pitch);
}

FVector StartMeleeLoc;
FVector EndMeleeLoc;
FHitResult MeleeHit;

void APlayerCharacter::Multi_Look_Implementation(const float Pitch)
{
	FRotator Rotation = GetFirstPersonCameraComponent()->GetComponentRotation();
	Rotation.Pitch = Pitch;
	GetFirstPersonCameraComponent()->SetWorldRotation(Rotation);
}

//@TODO Simplify by removing InteractionSphere. Interaction radius and casting should be done by interactables, not the player character.
//We should simply find the closest Interactable actor within an array of interact-ables
void APlayerCharacter::SetCurrentInteractable()
{
	TArray<AActor*> ActorsToIgnore;
	ActorsToIgnore.Add(this);
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

void APlayerCharacter::Melee_Implementation()
{
	if (GetWorld()->GetTimerManager().TimerExists(MeleeTimer)) return;
	GetWorld()->GetTimerManager().SetTimer(MeleeTimer, 1, false);

	const FVector TraceStart = GetFirstPersonCameraComponent()->GetComponentLocation();
	const FVector TraceEnd = GetFirstPersonCameraComponent()->GetComponentLocation() + GetFirstPersonCameraComponent()->GetForwardVector()*500;
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

void APlayerCharacter::MeleeDamageCode()
{
	FPointDamageEvent PointDamageEvent;
	PointDamageEvent.Damage = MeleeDamage;
	PointDamageEvent.HitInfo = MeleeHit;
	FVector Dir = MeleeHit.Location - MeleeHit.TraceStart;
	Dir.Normalize();
	IDamageableInterface::Execute_CustomTakePointDamage(MeleeHit.GetActor(), PointDamageEvent, MeleeForce, GetInstigatorController(), this);
	//HitActor->CustomTakePointDamage(PointDamageEvent, MeleeForce);
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
		
	}
	
	Super::OnHealthDepleted_Implementation(Damage, Force, HitLocation, HitBoneName, EventInstigator, DamageCauser);
	Cast<AFirefightGameMode>(GetWorld()->GetAuthGameMode())->OnPlayerCharDied.Broadcast(this, Cast<APlayerControllerBase>(PlayerController));
}

void APlayerCharacter::ThrowEquippedGrenade_Implementation()
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
			SwitchGrenadeType(CurGrenadeTypeI);
		}
		OnGrenadeInventoryUpdated.Broadcast(GrenadeInventory);
	}
}

void APlayerCharacter::Interact()
{
	Server_Interact();
}

void APlayerCharacter::Server_Interact_Implementation()
{
	Multi_Interact();
}

void APlayerCharacter::Multi_Interact_Implementation()
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
	if (HolsteredWeapon)
		HolsteredWeapon->SetActorHiddenInGame(true);
	if (Gun->DrawAnimation1P)
		GetMesh1P()->GetAnimInstance()->Montage_Play(Gun->DrawAnimation1P, Gun->DrawAnimation1P->GetPlayLength() / Gun->DrawSpeed);
	Gun->AttachToComponent(Mesh1P, FAttachmentTransformRules::SnapToTargetNotIncludingScale, "GripPoint");
	//Gun->Mesh->PlayAnimation(Gun->DrawAnimation1P, false);
	WeaponsUpdated.Broadcast(EquippedWeapon, HolsteredWeapon);
}

void APlayerCharacter::HolsterWeapon(AGunBase* Gun)
{
	Gun->ReleaseTrigger();
	GetWorldTimerManager().ClearTimer(Gun->ReloadTimer);
	Gun->ScopeOut();
	Gun->bReloading = false;

	if (Gun->HolsterAnimation1P)
		GetMesh1P()->GetAnimInstance()->Montage_Play(EquippedWeapon->HolsterAnimation1P, EquippedWeapon->HolsterAnimation1P->GetPlayLength() / EquippedWeapon->HolsterSpeed);
}

void APlayerCharacter::SwitchWeapon()
{
	Server_SwitchWeapon();
}

void APlayerCharacter::Server_SwitchWeapon_Implementation()
{
	Multi_SwitchWeapon();
}

void APlayerCharacter::Multi_SwitchWeapon_Implementation()
{
	if (!(EquippedWeapon && HolsteredWeapon))
		return;
	HolsterWeapon(EquippedWeapon);
	
	AGunBase* TempGun = EquippedWeapon;
	EquippedWeapon = HolsteredWeapon;
	HolsteredWeapon = TempGun;
	
	//EquipWeapon(EquippedWeapon);

	GetWorld()->GetTimerManager().SetTimer(HolsterHandle, FTimerDelegate::CreateUObject(this, &APlayerCharacter::EquipWeapon, EquippedWeapon), HolsteredWeapon->HolsterSpeed, false);
	
}

void APlayerCharacter::ScopeWeapon()
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

void APlayerCharacter::NotifyRestarted()
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
		}
		
	}
}

void APlayerCharacter::UnPossessed()
{
	if (const APlayerController* PC = Cast<APlayerController>(Controller))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PC->GetLocalPlayer()))
		{
			Subsystem->RemoveMappingContext(DefaultMappingContext);
			UE_LOG(LogTemp, Warning, TEXT("Player unpossessed"));
			
		}
	}
	Super::UnPossessed();
}
