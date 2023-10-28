// Fill out your copyright notice in the Description page of Project Settings.


#include "BaseGrenade.h"

#include "DamageableInterface.h"
#include "HaloHUDWidget.h"
#include "MyCustomBlueprintFunctionLibrary.h"
#include "NiagaraFunctionLibrary.h"
#include "PickupComponent.h"
#include "Engine/DamageEvents.h"
#include "GameFramework/ProjectileMovementComponent.h"
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
	SetRootComponent(Mesh);

	

	ProjectileMovementComponent = CreateDefaultSubobject<UProjectileMovementComponent>("ProjectileMovementComp");

	PickupComponent = CreateDefaultSubobject<UPickupComponent>("PickupComp");
	PickupComponent->SetupAttachment(GetRootComponent());
	PickupComponent->SetEnabled(false);
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

void ABaseGrenade::Explode_Implementation()
{
	if (ExplosionSFX) UGameplayStatics::PlaySoundAtLocation(GetWorld(), ExplosionSFX, GetActorLocation());
	if (ExplosionPFX) UNiagaraFunctionLibrary::SpawnSystemAtLocation(GetWorld(), ExplosionPFX, GetActorLocation());
	TArray<AActor*> ActorsToIgnore;
	UMyCustomBlueprintFunctionLibrary::FireExplosion(GetWorld(), ActorsToIgnore, GetActorLocation(), MaxExplosionDamage, MinExplosionDamage, OuterExplosionRadius, InnerExplosionRadius, ExplosionDamageFalloff, ExplosionForce, this, Cast<APawn>(GetOwner())->GetController());
	Destroy();
}

void ABaseGrenade::SetArmed(bool NewArmed)
{
	bArmed = NewArmed;
	PickupComponent->SetEnabled(!bArmed);
	if (NewArmed)
	{
		Mesh->OnComponentHit.AddDynamic(this, &ABaseGrenade::OnCollide);
	} else
	{
		Mesh->OnComponentHit.RemoveDynamic(this, &ABaseGrenade::OnCollide);
	}
}

void ABaseGrenade::Arm(float ArmTime)
{
	if (FuseStarted) return;
	FuseStarted = true;
	if (ArmTime > 0)
	{
		if (!GetWorldTimerManager().TimerExists(FuseTimer) || ArmTime < GetWorldTimerManager().GetTimerRemaining(FuseTimer))
		{
			bArmed = true;
			GetWorldTimerManager().SetTimer(FuseTimer, this, &ABaseGrenade::Explode, ArmTime);
		}
	} else
	{
		Explode();
	}
	
}

void ABaseGrenade::OnCollide_Implementation(UPrimitiveComponent* HitComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	if (!bArmed) return;
	Arm(FuseTime);
}

void ABaseGrenade::Pickup(AHaloFloodFanGame01Character* Character)
{
	IPickupInterface::Pickup(Character);
	bool FoundGrenade = false;
	for (int i = 0; i < Character->GrenadeInventory.Num(); i++)
	{
		if (Character->GrenadeInventory[i].GrenadeClass == GetClass())
		{
			FoundGrenade = true;
			if (Character->GrenadeInventory[i].GrenadeAmount < 4)
			{
				Character->GrenadeInventory[i].GrenadeAmount++;
				Destroy();
			}
		}
	}
	
	if (!FoundGrenade)
	{
		FGrenadeStruct GrenadeType;
		GrenadeType.GrenadeClass = GetClass();
		GrenadeType.GrenadeAmount = 1;
		Character->GrenadeInventory.Add(GrenadeType);
		Destroy();
	}
	Character->OnGrenadeInvetoryUpdated.Broadcast(Character->GrenadeInventory);
}

float ABaseGrenade::CustomOnTakeAnyDamage(float DamageAmount, FVector Force,
                               AController* EventInstigator, AActor* DamageCauser)
{
	IDamageableInterface::CustomOnTakeAnyDamage(DamageAmount, Force, EventInstigator, DamageCauser);
	if (EventInstigator) SetInstigator(EventInstigator->GetPawn());
	this->Arm(FMath::RandRange(0.25, 0.5));
	return DamageAmount;
}
