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
	RadialDamageEvent.Params = FRadialDamageParams(MaxExplosionDamage, MinExplosionDamage, InnerExplosionRadius, OuterExplosionRadius, ExplosionDamageFalloff);
	RadialDamageEvent.Origin = GetActorLocation();
	if (ExplosionSFX) UGameplayStatics::PlaySoundAtLocation(GetWorld(), ExplosionSFX, GetActorLocation());
	if (ExplosionPFX) UNiagaraFunctionLibrary::SpawnSystemAtLocation(GetWorld(), ExplosionPFX, GetActorLocation());
	TArray<FHitResult> HitResults;
	FVector Origin = GetActorLocation();
	FCollisionShape Sphere = FCollisionShape::MakeSphere(OuterExplosionRadius);
	//GetWorld()->Sweep
	TArray<TEnumAsByte<EObjectTypeQuery>> Objects;
	TArray<AActor*> ActorsToIgnore;
	TArray<AActor*> HitActors;
	UKismetSystemLibrary::SphereOverlapActors(GetWorld(), Origin, OuterExplosionRadius, Objects, AActor::StaticClass(), ActorsToIgnore, HitActors);
	//GetWorld()->SweepMultiByChannel(HitResults, Origin, Origin, FQuat(0,0,0,0), ECollisionChannel::ECC_Visibility, Sphere);
	for (auto HitActor : HitActors)
	{
		FVector Direction = HitActor->GetActorLocation() - GetActorLocation();
		Direction.Normalize();
		// if (HitResult.GetComponent()->IsSimulatingPhysics())
		// {
		// 	HitResult.GetComponent()->AddImpulse(Force*ExplosionForce, HitResult.BoneName);
		// }
		UE_LOG(LogTemp, Warning, TEXT("Hit actor: %s"), *HitActor->GetActorLabel());
		if (IDamageableInterface* HitDamageable = Cast<IDamageableInterface>(HitActor))
		{
			HitDamageable->TakeRadialDamage(ExplosionForce, RadialDamageEvent);
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

float ABaseGrenade::TakePointDamage(FPointDamageEvent const& PointDamageEvent, FVector Force, AController* Controller,
	AActor* DamageCauser)
{
	this->Arm(FMath::RandRange(0.25, 0.5));
	UE_LOG(LogTemp, Warning, TEXT("Logged point damage"));
	return 0;
}

float ABaseGrenade::TakeDamage(float DamageAmount, FVector Force, FDamageEvent const& DamageEvent,
                               AController* EventInstigator, AActor* DamageCauser)
{
	this->Arm(FMath::RandRange(0.25, 0.5));
	UE_LOG(LogTemp, Warning, TEXT("Logged damage"));
	return 0;
}
