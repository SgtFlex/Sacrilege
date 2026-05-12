// Fill out your copyright notice in the Description page of Project Settings.


#include "Core/WeaponBase.h"

#include "Subsystems/WorldCleanupManager.h"
#include "Camera/CameraComponent.h"
#include "Core/CharacterBase.h"
#include "Engine/DecalActor.h"
#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"

// Sets default values
AWeaponBase::AWeaponBase()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	
	Mesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("CharacterMesh1P"));
	Mesh->SetSimulatePhysics(true);
	RootComponent = Mesh;
	bReplicates = true;
	Mesh->SetComponentTickEnabled(false);
	Mesh->VisibilityBasedAnimTickOption = EVisibilityBasedAnimTickOption::OnlyTickPoseWhenRendered;

}

// Called when the game starts or when spawned
void AWeaponBase::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AWeaponBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void AWeaponBase::Pickup(ACharacterBase* Char)
{
	SetOwner(Char);
	CharacterOwner = Char;
	GetWorld()->GetSubsystem<UWorldCleanupManager>()->StopManagingWeapon(this);
	Mesh->SetComponentTickEnabled(true);
	OnWeaponPickedUp.Broadcast();
}

void AWeaponBase::Equip()
{
	// if (DrawSFX) UGameplayStatics::PlaySoundAtLocation(GetWorld(), DrawSFX, GetActorLocation());
}

void AWeaponBase::Drop()
{
	PrimaryFire_End();
	DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
	SetActorEnableCollision(true);
	Mesh->SetSimulatePhysics(true);
	SetOwner(nullptr);
	CharacterOwner = nullptr;
	GetWorld()->GetSubsystem<UWorldCleanupManager>()->ManageWeapon(this);
	Mesh->SetComponentTickEnabled(false);
	OnWeaponDropped.Broadcast();
}

void AWeaponBase::Holster()
{
	PrimaryFire_End();
	//SetActorHiddenInGame(true);
}

void AWeaponBase::GetAim(FVector& AimLocation, FVector& AimDirection)
{
	if (CharacterOwner)
	{
		FRotator Rot;
		CharacterOwner->GetActorEyesViewPoint(AimLocation, Rot);

		
		AimDirection = CharacterOwner->GetControlRotation().Vector();
	} else
	{
		AimLocation = Mesh->GetSocketLocation("Muzzle");
		AimDirection = GetActorRotation().Vector();
	}
}

void AWeaponBase::PrimaryFire_Start_Implementation()
{
}

void AWeaponBase::PrimaryFire_End_Implementation()
{
}


void AWeaponBase::SecondaryFire_Start_Implementation()
{
}

void AWeaponBase::SecondaryFire_End_Implementation()
{
}


void AWeaponBase::TertiaryFire_Implementation()
{
}

void AWeaponBase::Reload_Implementation()
{
}


void AWeaponBase::WeaponMelee_Implementation()
{
	ServerWeaponMelee();
}

void AWeaponBase::ServerWeaponMelee_Implementation()
{
	if (GetWorldTimerManager().TimerExists(MeleeCooldownHandle1)) return;
	FHitResult MeleeHit;
	GetMeleeHit(MeleeHit);
	FTimerDelegate TimerDel;
	TimerDel.BindUObject(this, &AWeaponBase::DoMeleeHit, MeleeHit);
	GetWorldTimerManager().SetTimer(MeleeHitDelayHandle, TimerDel, MeleeDelay, false);
	GetWorldTimerManager().SetTimer(MeleeCooldownHandle1, MeleeCooldownRate, false);
	//if (MeleeAnimation1P) CharacterOwner->GetMesh1P()->GetAnimInstance()->Montage_Play(MeleeAnimation1P);
	MulticastWeaponMelee();
	if (MeleeHit.GetActor() && MeleeHit.Distance < MeleeLungeRange)
	{
		if (const ACharacterBase* MeleeChar = Cast<ACharacterBase>(MeleeHit.GetActor()))
		{
			CharacterOwner->Lunge(MeleeHit.GetActor(), MeleeHit.ImpactPoint);
		}
	}
}

void AWeaponBase::MulticastWeaponMelee_Implementation()
{
	if (MeleeAnimation1P) CharacterOwner->GetMesh1P()->GetAnimInstance()->Montage_Play(MeleeAnimation1P);
	if (CharacterOwner->MeleeAnim) CharacterOwner->GetMesh()->GetAnimInstance()->Montage_Play(CharacterOwner->MeleeAnim);
	if (MeleeMissSound) UGameplayStatics::SpawnSoundAttached(MeleeMissSound, Mesh);
}

void AWeaponBase::DoMeleeHit_Implementation(const FHitResult MeleeHit)
{
	ServerMeleeHit(MeleeHit);
}

void AWeaponBase::ServerMeleeHit_Implementation(const FHitResult MeleeHit)
{
	if ((CharacterOwner->GetActorLocation() - MeleeHit.ImpactPoint).Length() <= MeleeDamageRange)
	{
		if (MeleeHit.GetActor())
		{
			if (MeleeHit.GetActor()->Implements<UDamageableInterface>())
			{
				const FVector Dir = (MeleeHit.GetActor()->GetActorLocation() - GetActorLocation()).GetSafeNormal();
				IDamageableInterface::Execute_CustomTakePointDamage(MeleeHit.GetActor(), MeleeDamage, Dir, MeleeHit, MeleeForce, GetInstigatorController(), this);
			}
			UPrimitiveComponent* HitComp = MeleeHit.GetComponent();
			if (HitComp && HitComp->IsSimulatingPhysics())
			{
				FVector ForceVector = (HitComp->GetComponentLocation() - GetActorLocation());
				ForceVector.Normalize();
				HitComp->AddImpulse(ForceVector*MeleeForce);
			}
		}
		MulticastDoMeleeHit(MeleeHit);
	}
}

void AWeaponBase::MulticastDoMeleeHit_Implementation(const FHitResult MeleeHit)
{
	if (MeleeImpactFX.Contains(MeleeHit.PhysMaterial->SurfaceType))
	{
		if (TSubclassOf<ADecalActor> MeleeImpactClass = *MeleeImpactFX.Find(MeleeHit.PhysMaterial->SurfaceType))
		{
			const FVector Loc = MeleeHit.Location;
			const FRotator Rot =  MeleeHit.ImpactNormal.Rotation() + FRotator(-90,0,0);
			GetWorld()->SpawnActor(MeleeImpactClass, &Loc, &Rot);
		}
	}
}


void AWeaponBase::GetMeleeHit(FHitResult& MeleeHit)
{
	FVector AimLoc;
	FVector AimDir;
	GetAim(AimLoc, AimDir);
	ActorsToIgnore.Add(CharacterOwner);
	UKismetSystemLibrary::LineTraceSingle(GetWorld(), AimLoc, AimLoc + (AimDir*MeleeLungeRange), UEngineTypes::ConvertToTraceType(ECC_Visibility), true, ActorsToIgnore, EDrawDebugTrace::None, MeleeHit, true);
}

void AWeaponBase::OnInteract_Implementation(ACharacterBase* Character)
{
	if (!Character->GameplayTags.HasTag(FGameplayTag::RequestGameplayTag(FName("Character.CanUseWeapons")))) return;
	IInteractableInterface::OnInteract_Implementation(Character);
	Character->PickupWeapon(this);
}

// void AWeaponBase::GetInteractInfo_Implementation(FText& Text, UTexture2D*& Icon, ACharacterBase* InteractingCharacter)
// {
// 	IInteractableInterface::GetInteractInfo_Implementation(Text, Icon, InteractingCharacter);
//
// 	Text = InteractText;
// 	Icon = InteractIcon;
// }

void AWeaponBase::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(AWeaponBase, CharacterOwner);
}

