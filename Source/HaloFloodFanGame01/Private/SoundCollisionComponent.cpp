// Fill out your copyright notice in the Description page of Project Settings.


#include "SoundCollisionComponent.h"

#include "Kismet/GameplayStatics.h"

// Sets default values for this component's properties
USoundCollisionComponent::USoundCollisionComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	
	// ...
}


// Called when the game starts
void USoundCollisionComponent::BeginPlay()
{
	Super::BeginPlay();

	if (Cast<UPrimitiveComponent>(ComponentReference.GetComponent(GetOwner())))
	{
		PrimitiveComponent = Cast<UPrimitiveComponent>(ComponentReference.GetComponent(GetOwner()));
		//UE_LOG(LogTemp, Warning, TEXT("Soundcollisioncomp successfully created"));
		PrimitiveComponent->OnComponentHit.AddDynamic(this, &USoundCollisionComponent::OnMyHit);
		
	}
}


// Called every frame
void USoundCollisionComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}

void USoundCollisionComponent::OnMyHit(UPrimitiveComponent* HitComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	//DrawDebugLine(GetWorld(), Hit.ImpactPoint, Hit.ImpactPoint + (NormalImpulse.GetSafeNormal() * 50), FColor(255, 0, 0), false, 5);
	if (HitSound)
	{
		float AngleDifference =  FMath::RadiansToDegrees(FMath::Acos(HitComponent->GetComponentVelocity().GetSafeNormal().Dot(Hit.ImpactNormal)));

		//UE_LOG(LogTemp, Warning, TEXT("%f"), AngleDifference);
		if (HitComponent->GetComponentVelocity().Length() > 150 && AngleDifference < 85) {
			UGameplayStatics::PlaySoundAtLocation(GetWorld(), HitSound, Hit.ImpactPoint, FMath::Max(1, HitComponent->GetComponentVelocity().Length() / 250));
		}
	}
}

