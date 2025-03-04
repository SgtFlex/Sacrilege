// Fill out your copyright notice in the Description page of Project Settings.


#include "HaloWorldSubsystem.h"

FNeedleStruct UHaloWorldSubsystem::AddNeedleToActor(AActor* Actor, AProjectileBase* Needle)
{
	for (FNeedleStruct& Struct : NeedledActorArray)
	{
		if (Struct.Actor == Actor)
		{
			Struct.Actor = Actor;
			Struct.Projectiles.Add(Needle);
			
			UE_LOG(LogTemp, Warning, TEXT("Found Struct for %s, added %s, length %d"), *Actor->GetActorLabel(), *Needle->GetActorLabel(), Struct.Projectiles.Num());
			return Struct;
		}
	}
	FNeedleStruct Struct;
	Struct.Actor = Actor;
	Struct.Projectiles.Add(Needle);
	NeedledActorArray.Add(Struct);
	UE_LOG(LogTemp, Warning, TEXT("Created Struct"));
	return Struct;
}

void UHaloWorldSubsystem::GetEmbeddedNeedles(AActor* Actor, TArray<AProjectileBase*>& Needles)
{
	for (FNeedleStruct& Struct : NeedledActorArray)
	{
		if (Struct.Actor == Actor)
		{
			Needles = Struct.Projectiles;
		}
	}
}

void UHaloWorldSubsystem::RemoveNeedleFromActor(AActor* Actor, AProjectileBase* Needle)
{
	for (FNeedleStruct Struct : NeedledActorArray)
	{
		if (Struct.Actor == Actor)
		{
			Struct.Projectiles.Remove(Needle);
			if (Struct.Projectiles.Num() <= 0)
			{
				NeedledActorArray.Remove(Struct);
			}
			return;
		}
	}
}

void UHaloWorldSubsystem::RemoveAllNeedlesFromActor(AActor* Actor)
{
	FNeedleStruct FoundStruct;
	for (FNeedleStruct Struct : NeedledActorArray)
	{
		if (Struct.Actor == Actor)
		{
			NeedledActorArray.Remove(Struct);
			return;
			// for (auto Needle : Struct.Projectiles)
			// {
			// 	RemoveNeedleFromActor(Struct.Actor, Needle);
			// }
		}
	}
}


