// Fill out your copyright notice in the Description page of Project Settings.


#include "WeaponBase.h"

#include "WorldCleanupManager.h"
#include "Camera/CameraComponent.h"
#include "Core/CharacterBase.h"
#include "Engine/DecalActor.h"
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
	FHitResult MeleeHit;
	CharacterOwner->GetPlayerAim(MeleeHit);

	if (MeleeHit.GetActor() && MeleeHit.Distance < MeleeDistance)
	{
		ACharacterBase* MeleeChar = Cast<ACharacterBase>(MeleeHit.GetActor());
		if (MeleeChar)
		{
			CharacterOwner->Lunge(MeleeHit.GetActor());
			MeleeContact(MeleeHit, MeleeHit.GetActor());
		} else
		{
			MeleeContact(MeleeHit, MeleeHit.GetActor());
		}
	} else
	{
		MeleeContact(MeleeHit);
	}
}

void AWeaponBase::MeleeContact_Implementation(FHitResult& MeleeHit, AActor* Actor)
{
	if (Actor)
	{
		if (Actor->Implements<UDamageableInterface>())
		{
			const FVector Dir = (Actor->GetActorLocation() - GetActorLocation()).GetSafeNormal();
			IDamageableInterface::Execute_CustomTakePointDamage(Actor, MeleeDamage, Dir, MeleeHit, MeleeForce, GetInstigatorController(), this);
		}
		UPrimitiveComponent* HitComp = MeleeHit.GetComponent();
		if (HitComp && HitComp->IsSimulatingPhysics())
		{
			FVector ForceVector = (HitComp->GetComponentLocation() - GetActorLocation());
			ForceVector.Normalize();
			HitComp->AddImpulse(ForceVector*MeleeForce);
		}
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
}



void AWeaponBase::OnInteract_Implementation(ACharacterBase* Character)
{
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

