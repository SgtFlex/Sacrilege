// Fill out your copyright notice in the Description page of Project Settings.


#include "VehicleBase.h"

#include "AIControllerBase.h"
#include "HealthComponent.h"
#include "Core/CharacterBase.h"
#include "Engine/DamageEvents.h"

// Sets default values
AVehicleBase::AVehicleBase()
{
 	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	VehicleMesh = CreateDefaultSubobject<UStaticMeshComponent>("VehicleMesh");
	SetRootComponent(VehicleMesh);
	HealthComponent = CreateDefaultSubobject<UHealthComponent>("HealthComponent");
}

// Called when the game starts or when spawned
void AVehicleBase::BeginPlay()
{
	Super::BeginPlay();
	HealthComponent->OnHealthUpdate.AddDynamic(this, &AVehicleBase::OnHealthUpdated);
	HealthComponent->OnHealthDepleted.AddDynamic(this, &AVehicleBase::OnHealthDepleted);
}

// Called every frame
void AVehicleBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

// Called to bind functionality to input
void AVehicleBase::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}

void AVehicleBase::SpawnDefaultControllerWithTeam(uint8 TeamId)
{
	SpawnDefaultController();
	if (AAIControllerBase* AIC = Cast<AAIControllerBase>(GetController()))
	{
		AIC->SetGenericTeamId(TeamId);
	}
}

void AVehicleBase::OnHealthUpdated(UHealthComponent* HealthComp)
{
	const float Health = HealthComponent->GetHealth();
	const float MaxHealth = HealthComponent->GetMaxHealth();
	if (Health <= MaxHealth && Health > MaxHealth * 0.75)
	{
		SetDamageState(Healthy);
	} else if (Health <= MaxHealth * 0.75 && Health > MaxHealth * 0.5)
	{
		SetDamageState(Damaged);
	} else
	{
		SetDamageState(Critical);
	}
}

void AVehicleBase::OnHealthDepleted(float Damage, FVector Force, FVector HitLocation, FName HitBoneName,
	AController* EventInstigator, AActor* DamageCauser)
{
	SetIsDestroyed(true);
}

void AVehicleBase::SetDamageState_Implementation(EDamageState NewDamageState)
{
	if (DamageState != NewDamageState)
	{
		OnDamageStateChanged.Broadcast(NewDamageState);
		DamageState = NewDamageState;
	}
}

void AVehicleBase::SetIsDestroyed(bool bNewIsDestroyed)
{
	bIsDestroyed = bNewIsDestroyed;
}

float AVehicleBase::CustomTakePointDamage_Implementation(FPointDamageEvent const& PointDamageEvent, float Force,
                                                         AController* EventInstigator, AActor* DamageCauser)
{
	HealthComponent->TakeDamage(PointDamageEvent.Damage, PointDamageEvent.ShotDirection*Force, PointDamageEvent.HitInfo.Location, PointDamageEvent.HitInfo.BoneName, EventInstigator, DamageCauser);
	return PointDamageEvent.Damage;
}

float AVehicleBase::CustomTakeRadialDamage_Implementation(float Force, FRadialDamageEvent const& RadialDamageEvent,
	AController* EventInstigator, AActor* DamageCauser)
{

	HealthComponent->TakeDamage(RadialDamageEvent.Params.BaseDamage, (GetActorLocation() - RadialDamageEvent.Origin).GetSafeNormal()*Force, this->GetActorLocation(), NAME_None, EventInstigator, DamageCauser);
	return RadialDamageEvent.Params.BaseDamage;
}

void AVehicleBase::OnInteract_Implementation(ACharacterBase* Character)
{
	IInteractableInterface::OnInteract_Implementation(Character);

	if (bIsDestroyed) return;
}
