// Fill out your copyright notice in the Description page of Project Settings.


#include "Core/PhysicsActor.h"

#include "Engine/DamageEvents.h"
#include "Kismet/GameplayStatics.h"

// Sets default values
APhysicsActor::APhysicsActor()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	StaticMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("SM"));
	StaticMesh->SetNotifyRigidBodyCollision(true);
	StaticMesh->SetSimulatePhysics(true);

}

// Called when the game starts or when spawned
void APhysicsActor::BeginPlay()
{
	Super::BeginPlay();

}

// Called every frame
void APhysicsActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void APhysicsActor::NotifyHit(UPrimitiveComponent* MyComp, AActor* Other, UPrimitiveComponent* OtherComp,
	bool bSelfMoved, FVector HitLocation, FVector HitNormal, FVector NormalImpulse, const FHitResult& Hit)
{
	Super::NotifyHit(MyComp, Other, OtherComp, bSelfMoved, HitLocation, HitNormal, NormalImpulse, Hit);

	UE_LOG(LogTemp, Warning, TEXT("%f"), NormalImpulse.Length());
	TSubclassOf<UDamageType> DamageType;
	UGameplayStatics::ApplyDamage(Other, 5, nullptr, this, DamageType);
}

