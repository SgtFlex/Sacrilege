// Copyright Epic Games, Inc. All Rights Reserved.

#include "HaloFloodFanGame01Projectile.h"

#include "DamageableInterface.h"
#include "GunBase.h"
#include "NiagaraFunctionLibrary.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Components/SphereComponent.h"
#include "Core/BaseCharacter.h"
#include "Engine/DamageEvents.h"

AHaloFloodFanGame01Projectile::AHaloFloodFanGame01Projectile() 
{
	// Use a sphere as a simple collision representation
	CollisionComp = CreateDefaultSubobject<USphereComponent>(TEXT("SphereComp"));
	CollisionComp->InitSphereRadius(5.0f);
	CollisionComp->SetGenerateOverlapEvents(true);
	CollisionComp->BodyInstance.SetCollisionProfileName("Projectile");
	
	// Players can't walk on it
	CollisionComp->SetWalkableSlopeOverride(FWalkableSlopeOverride(WalkableSlope_Unwalkable, 0.f));
	CollisionComp->CanCharacterStepUpOn = ECB_No;

	// Set as root component
	RootComponent = CollisionComp;

	// Use a ProjectileMovementComponent to govern this projectile's movement
	ProjectileMovement = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileComp"));
	ProjectileMovement->UpdatedComponent = CollisionComp;
	ProjectileMovement->InitialSpeed = 3000.f;
	ProjectileMovement->MaxSpeed = 3000.f;
	ProjectileMovement->bRotationFollowsVelocity = true;
	ProjectileMovement->bShouldBounce = true;

	// Die after 3 seconds by default
	InitialLifeSpan = 5.0f;
}

void AHaloFloodFanGame01Projectile::BeginPlay()
{
	Super::BeginPlay();

	CollisionComp->OnComponentBeginOverlap.AddDynamic(this, &AHaloFloodFanGame01Projectile::OnOverlap);
}

void AHaloFloodFanGame01Projectile::OnOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
                                              UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (Cast<ABaseCharacter>(OtherActor) == GetInstigator() || OtherActor == GetOwner())
	{
		return;
	}
	IDamageableInterface* DamageableActor = Cast<IDamageableInterface>(OtherActor);
	FVector Direction = GetVelocity();
	Direction.Normalize();
	if ((OtherActor != nullptr) && (OtherActor != this) && (OtherComp != nullptr) && (DamageableActor))
	{
		FPointDamageEvent PointDamageEvent = FPointDamageEvent(Damage, SweepResult, Direction, UDamageType::StaticClass());
		DamageableActor->TakePointDamage(PointDamageEvent, Direction*Force, GetInstigator()->GetController(), this);
	}
	if (OverlappedComponent->IsSimulatingPhysics())
	{
		OverlappedComponent->AddImpulse(Direction*Force);
	}
	if (AGunBase* Gun = Cast<AGunBase>(GetOwner())) UNiagaraFunctionLibrary::SpawnSystemAtLocation(GetWorld(), Gun->MuzzlePFX, SweepResult.Location, SweepResult.Normal.Rotation());
	Destroy();
}
