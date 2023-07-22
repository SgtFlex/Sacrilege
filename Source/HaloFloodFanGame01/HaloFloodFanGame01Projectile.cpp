// Copyright Epic Games, Inc. All Rights Reserved.

#include "HaloFloodFanGame01Projectile.h"

#include "DamageableInterface.h"
#include "GunBase.h"
#include "NiagaraComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "Components/AudioComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Components/SphereComponent.h"
#include "Core/BaseCharacter.h"
#include "Engine/DamageEvents.h"
#include "Kismet/GameplayStatics.h"
#include "Perception/AISense_Hearing.h"

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
	if (IdleSound)
	{
		IdleSoundComponent = UGameplayStatics::SpawnSoundAttached(IdleSound, GetRootComponent());
		IdleSoundComponent->Play();
	}
}

void AHaloFloodFanGame01Projectile::OnProjectileOverlapped_Implementation(UPrimitiveComponent* OverlappedComponent,
	AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep,
	const FHitResult& SweepResult)
{
	UAISense_Hearing::ReportNoiseEvent(GetWorld(), SweepResult.Location, 300.0f, GetInstigator(), 300.0f);
	IDamageableInterface* DamageableActor = Cast<IDamageableInterface>(OtherActor);
	FVector Direction = GetVelocity();
	Direction.Normalize();
	if (UPrimitiveComponent* PrimComponent = Cast<UPrimitiveComponent>(OtherActor->GetRootComponent()))
	{
		if (PrimComponent->IsSimulatingPhysics())
		{
			PrimComponent->AddImpulse(Direction*Force);
		}
	}
	if ((OtherActor != nullptr) && (OtherActor != this) && (OtherComp != nullptr) && (DamageableActor))
	{
		FPointDamageEvent PointDamageEvent = FPointDamageEvent(Damage, SweepResult, Direction, UDamageType::StaticClass());
		DamageableActor->CustomTakePointDamage(PointDamageEvent, Force, GetInstigator()->GetController(), this);
	}
	
	AGunBase* Gun = Cast<AGunBase>(GetOwner());
	if (Gun && HitPFX)
	{
		UNiagaraFunctionLibrary::SpawnSystemAtLocation(GetWorld(), HitPFX, SweepResult.Location, SweepResult.Normal.Rotation());
		
	}
	if (HitSound) UGameplayStatics::PlaySoundAtLocation(GetWorld(), HitSound, SweepResult.Location);
	if (IdleSoundComponent) IdleSoundComponent->Stop();
	FTimerDelegate TimerDelegate;
	TimerDelegate.BindLambda([&]()
	{
		Destroy();
	});
	if (!GetWorldTimerManager().TimerExists(DespawnTimerHandle))
	{
		GetWorldTimerManager().SetTimer(DespawnTimerHandle, TimerDelegate, 5.0f, false);
	}
	
	CollisionComp->Deactivate();
	ProjectileMovement->Deactivate();
}

void AHaloFloodFanGame01Projectile::OnOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
                                                             UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (Cast<ABaseCharacter>(OtherActor) == GetInstigator() || OtherActor == GetOwner())
	{
		return;
	}
	OnProjectileOverlapped(OverlappedComponent, OtherActor, OtherComp, OtherBodyIndex, bFromSweep, SweepResult);
}
