// Fill out your copyright notice in the Description page of Project Settings.


#include "HaloGameState.h"

void AHaloGameState::ManageRagdoll_Implementation(AActor* Actor)
{
	Ragdolls.Add(Actor);
	if (Ragdolls.Num() > MaxRagdolls)
	{
		Ragdolls[0]->Destroy();
		Ragdolls.RemoveAt(0);
	}
}

void AHaloGameState::ManageDecal_Implementation(AActor* Actor)
{
	Decals.Add(Actor);
	if (Decals.Num() > MaxDecals)
	{
		Decals[0]->Destroy();
		Decals.RemoveAt(0);
	}
}

void AHaloGameState::ManageActor_Implementation(AActor* Actor)
{
	
}
