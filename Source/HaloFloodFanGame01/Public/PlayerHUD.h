// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/TextBlock.h"
#include "PlayerHUD.generated.h"


class UUniformGridPanel;
class UListView;
class UTextBlock;
class APlayerCharacter;
/**
 * 
 */
UCLASS()
class HALOFLOODFANGAME01_API UPlayerHUD : public UUserWidget
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite,Category="Colors")
	FLinearColor EnemyColor = FColor(255, 25, 25, 255);

	UPROPERTY(EditAnywhere, BlueprintReadWrite,Category="Colors")
	FLinearColor AllyColor = FColor(25, 255, 25, 255);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Colors")
	FLinearColor HUDColor = FColor(255, 150, 50, 255);

protected:
	UPROPERTY()
	class ACharacterBase* PlayerCharacter;
};