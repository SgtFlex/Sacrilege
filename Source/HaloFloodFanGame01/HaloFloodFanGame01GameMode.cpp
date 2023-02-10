// Copyright Epic Games, Inc. All Rights Reserved.

#include "HaloFloodFanGame01GameMode.h"
#include "HaloFloodFanGame01Character.h"
#include "UObject/ConstructorHelpers.h"

AHaloFloodFanGame01GameMode::AHaloFloodFanGame01GameMode()
	: Super()
{
	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnClassFinder(TEXT("/Game/FirstPerson/Blueprints/BP_FirstPersonCharacter"));
	DefaultPawnClass = PlayerPawnClassFinder.Class;

}
