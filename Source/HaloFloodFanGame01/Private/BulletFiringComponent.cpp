// Fill out your copyright notice in the Description page of Project Settings.


#include "BulletFiringComponent.h"

#include "Bullet.h"
#include "MyCustomBlueprintFunctionLibrary.h"
#include "NiagaraFunctionLibrary.h"
#include "Engine/PointLight.h"
#include "HaloFloodFanGame01/ProjectileBase.h"
#include "Kismet/GameplayStatics.h"

// Sets default values for this component's properties
UBulletFiringComponent::UBulletFiringComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
}


// Called when the game starts
void UBulletFiringComponent::BeginPlay()
{
	Super::BeginPlay();

	// ...
	
}

AProjectileBase* UBulletFiringComponent::FireProjectile(TSubclassOf<AProjectileBase> ProjectileClass, FVector Direction, AActor* Owner, AController* Instigator, const TArray<
                                                        AActor*>& IgnoreActors)
{
	OnBulletFired.Broadcast();
	FVector Location = GetComponentLocation() + Direction*50.0f;
	FRotator Rotation = Direction.Rotation();
	FActorSpawnParameters ActorSpawnParameters;
	ActorSpawnParameters.Owner = GetOwner();
	ActorSpawnParameters.Instigator = GetOwner()->GetInstigator();
	ActorSpawnParameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	APawn* PawnInstigator = nullptr;
	if (Instigator)
		if (APawn* Pawn = Instigator->GetPawn())
			PawnInstigator = Pawn;
	PlayFX(Location, Rotation);
	const FTransform SpawnTransform = FTransform( Rotation + FRotator(FMath::RandRange(-VerticalSpread, VerticalSpread), FMath::RandRange(-HorizontalSpread, HorizontalSpread),0), Location);
	AProjectileBase* Bullet = GetWorld()->SpawnActorDeferred<AProjectileBase>(ProjectileClass, SpawnTransform, Owner, PawnInstigator, ESpawnActorCollisionHandlingMethod::AlwaysSpawn);
	Bullet->IgnoreActors.Append(IgnoreActors);
	Bullet->FinishSpawning(SpawnTransform);
	return Bullet;
	//return GetWorld()->SpawnActor<AProjectileBase>(ProjectileClass, Location, Rotation + FRotator(FMath::RandRange(-VerticalSpread, VerticalSpread), FMath::RandRange(-HorizontalSpread, HorizontalSpread),0), ActorSpawnParameters);
}

void UBulletFiringComponent::PlayFX_Implementation(FVector Location, FRotator Rotation)
{
	
	UGameplayStatics::PlaySoundAtLocation(GetWorld(), FiringSound, Location, Rotation);
	if (PointLightClass) GetWorld()->SpawnActor<AActor>(PointLightClass, GetComponentLocation(), FRotator::ZeroRotator);
	UNiagaraFunctionLibrary::SpawnSystemAttached(FiringVFX, this, NAME_None, FVector(0,0,0), FRotator(0,0,0), EAttachLocation::SnapToTarget, true);
}

void UBulletFiringComponent::FireBullet(FHitResult& HitResult, FVector AimLocation, FVector Direction,  TArray<AActor*> ActorsToIgnore, AActor* DamageCauser, AController* EventInstigator)
{
	OnBulletFired.Broadcast();
	if (!BulletInfo) return;
	
	UMyCustomBlueprintFunctionLibrary::FireHitScanBullet(HitResult, ActorsToIgnore, AimLocation,
	                                                     (Direction.Rotation() + FRotator(FMath::RandRange(-VerticalSpread, VerticalSpread), FMath::RandRange(-HorizontalSpread, HorizontalSpread),0)).Vector(), BulletInfo.GetDefaultObject()->Range,BulletInfo.GetDefaultObject()->FalloffCurve, BulletInfo.GetDefaultObject()->Damage, BulletInfo.GetDefaultObject()->Force, DamageCauser, EventInstigator);
	OnBulletHit.Broadcast(HitResult);
	PlayFX(GetComponentLocation(), Direction.Rotation());
}


// Called every frame
void UBulletFiringComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}

