// Fill out your copyright notice in the Description page of Project Settings.


#include "FunctionLibraries/MyCustomBlueprintFunctionLibrary.h"

#include "AudioDevice.h"
#include "Interfaces/DamageableInterface.h"
#include "NiagaraEmitterHandle.h"
#include "NiagaraFunctionLibrary.h"
#include "Interfaces/PhysicsInterface.h"
#include "AI/NavigationSystemBase.h"
#include "Camera/CameraComponent.h"
#include "Engine/DamageEvents.h"
#include "GeometryCollection/GeometryCollectionSimulationTypes.h"
#include "PlayerCharacter.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "Perception/AISense_Hearing.h"
#include "PhysicsEngine/PhysicsObjectBlueprintLibrary.h"

void UMyCustomBlueprintFunctionLibrary::Ignite(UPrimitiveComponent* Component, float DamagePerSecond, float Duration)
{
	
}

void UMyCustomBlueprintFunctionLibrary::FireHitScanBullet(FHitResult& Hit, const TArray<AActor*>& ActorsToIgnore, FVector StartLocation, FVector Direction, float Range, UCurveFloat* FalloffCurve, float Damage, float Force, AActor* DamageCauser, AController* EventInstigator, float MagnetizeRadius)
{
	UWorld* World = GEngine->GameViewport->GetWorld();
	
	GetHitMagnetized(Hit, ActorsToIgnore, StartLocation, Direction, Range, MagnetizeRadius);

	if (Hit.bBlockingHit && Hit.GetActor())
	{
		FVector HitDir = (Hit.Location - StartLocation).GetSafeNormal();
		UAISense_Hearing::ReportNoiseEvent(World, Hit.Location, 1.0f, EventInstigator);
		if (Hit.GetActor()->Implements<UDamageableInterface>())
		{
			Damage = FalloffCurve!=nullptr ? Damage * FalloffCurve->GetFloatValue(Hit.Distance/Range) : Damage;
			FPointDamageEvent PointDamageEvent = FPointDamageEvent(Damage, Hit, HitDir, UDamageType::StaticClass());
			IDamageableInterface::Execute_CustomTakePointDamage(Hit.GetActor(), Damage, HitDir, Hit, Force, EventInstigator, DamageCauser);
		}
		if (Hit.GetComponent() && Hit.GetComponent()->IsSimulatingPhysics())
		{
			Hit.GetComponent()->AddImpulseAtLocation(HitDir*Force, Hit.ImpactPoint, Hit.BoneName);
		}
	}
	
}

void UMyCustomBlueprintFunctionLibrary::GetHitMagnetized(FHitResult& Hit, const TArray<AActor*>& ActorsToIgnore,
	const FVector StartLocation, const FVector Direction, const float Range, const float MagnetizeRadius)
{
	UWorld* World = GEngine->GameViewport->GetWorld();
	
	FHitResult HitThin;
	UKismetSystemLibrary::LineTraceSingle(World, StartLocation, StartLocation + (Direction*Range), TraceTypeQuery1, true, ActorsToIgnore, EDrawDebugTrace::None, HitThin, true);
	if (HitThin.bBlockingHit && HitThin.GetActor()->Implements<UDamageableInterface>())
	{
		Hit = HitThin;
	} else
	{
		FHitResult HitMagnetized;
		UKismetSystemLibrary::SphereTraceSingle(World, StartLocation, StartLocation + (Direction * Range), MagnetizeRadius, TraceTypeQuery1, true, ActorsToIgnore, EDrawDebugTrace::None, HitMagnetized, true);
		if (HitMagnetized.bBlockingHit && HitMagnetized.GetActor()->Implements<UDamageableInterface>())
		{
			Hit = HitMagnetized;
		} else
		{
			Hit = HitThin;
		}
	}
}

AActor* UMyCustomBlueprintFunctionLibrary::FireProjectile(FVector StartLocation,
                                                          FVector Direction, TSubclassOf<AActor> ActorToSpawn)
{
	UWorld* World = GEngine->GameViewport->GetWorld();

	AActor* Actor = nullptr;
	

	FRotator Rotation = Direction.Rotation();
	Actor = World->SpawnActor(ActorToSpawn, &StartLocation, &Rotation);
	
	
	return Actor;
}


void UMyCustomBlueprintFunctionLibrary::FireExplosion(TArray<AActor*> ActorsToIgnore, FVector Location, float BaseDamage, float MinimumDamage, float OuterRadius, float InnerRadius, float DamageFalloff, float Force, AActor* DamageCauser, AController* EventInstigator)
{
	UWorld* World = GEngine->GameViewport->GetWorld();

	TArray<AActor*> Actors;


	FRadialDamageEvent RadialDamageEvent;
	RadialDamageEvent.Params = FRadialDamageParams(BaseDamage, MinimumDamage, InnerRadius, OuterRadius, DamageFalloff);
	RadialDamageEvent.Origin = Location;
	TArray<TEnumAsByte<EObjectTypeQuery>> Objects;
	TArray<AActor*> HitActors;

	TArray<FHitResult> OutHits;
	UKismetSystemLibrary::SphereTraceMulti(World, Location, Location, OuterRadius, TraceTypeQuery1, false, ActorsToIgnore, EDrawDebugTrace::None, OutHits, true);
// #if !(UE_BUILD_SHIPPING || UE_BUILD_TEST)
// 	DrawDebugSphere(World, Location, OuterRadius, 8, FColor::Green, false, 3);
// 	DrawDebugSphere(World, Location, InnerRadius, 8, FColor::Red, false, 3);
// #endif
	//UKismetSystemLibrary::SphereTraceMultiByProfile(World, Location, Location, OuterRadius, FName("Projectile"), false, ActorsToIgnore, EDrawDebugTrace::None, OutHits, true);
	//UKismetSystemLibrary::SphereTraceMulti(World, Location, Location, OuterRadius, TraceTypeQuery1, false, ActorsToIgnore, EDrawDebugTrace::ForDuration, OutHits, true);
	//UKismetSystemLibrary::SphereOverlapActors(World, Location, OuterRadius, Objects, AActor::StaticClass(), ActorsToIgnore, HitActors);
	ActorsToIgnore.Add(DamageCauser);
	for (FHitResult Hit : OutHits)
	{
		if (AActor* HitActor = Hit.GetActor())
		{
			if (!ActorsToIgnore.Contains(HitActor))
			{
				
				//TODO - Maybe use a "ForceInterface" for characters, projectiles, and others instead of casting?
				FHitResult LOSCheck;
				UKismetSystemLibrary::LineTraceSingle(World, Location, Hit.ImpactPoint, TraceTypeQuery1, false, ActorsToIgnore, EDrawDebugTrace::None, LOSCheck, true);
				if (LOSCheck.GetActor() == HitActor)
				{
					//We only want to hit once per object
					ActorsToIgnore.AddUnique(Hit.GetActor());
					//GEngine->AddOnScreenDebugMessage(-1, 1.0f, FColor::Cyan, FString::Printf(TEXT("%s"), *HitActor->GetActorLabel()));
					if (HitActor->Implements<UDamageableInterface>())
					{
						// (HitDist - MinRange)/(MaxRange - MinRange)
						// Lerp result between Min damage and max damage
						// FMath::Lerp(MinimumDamage, MaximumDamage, (MaxRange - HitDist)/(MaxRange - MinRange))
						//IDamageableInterface::Execute_CustomTakeRadialDamage(HitActor, Force, RadialDamageEvent, EventInstigator, DamageCauser);
						IDamageableInterface::Execute_CustomTakeRadialDamage(HitActor, Location, OuterRadius, Force, Hit,
																			 RadialDamageEvent, InnerRadius, EventInstigator,
																			 DamageCauser);
					} else if (HitActor->Implements<UPhysicsInterface>())
					{
						IPhysicsInterface::Execute_ApplyPhysicsImpulse(HitActor, (Hit.ImpactPoint - Location).GetSafeNormal() * Force, Hit.ImpactPoint, NAME_None);
					
					} else if (UPrimitiveComponent* PrimComponent = Cast<UPrimitiveComponent>(HitActor->GetRootComponent()))
					{
						if (PrimComponent->IsSimulatingPhysics() && PrimComponent->GetCollisionEnabled() ==
							ECollisionEnabled::QueryAndPhysics)
						{
							PrimComponent->AddImpulseAtLocation(((Hit.ImpactPoint - Location).GetSafeNormal() * Force), Hit.ImpactPoint);
							//PrimComponent->AddImpulse((HitActor->GetActorLocation() - Location).GetSafeNormal() * FMath::Lerp(0, Force, (FVector::Distance(HitActor->GetActorLocation(), Location)) + InnerRadius));
						}
					}
				}
				else
				{
					//DrawDebugLine(World, Location, Hit.ImpactPoint, FColor::Red, false, 2);
				}
				
			}
		}
	}
}

void UMyCustomBlueprintFunctionLibrary::FireExplosionWithCosmetics(
	TArray<AActor*>& ActorsToIgnore, FVector Location, float BaseDamage, float MinimumDamage, float OuterRadius,
	float InnerRadius, float DamageFalloff, float Force, AActor* DamageCauser, AController* EventInstigator, UNiagaraSystem* Particles, USoundBase* Sound, TSubclassOf<UCameraShakeBase> CameraShake, UForceFeedbackEffect* ForceFeedbackEffect)
{
	UWorld* World = GEngine->GameViewport->GetWorld();
	FireExplosion(ActorsToIgnore, Location, BaseDamage, MinimumDamage, OuterRadius, InnerRadius, DamageFalloff, Force, DamageCauser, EventInstigator);
	if (Particles) UNiagaraFunctionLibrary::SpawnSystemAtLocation(World, Particles, Location);
	if (Sound) UGameplayStatics::PlaySoundAtLocation(World, Sound, Location);
	if (CameraShake) UGameplayStatics::PlayWorldCameraShake(World, CameraShake, Location, InnerRadius, OuterRadius * 10);
	if (ForceFeedbackEffect) UGameplayStatics::SpawnForceFeedbackAtLocation(World, ForceFeedbackEffect, Location);
}

float UMyCustomBlueprintFunctionLibrary::SetGlobalGravity(AWorldSettings* WorldSettings, float GlobalGravity)
{
	WorldSettings->bGlobalGravitySet = true;
	WorldSettings->GlobalGravityZ = GlobalGravity;

	return WorldSettings->GlobalGravityZ;
}

TArray<USoundMix*> UMyCustomBlueprintFunctionLibrary::GetCurrentSoundMixModifiers()
{
	UWorld* World = GEngine->GameViewport->GetWorld();
	FAudioDevice* AudioDevice = World->GetAudioDevice().GetAudioDevice();
	TMap<USoundMix*, FSoundMixState> SoundMixes = AudioDevice->GetSoundMixModifiers();
	TArray<USoundMix*> SoundMixModifiers;
	SoundMixes.GenerateKeyArray(SoundMixModifiers);

	return SoundMixModifiers;
}
