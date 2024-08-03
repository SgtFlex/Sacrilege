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

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnScoreUpdated, AHaloPlayerState*, PlayerState, int, NewScore);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnResourceUpdated, AHaloPlayerState*, PlayerState, int, NewResource);

UCLASS()
class HALOFLOODFANGAME01_API AHaloPlayerState : public APlayerState
{
	GENERATED_BODY()

public:
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	
	UFUNCTION(BlueprintCallable)
	void AddPlayerScore(int AddScore);

	UFUNCTION(BlueprintCallable)
	void SubtractPlayerScore(int SubtractScore);

	UFUNCTION(BlueprintSetter, Server, Reliable)
	void SetPlayerScore(int NewScore);

	UFUNCTION(BlueprintGetter)
	int GetPlayerScore();

	UFUNCTION(BlueprintCallable)
	void AddPlayerResource(int AddResource);

	UFUNCTION(BlueprintCallable)
	void SubtractPlayerResource(int SubtractResource);

	UFUNCTION(BlueprintSetter, Server, Reliable)
	void SetPlayerResource(int NewResource);

	UFUNCTION(BlueprintGetter)
	int GetPlayerResource();

	UFUNCTION(BlueprintCallable, BlueprintPure)
	bool HasPlayerResource(int Amount);

	UFUNCTION(BlueprintCallable)
	void SetPlayerID(const FString& NewName);
	
public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	uint8 Team;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	TSubclassOf<ACharacterBase> CharacterClass;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	TSubclassOf<AGunBase> PrimaryWeaponClass;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	TSubclassOf<AGunBase> SecondaryWeaponClass;

	UPROPERTY(BlueprintReadOnly, VisibleDefaultsOnly, Replicated)
	int PlayerResource = 0;

	UPROPERTY(BlueprintReadOnly, VisibleDefaultsOnly, Replicated)
	int PlayerScore = 0;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	float SensitivityMultiplier = 1;

	//Delegates

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, BlueprintAssignable)
	FOnScoreUpdated OnScoreUpdated;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, BlueprintAssignable)
	FOnResourceUpdated OnResourceUpdated;
};
