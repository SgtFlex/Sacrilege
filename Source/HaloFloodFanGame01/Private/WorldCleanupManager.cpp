// Fill out your copyright notice in the Description page of Project Settings.


#include "WorldCleanupManager.h"

#include "Components/DecalComponent.h"

void UWorldCleanupManager::ManageRagdoll_Implementation(AActor* Actor)
{
	Ragdolls.Add(Actor);
	if (Ragdolls.Num() > MaxRagdolls)
	{
		if (Ragdolls[0])
			Ragdolls[0]->Destroy();
		Ragdolls.RemoveAt(0);
	}
}

void UWorldCleanupManager::ManageDecal_Implementation(UDecalComponent* Decal)
{
	Decals.Add(Decal);
	if (Decals.Num() > MaxDecals)
	{
		if (Decals[0])
			Decals[0]->DestroyComponent();
		Decals.RemoveAt(0);
	}
}

void UWorldCleanupManager::ManageActor_Implementation(AActor* Actor)
{
	
}

void UWorldCleanupManager::ManageWeapon_Implementation(AActor* Weapon)
{
	Weapons.Add(Weapon);
	if (Weapons.Num() > MaxWeapons)
	{
		if (Weapons[0])
			Weapons[0]->Destroy();
		Weapons.RemoveAt(0);
	}
}

void UWorldCleanupManager::StopManagingWeapon_Implementation(AActor* Weapon)
{
	if (Weapons.Contains(Weapon))
	{
		Weapons.Remove(Weapon);
	}
}