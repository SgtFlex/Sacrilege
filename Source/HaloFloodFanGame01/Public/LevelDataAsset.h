// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "LevelDataAsset.generated.h"

/**
 * 
 */
UCLASS(BlueprintType)
class HALOFLOODFANGAME01_API ULevelDataAsset : public UDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FString LevelPath;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FText LevelName;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	UTexture2D* LevelThumbnail;
};
