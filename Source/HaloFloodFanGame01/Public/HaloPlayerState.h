// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "HaloPlayerState.generated.h"

/**
 * 
 */

class AGunBase;
class ACharacterBase;

UCLASS()
class HALOFLOODFANGAME01_API AHaloPlayerState : public APlayerState
{
	GENERATED_BODY()
	
public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	uint8 Team;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	TSubclassOf<ACharacterBase> CharacterClass;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	TSubclassOf<AGunBase> PrimaryWeaponClass;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	TSubclassOf<AGunBase> SecondaryWeaponClass;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	float SensitivityMultiplier = 1;	
};
