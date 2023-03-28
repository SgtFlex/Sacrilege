// Fill out your copyright notice in the Description page of Project Settings.


#include "Core/BasePawn.h"

#include "Components/CapsuleComponent.h"

// Sets default values
ABasePawn::ABasePawn()
{
 	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	CapsuleCollider = CreateDefaultSubobject<UCapsuleComponent>(TEXT("Capsule"));
	RootComponent = CapsuleCollider;
	MainSkelMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("MainSkelMesh"));
	MainSkelMesh->SetupAttachment(CapsuleCollider);
}

// Called when the game starts or when spawned
void ABasePawn::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void ABasePawn::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

// Called to bind functionality to input
void ABasePawn::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}

void ABasePawn::Death_Implementation()
{
	FTimerHandle CleanupTimer;
	MainSkelMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	MainSkelMesh->SetSimulatePhysics(true);
	GetWorldTimerManager().SetTimer(CleanupTimer, this, &ABasePawn::DeathCleanup, 5);
}

void ABasePawn::DeathCleanup()
{
	
}

float ABasePawn::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator,
                            AActor* DamageCauser)
{
	float DamageLeft = DamageAmount;
	//GetWorldTimerManager().SetTimer(ShieldDelayTimerHandle, this, &AHaloFloodFanGame01Character::RegenShield, ShieldRegenTickRate, true, ShieldRegenDelay);
	if (CurShields > 0)
	{
		DamageLeft = DamageAmount - CurShields;
		CurShields -= DamageAmount;
	} 
	if (CurShields <= 0 && MaxHealth > 0)
	{
		MaxHealth -= DamageLeft;
		if (MaxHealth <= 0)
		{
			this->Death();
		}
	}
	return DamageAmount;
}
