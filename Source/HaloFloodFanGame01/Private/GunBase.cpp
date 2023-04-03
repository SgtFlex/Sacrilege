// Fill out your copyright notice in the Description page of Project Settings.


#include "GunBase.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "HaloHUDWidget.h"
#include "HealthComponent.h"
#include "VectorTypes.h"
#include "Engine/DamageEvents.h"
#include "Kismet/KismetMathLibrary.h"


#include "HaloFloodFanGame01/HaloFloodFanGame01Character.h"
#include "NaniteUtilities/Public/Affine.h"


// Sets default values
AGunBase::AGunBase()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	
	Mesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("CharacterMesh1P"));
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
int32 BulletsFired;
void AGunBase::PullTrigger()
{
	UE_LOG(LogTemp, Warning, TEXT("Trigger pulled"));
	BulletsFired = 0;
	GetWorldTimerManager().SetTimer(FireHandle, this, &AGunBase::Fire, 60/FireRate, true, 0);
}

void AGunBase::ReleaseTrigger()
{
	UE_LOG(LogTemp, Warning, TEXT("Trigger released"));
	GetWorldTimerManager().ClearTimer(FireHandle);
}

void AGunBase::OnInteract_Implementation(AHaloFloodFanGame01Character* Character)
{
	IInteractableInterface::OnInteract_Implementation(Character);

	UE_LOG(LogTemp, Warning, TEXT("Implementation"));

	Character->PickupWeapon(this);
}

void AGunBase::Fire_Implementation()
{
	UE_LOG(LogTemp, Warning, TEXT("Gun firing"));
	if (CurMagazine <= 0) return;
	CurMagazine--;
	if (HUDRef) HUDRef->SetBulletUsed(CurMagazine, false);
	ABaseCharacter* OwningChar = Cast<ABaseCharacter>(GetOwner());
	if (!OwningChar) return;
	UGameplayStatics::PlaySoundAtLocation(GetWorld(), FiringSound, GetActorLocation());
	if (Mesh->DoesSocketExist("Muzzle") && MuzzlePFX)
	{
		UGameplayStatics::SpawnEmitterAttached(MuzzlePFX, GetRootComponent(), "Muzzle");
	}
	if (PrimProj)
	{
		FVector Location = GetActorLocation() + GetActorForwardVector()*50.0f;
		FRotator Rotation = OwningChar->GetBaseAimRotation();
		AActor* SpawnedProj = GetWorld()->SpawnActor(PrimProj, &Location, &Rotation);
		//Cast<UPrimitiveComponent>(SpawnedProj->GetRootComponent())->AddImpulse(Camera->GetForwardVector()*1000.0f);
	} else
	{
		FHitResult Hit;
		FVector TraceStart = GetActorLocation();
		FRotator Dir;
		FVector TraceEnd = GetActorLocation() + OwningChar->GetBaseAimRotation().Vector()*500;
		OwningChar->GetActorEyesViewPoint(TraceStart, Dir);
		TraceEnd = TraceStart + Dir.Vector()*Range;
		FCollisionQueryParams CollisionParameters;
		CollisionParameters.AddIgnoredActor(this);
		CollisionParameters.AddIgnoredActor(GetAttachParentActor());
		GetWorld()->LineTraceSingleByChannel(Hit, TraceStart, TraceEnd, ECollisionChannel::ECC_WorldDynamic, CollisionParameters);
		DrawDebugLine(GetWorld(), TraceStart, TraceEnd, FColor(255, 0, 0), false, 3);
		IDamageableInterface* HitActor = Cast<IDamageableInterface>(Hit.GetActor());
		if (Hit.bBlockingHit)
		{
			FVector ForceVector = (TraceEnd-TraceStart);
			ForceVector.Normalize();
			
			if (Hit.GetComponent()->IsSimulatingPhysics())
				Hit.GetComponent()->AddImpulse(ForceVector*Force);
			
			if (HitActor)
			{
				FVector HitDir = Hit.Location - TraceStart;
				HitDir.Normalize();
				FPointDamageEvent PointDamageEvent;
				PointDamageEvent.Damage = Damage;
				PointDamageEvent.HitInfo = Hit;
				HitActor->TakePointDamage(Damage, HitDir*Force, PointDamageEvent);
			}
		}
	}
	BulletsFired++;
	if (BulletsFired==BurstAmount)
	{
		ReleaseTrigger();
	}
}

void AGunBase::Reload_Implementation()
{
	if (CurReserve <= 0 || CurMagazine == MaxMagazine) return;
	int32 AmountNeed = MaxMagazine - CurMagazine; //32 - 27 gives 5 for example
	int32 AmountGrabbed = FMath::Min(AmountNeed, CurReserve);
	CurMagazine = CurMagazine + AmountGrabbed;
	CurReserve = CurReserve - AmountGrabbed;
}

