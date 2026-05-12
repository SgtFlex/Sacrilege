// Fill out your copyright notice in the Description page of Project Settings.


#include "Subsystems/HaloWorldSubsystem.h"

FNeedleStruct UHaloWorldSubsystem::AddNeedleToActor(AActor* Actor, AProjectileBase* Needle)
{
	for (FNeedleStruct& Struct : NeedledActorArray)
	{
		if (Struct.Actor == Actor)
		{
			Struct.Actor = Actor;
			Struct.Projectiles.Add(Needle);
			
			return Struct;
		}
	}
	FNeedleStruct Struct;
	Struct.Actor = Actor;
	Struct.Projectiles.Add(Needle);
	NeedledActorArray.Add(Struct);
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
	for (int32 i = 0; i < NeedledActorArray.Num(); i++)
	{
		if (NeedledActorArray[i].Actor == Actor)
		{
			NeedledActorArray[i].Projectiles.Remove(Needle);
			if (NeedledActorArray[i].Projectiles.Num() <= 0)
			{
				NeedledActorArray.RemoveAt(i);
			}
			return;
		}
	}
	// for (FNeedleStruct& Struct : NeedledActorArray)
	// {
	// 	if (Struct.Actor == Actor)
	// 	{
	// 		Struct.Projectiles.Remove(Needle);
	// 		if (Struct.Projectiles.Num() <= 0)
	// 		{
	// 			NeedledActorArray.Remove(Struct);
	// 		}
	// 		return;
	// 	}
	// }
}

void UHaloWorldSubsystem::RemoveAllNeedlesFromActor(AActor* Actor)
{
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


