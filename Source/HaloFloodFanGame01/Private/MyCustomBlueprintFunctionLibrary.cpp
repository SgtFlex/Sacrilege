// Fill out your copyright notice in the Description page of Project Settings.


#include "MyCustomBlueprintFunctionLibrary.h"

#include "AudioDevice.h"
#include "DamageableInterface.h"
#include "NiagaraEmitterHandle.h"
#include "NiagaraFunctionLibrary.h"
#include "AI/NavigationSystemBase.h"
#include "Camera/CameraComponent.h"
#include "Engine/DamageEvents.h"
#include "GeometryCollection/GeometryCollectionSimulationTypes.h"
#include "HaloFloodFanGame01/PlayerCharacter.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "Perception/AISense_Hearing.h"
#include "PhysicsEngine/PhysicsObjectBlueprintLibrary.h"

void UMyCustomBlueprintFunctionLibrary::Ignite(UPrimitiveComponent* Component, float DamagePerSecond, float Duration)
{
	
}

void UMyCustomBlueprintFunctionLibrary::FireHitScanBullet(FHitResult& Hit, TArray<AActor*>& ActorsToIgnore, FVector StartLocation, FVector Direction, float Range, UCurveFloat* FalloffCurve, float Damage, float Force, AActor* DamageCauser, AController* EventInstigator, float MagnetizeRadius)
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
			Hit.GetComponent()->AddImpulse(HitDir*Force);
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


void UMyCustomBlueprintFunctionLibrary::FireExplosion(TArray<AActor*>& ActorsToIgnore, FVector Location, float BaseDamage, float MinimumDamage, float OuterRadius, float InnerRadius, float DamageFalloff, float Force, AActor* DamageCauser, AController* EventInstigator)
{
	UWorld* World = GEngine->GameViewport->GetWorld();

	TArray<AActor*> Actors;


	FRadialDamageEvent RadialDamageEvent;
	RadialDamageEvent.Params = FRadialDamageParams(BaseDamage, MinimumDamage, InnerRadius, OuterRadius, DamageFalloff);
	RadialDamageEvent.Origin = Location;
	TArray<TEnumAsByte<EObjectTypeQuery>> Objects;
	TArray<AActor*> HitActors;
	UKismetSystemLibrary::SphereOverlapActors(World, Location, OuterRadius, Objects, AActor::StaticClass(), ActorsToIgnore, HitActors);
	for (auto HitActor : HitActors)
	{
		if (!ActorsToIgnore.Contains(HitActor))
		{
			if (HitActor->Implements<UDamageableInterface>())
			{
				IDamageableInterface::Execute_CustomTakeRadialDamage(HitActor, Force, RadialDamageEvent, EventInstigator, DamageCauser);
				//HitDamageable->CustomTakeRadialDamage(Force, RadialDamageEvent, EventInstigator, DamageCauser);
			}
			if (UPrimitiveComponent* PrimComponent = Cast<UPrimitiveComponent>(HitActor->GetRootComponent()))
			{
				if (PrimComponent->IsSimulatingPhysics() && PrimComponent->GetCollisionEnabled() == ECollisionEnabled::QueryAndPhysics)
				{
					PrimComponent->AddImpulse((HitActor->GetActorLocation() - Location).GetSafeNormal() * Force);
					//PrimComponent->AddImpulse((HitActor->GetActorLocation() - Location).GetSafeNormal() * FMath::Lerp(0, Force, (FVector::Distance(HitActor->GetActorLocation(), Location)) + InnerRadius));

				}
			}
			//TODO - Maybe use a "ForceInterface" for characters, projectiles, and others instead of casting?
			if (ACharacterBase* Character = Cast<ACharacterBase>(HitActor))
			{
				Character->LaunchCharacter((Character->GetActorLocation() - Location).GetSafeNormal() * Force * 0.01, false, false);
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
