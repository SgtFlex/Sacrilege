// Fill out your copyright notice in the Description page of Project Settings.


#include "BulletFiringComponent.h"

#include "NiagaraFunctionLibrary.h"
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

AProjectileBase* UBulletFiringComponent::FireProjectile(TSubclassOf<AProjectileBase> ProjectileClass, FVector Direction, AActor* Owner, AController* Instigator)
{
	FVector Location = GetComponentLocation();
	FRotator Rotation = Direction.Rotation();
	FActorSpawnParameters ActorSpawnParameters;
	ActorSpawnParameters.Owner = Owner;
	if (Instigator)
		if (APawn* Pawn = Instigator->GetPawn())
			ActorSpawnParameters.Instigator = Pawn;
	UGameplayStatics::PlaySoundAtLocation(GetWorld(), FiringSound, Location, Rotation);
	UNiagaraFunctionLibrary::SpawnSystemAttached(FiringVFX, this, NAME_None, FVector(0,0,0), FRotator(0,0,0), EAttachLocation::SnapToTarget, true);
	return GetWorld()->SpawnActor<AProjectileBase>(ProjectileClass, Location, Rotation, ActorSpawnParameters);
}

void UBulletFiringComponent::FireBullet(FHitResult& HitResult, FVector EndLocation)
{
}


// Called every frame
void UBulletFiringComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}

