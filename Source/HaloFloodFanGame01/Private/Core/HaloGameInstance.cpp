// Fill out your copyright notice in the Description page of Project Settings.


#include "Core/HaloGameInstance.h"

void UHaloGameInstance::LoadComplete(const float LoadTime, const FString& MapName)
{
	Super::LoadComplete(LoadTime, MapName);

	RemoveLoadScreen();
}
