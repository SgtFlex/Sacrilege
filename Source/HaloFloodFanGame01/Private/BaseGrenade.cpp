// Fill out your copyright notice in the Description page of Project Settings.


#include "BaseGrenade.h"

#include "DamageableInterface.h"
#include "HaloHUDWidget.h"
#include "NiagaraFunctionLibrary.h"
#include "PickupComponent.h"
#include "Engine/DamageEvents.h"
#include "HaloFloodFanGame01/HaloFloodFanGame01Character.h"
#include "Kismet/GameplayStatics.h"

// Sets default values
ABaseGrenade::ABaseGrenade()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	Mesh = CreateDefaultSubobject<UStaticMeshComponent>("Mesh");
	Mesh->SetSimulatePhysics(true);
	Mesh->SetNotifyRigidBodyCollision(true);

	PickupComponent = CreateDefaultSubobject<UPickupComponent>("PickupComp");
	PickupComponent->SetupAttachment(Mesh);
	RootComponent = Mesh;
}

// Called when the game starts or when spawned
void ABaseGrenade::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void ABaseGrenade::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void ABaseGrenade::Explode()
{
	FRadialDamageEvent RadialDamageEvent;
	RadialDamageEvent.Params = FRadialDamageParams(MaxExplosionDamage, InnerExplosionRadius, OuterExplosionRadius, ExplosionDamageFalloff);
	RadialDamageEvent.Origin = GetActorLocation();
	if (ExplosionSFX) UGameplayStatics::PlaySoundAtLocation(GetWorld(), ExplosionSFX, GetActorLocation());
	if (ExplosionPFX) UNiagaraFunctionLibrary::SpawnSystemAtLocation(GetWorld(), ExplosionPFX, GetActorLocation());
	TArray<FHitResult> HitActors;
	FVector Origin = GetActorLocation();
	FCollisionShape Sphere = FCollisionShape::MakeSphere(OuterExplosionRadius);
	GetWorld()->SweepMultiByChannel(HitActors, Origin, Origin, FQuat(0,0,0,0), ECollisionChannel::ECC_Visibility, Sphere);
	for (auto HitActor : HitActors)
	{
		FVector Force = HitActor.Component->GetComponentLocation() - GetActorLocation();
		Force.Normalize();

		if (HitActor.GetComponent()->IsSimulatingPhysics())
		{
			HitActor.GetComponent()->AddImpulse(Force*ExplosionForce, HitActor.BoneName);
		}
		if (IDamageableInterface* HitDamageable = Cast<IDamageableInterface>(HitActor.GetActor()))
		{
			HitDamageable->TakeRadialDamage(MaxExplosionDamage, Force*ExplosionForce, RadialDamageEvent);
		}
	}
	Destroy();
}

void ABaseGrenade::SetArmed(bool bNewArmed)
{
	bArmed = bNewArmed;
	if (bArmed)
		PickupComponent->SetEnabled(false);
	else
		PickupComponent->SetEnabled(true);
}

void ABaseGrenade::Arm(float ArmTime)
{
	if (!ArmTime) ArmTime = FuseTime;
	GetWorldTimerManager().SetTimer(FuseTimer, this, &ABaseGrenade::Explode, ArmTime);
}

void ABaseGrenade::NotifyHit(UPrimitiveComponent* MyComp, AActor* Other, UPrimitiveComponent* OtherComp,
                             bool bSelfMoved, FVector HitLocation, FVector HitNormal, FVector NormalImpulse, const FHitResult& Hit)
{
	if (!bArmed) return
	Super::NotifyHit(MyComp, Other, OtherComp, bSelfMoved, HitLocation, HitNormal, NormalImpulse, Hit);
	Mesh->SetNotifyRigidBodyCollision(false);
	Arm();
}

void ABaseGrenade::Pickup(AHaloFloodFanGame01Character* Character)
{
	IPickupInterface::Pickup(Character);

	if (Character->FragCount < 4)
	{
		Character->SetFragCount(Character->FragCount+1);
		Destroy();
	}
	
}

float ABaseGrenade::TakePointDamage(float DamageAmount, FVector Force, FPointDamageEvent const& PointDamageEvent,
	AController* Controller, AActor* DamageCauser)
{
	this->Arm(FMath::RandRange(0.25, 0.5));
	return IDamageableInterface::TakePointDamage(DamageAmount, Force, PointDamageEvent, Controller, DamageCauser);
}

float ABaseGrenade::TakeRadialDamage(float DamageAmount, FVector Force, FRadialDamageEvent const& RadialDamageEvent,
	AController* EventInstigator, AActor* DamageCauser)
{
	this->Arm(FMath::RandRange(0.25, 0.5));
	return IDamageableInterface::
		TakeRadialDamage(DamageAmount, Force, RadialDamageEvent, EventInstigator, DamageCauser);
}
