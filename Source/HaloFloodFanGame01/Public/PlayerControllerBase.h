// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GenericTeamAgentInterface.h"
#include "GameFramework/PlayerController.h"
#include "PlayerControllerBase.generated.h"

class UPlayerHUD;
/**
 * 
 */
UCLASS()
class HALOFLOODFANGAME01_API APlayerControllerBase : public APlayerController, public IGenericTeamAgentInterface
{
	GENERATED_BODY()
public:
	APlayerControllerBase();

	virtual void BeginPlay() override;

	virtual void OnPossess(APawn* InPawn) override;

private:
	FGenericTeamId TeamId;
	FGenericTeamId GetGenericTeamId() const;

public:
	UPROPERTY(EditAnywhere)
	TSubclassOf<class UUserWidget> PlayerHUDClass;

	UPROPERTY(EditAnywhere)
	int TeamNumber = 1;
};
