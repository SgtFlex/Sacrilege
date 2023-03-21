// Fill out your copyright notice in the Description page of Project Settings.


#include "GunBase.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "HaloHUDWidget.h"
#include "Kismet/KismetMathLibrary.h"


#include "HaloFloodFanGame01/HaloFloodFanGame01Character.h"


// Sets default values
AGunBase::AGunBase()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	Mesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("CharacterMesh1P"));
	Mesh->SetupAttachment(GetRootComponent());
	Mesh->SetSimulatePhysics(true);

	
}

// Called when the game starts or when spawned
void AGunBase::BeginPlay()
{
	Super::BeginPlay();

	CurMagazine = MaxMagazine;
	CurReserve = MaxReserve;
	
}

// Called every frame
void AGunBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void AGunBase::OnInteract_Implementation(AHaloFloodFanGame01Character* Character)
{
	IInteractableInterface::OnInteract_Implementation(Character);

	UE_LOG(LogTemp, Warning, TEXT("Implementation"));

	Character->PickupWeapon(this);
}

void AGunBase::PrimaryAttack_Implementation()
{
	if (CurMagazine <= 0) return;
	CurMagazine--;
	if (HUDRef) HUDRef->SetBulletUsed(CurMagazine, false);
	if (PrimProj && Camera)
	{
		FVector Location = (Camera->GetComponentLocation()) + Camera->GetForwardVector()*50.0f;
		FRotator Rotation = (Camera->GetComponentRotation());
		AActor* SpawnedProj = GetWorld()->SpawnActor(PrimProj, &Location, &Rotation);
		//Cast<UPrimitiveComponent>(SpawnedProj->GetRootComponent())->AddImpulse(Camera->GetForwardVector()*1000.0f);
	}
}

void AGunBase::SecondaryAttack_Implementation()
{
}

void AGunBase::Fire_Implementation()
{
}

void AGunBase::Reload_Implementation()
{
	if (CurReserve <= 0 || CurMagazine == MaxMagazine) return;
	int32 AmountNeed = MaxMagazine - CurMagazine; //32 - 27 gives 5 for example
	int32 AmountGrabbed = FMath::Min(AmountNeed, CurReserve);
	CurMagazine = CurMagazine + AmountGrabbed;
	CurReserve = CurReserve - AmountGrabbed;
}

