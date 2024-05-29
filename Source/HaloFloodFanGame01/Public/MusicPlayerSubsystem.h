// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "HaloGameInstance.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "MusicPlayerSubsystem.generated.h"

/**
 * 
 */
UCLASS()
class HALOFLOODFANGAME01_API UMusicPlayerSubsystem : public UHaloGameInstance
{
	GENERATED_BODY()
	
public:
	UPROPERTY(EditDefaultsOnly)
	bool bEnableMusic = true;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	TArray<USoundBase*> Soundtracks;
};
