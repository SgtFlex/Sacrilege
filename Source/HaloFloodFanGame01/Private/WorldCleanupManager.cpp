// Fill out your copyright notice in the Description page of Project Settings.


#include "WorldCleanupManager.h"
#include "Components/DecalComponent.h"

UWorldCleanupManager::UWorldCleanupManager()
{
	AddCleanupCategory("Weapons", MaxWeapons);
	AddCleanupCategory("Characters", MaxCorpses);
	AddCleanupCategory("Decals", MaxDecals);
	AddCleanupCategory("Grenades", MaxGrenades);
}

void UWorldCleanupManager::ManageCorpse_Implementation(AActor* Actor)
{
	Ragdolls.Add(Actor);
	Actor->OnDestroyed.AddDynamic(this, &UWorldCleanupManager::StopManagingCorpse);
	if (Ragdolls.Num() > MaxCorpses)
	{
		if (IsValid(Ragdolls[0]))
		{
			UE_LOG(LogTemp, Warning, TEXT("Cleaning up corpse %s"), *Ragdolls[0]->GetName());
			Ragdolls[0]->OnDestroyed.RemoveDynamic(this, &UWorldCleanupManager::StopManagingCorpse);
			Ragdolls[0]->Destroy();
		}
		Ragdolls.RemoveAt(0);
	}
}

void UWorldCleanupManager::StopManagingCorpse_Implementation(AActor* Ragdoll)
{
	Ragdoll->OnDestroyed.RemoveDynamic(this, &UWorldCleanupManager::StopManagingCorpse);
	if (Ragdolls.Contains(Ragdoll))
	{
		Ragdolls.Remove(Ragdoll);
	}
}


void UWorldCleanupManager::ManageDecal_Implementation(UDecalComponent* Decal)
{
	Decals.Add(Decal);
	if (Decals.Num() > MaxDecals)
	{
		if (IsValid(Decals[0]))
		{
			UE_LOG(LogTemp, Warning, TEXT("Cleaning up decal %s"), *Decals[0]->GetName());
			Decals[0]->DestroyComponent();
		}
		Decals.RemoveAt(0);
	}
}

void UWorldCleanupManager::StopManagingDecal_Implementation(UDecalComponent* Decal)
{

}

void UWorldCleanupManager::AddCleanupCategory_Implementation(const FString& Category, uint8 ActorLimit)
{
	UE_LOG(LogTemp, Warning, TEXT("Created new cleanup category %s"), *Category);
	FManagedActorStruct ManagedActorStruct;
	ManagedActorStruct.Name = Category;
	ManagedActorStruct.ActorLimit = ActorLimit;
	CleanupStructs.Add(ManagedActorStruct);
}

void UWorldCleanupManager::ManageActor_Implementation(const FString& Category, AActor* Actor)
{
	for (FManagedActorStruct ActorCategoryStruct : CleanupStructs)
	{
		if (ActorCategoryStruct.Name.Equals(Category, ESearchCase::IgnoreCase))
		{
			ActorCategoryStruct.ManagedActors.Add(Actor);
			if (ActorCategoryStruct.ManagedActors.Num() > ActorCategoryStruct.ActorLimit)
			{
				UE_LOG(LogTemp, Warning, TEXT("Cleaning up %s from category %s"), *ActorCategoryStruct.ManagedActors[0]->GetName(), *Category);
				ActorCategoryStruct.ManagedActors[0]->Destroy();
				ActorCategoryStruct.ManagedActors.RemoveAt(0);
			}
		}
	}	
}

void UWorldCleanupManager::StopManagingActor_Implementation(const FString& Category, AActor* Actor)
{
	for (FManagedActorStruct ActorCategoryStruct : CleanupStructs)
	{
		if (ActorCategoryStruct.Name.Equals(Category))
		{
			if (ActorCategoryStruct.ManagedActors.Contains(Actor))
			{
				ActorCategoryStruct.ManagedActors.Remove(Actor);
			}
		}
	}
}


void UWorldCleanupManager::ManageWeapon_Implementation(AActor* Weapon)
{
	Weapons.Add(Weapon);
	Weapon->OnDestroyed.AddDynamic(this, &UWorldCleanupManager::StopManagingWeapon);
	if (Weapons.Num() > MaxWeapons)
	{
		if (IsValid(Weapons[0]))
		{
			UE_LOG(LogTemp, Warning, TEXT("Cleaning up weapon %s"), *Weapons[0]->GetName());
			Weapons[0]->OnDestroyed.RemoveDynamic(this, &UWorldCleanupManager::StopManagingWeapon);
			Weapons[0]->Destroy();
		}
		Weapons.RemoveAt(0);
	}
}

void UWorldCleanupManager::StopManagingWeapon_Implementation(AActor* Weapon)
{
	Weapon->OnDestroyed.RemoveDynamic(this, &UWorldCleanupManager::StopManagingWeapon);
	if (Weapons.Contains(Weapon))
	{
		Weapons.Remove(Weapon);
	}
}

void UWorldCleanupManager::ManageGrenade_Implementation(AActor* Grenade)
{
	Grenades.Add(Grenade);
	Grenade->OnDestroyed.AddDynamic(this, &UWorldCleanupManager::StopManagingGrenade);
	if (Grenades.Num() > MaxGrenades)
	{
		if (IsValid(Grenades[0]))
		{
			UE_LOG(LogTemp, Warning, TEXT("Cleaning up grenade %s"), *Grenades[0]->GetName());
			Grenades[0]->OnDestroyed.RemoveDynamic(this, &UWorldCleanupManager::StopManagingGrenade);
			Grenades[0]->Destroy();
		}
		Grenades.RemoveAt(0);
	}
}

void UWorldCleanupManager::StopManagingGrenade_Implementation(AActor* Grenade)
{
	Grenade->OnDestroyed.RemoveDynamic(this, &UWorldCleanupManager::StopManagingGrenade);
	if (Grenades.Contains(Grenade))
	{
		Grenades.Remove(Grenade);
	}
}
