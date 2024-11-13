// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GenericTeamAgentInterface.h"
#include "GameFramework/PlayerController.h"
#include "PlayerControllerBase.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnPlayerDeath, ACharacterBase*, PlayerCharacter, APlayerControllerBase*, PlayerController);

class ACharacterBase;
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

	UFUNCTION()
	void OnControlledCharacterDied(ACharacterBase* DeadCharacter, AController* Inst, AActor* Causer);

	//----------------------------------------------------------------------//
	// IGenericTeamAgentInterface
	//----------------------------------------------------------------------//
	
public:
	
	virtual void SetGenericTeamId(const FGenericTeamId& NewTeamID) override;
	
	virtual FGenericTeamId GetGenericTeamId() const override { return TeamID; }

	virtual void SetPawn(APawn* InPawn) override;

public:
	UPROPERTY(EditAnywhere)
	TSubclassOf<class UUserWidget> PlayerHUDClass;

	UPROPERTY(BlueprintReadOnly)
	FTimerHandle PlayerRespawnTimerHandle;

	UPROPERTY()
	ACharacterBase* ControlledCharacter;

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	uint8 TeamNumber = 2;

	UPROPERTY(BlueprintAssignable)
	FOnPlayerDeath OnPlayerDeath;

private:
	UPROPERTY()
	FGenericTeamId TeamID;
};
