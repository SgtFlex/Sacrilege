// Fill out your copyright notice in the Description page of Project Settings.


#include "MyCustomBlueprintFunctionLibrary.h"

#include "DamageableInterface.h"
#include "AI/NavigationSystemBase.h"
#include "Engine/DamageEvents.h"
#include "Kismet/GameplayStatics.h"
#include "Perception/AISense_Hearing.h"

void UMyCustomBlueprintFunctionLibrary::Ignite(UPrimitiveComponent* Component, float DamagePerSecond, float Duration)
{
}

void UMyCustomBlueprintFunctionLibrary::FireHitScanBullet(FHitResult& Hit, const UObject* WorldContextObject, TArray<AActor*>& ActorsToIgnore, FVector StartLocation, FVector Direction, float Range, UCurveFloat* FalloffCurve, float Damage, float Force, AActor* DamageCauser, AController* EventInstigator)
{
	if (UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull))
	{
		UKismetSystemLibrary::SphereTraceSingle(World, StartLocation, StartLocation + (Direction * Range), 20, UEngineTypes::ConvertToTraceType(ECollisionChannel::ECC_Camera), false, ActorsToIgnore, EDrawDebugTrace::None, Hit, true, FLinearColor::Red, FLinearColor::Green, 5);
		//DrawDebugLine(World, StartLocation, StartLocation + (Direction * Range), FColor(0, 255, 0, 255), false, 5, 0, 5);
		if (Hit.bBlockingHit)
		{
			FVector HitDir = (Hit.Location - StartLocation).GetSafeNormal();
			UAISense_Hearing::ReportNoiseEvent(World, Hit.Location, 1.0f, EventInstigator);
			if (IDamageableInterface* DamageableActor = Cast<IDamageableInterface>(Hit.GetActor()))
			{
				Damage = FalloffCurve!=nullptr ? Damage * FalloffCurve->GetFloatValue(Hit.Distance/Range) : Damage;
				FPointDamageEvent PointDamageEvent = FPointDamageEvent(Damage, Hit, HitDir, UDamageType::StaticClass());
				DamageableActor->CustomTakePointDamage(PointDamageEvent, Force, EventInstigator, DamageCauser);
			}
			if (Hit.GetComponent() && Hit.GetComponent()->IsSimulatingPhysics())
			{
				Hit.GetComponent()->AddImpulse(HitDir*Force);
			}
		}
	}
}

AActor* UMyCustomBlueprintFunctionLibrary::FireProjectile(const UObject* WorldContextObject, FVector StartLocation,
	FVector Direction, TSubclassOf<AActor> ActorToSpawn)
{
	AActor* Actor = nullptr;
	
	if (UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull))
	{
		FRotator Rotation = Direction.Rotation();
		Actor = World->SpawnActor(ActorToSpawn, &StartLocation, &Rotation);
	}
	
	return Actor;
}


void UMyCustomBlueprintFunctionLibrary::FireExplosion(const UObject* WorldContextObject, TArray<AActor*>& ActorsToIgnore, FVector Location, float BaseDamage, float MinimumDamage, float OuterRadius, float InnerRadius, float DamageFalloff, float Force, AActor* DamageCauser, AController* EventInstigator)
{
	TArray<AActor*> Actors;

	if (UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull))
	{
		FRadialDamageEvent RadialDamageEvent;
		RadialDamageEvent.Params = FRadialDamageParams(BaseDamage, MinimumDamage, InnerRadius, OuterRadius, DamageFalloff);
		RadialDamageEvent.Origin = Location;
		TArray<TEnumAsByte<EObjectTypeQuery>> Objects;
		TArray<AActor*> HitActors;
		UKismetSystemLibrary::SphereOverlapActors(World, Location, OuterRadius, Objects, AActor::StaticClass(), ActorsToIgnore, HitActors);
		for (auto HitActor : HitActors)
		{
			if (IDamageableInterface* HitDamageable = Cast<IDamageableInterface>(HitActor))
			{
				HitDamageable->CustomTakeRadialDamage(Force, RadialDamageEvent, EventInstigator, DamageCauser);
			}
			if (UPrimitiveComponent* PrimComponent = Cast<UPrimitiveComponent>(HitActor->GetRootComponent()))
			{
				if (PrimComponent->IsSimulatingPhysics())
				{
					PrimComponent->AddImpulse((HitActor->GetActorLocation() - Location).GetSafeNormal() * Force);
				}
			}
		}
	}
}
