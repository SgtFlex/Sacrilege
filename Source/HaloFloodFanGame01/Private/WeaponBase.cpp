// Fill out your copyright notice in the Description page of Project Settings.


#include "WeaponBase.h"

#include "WorldCleanupManager.h"
#include "Core/CharacterBase.h"
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

