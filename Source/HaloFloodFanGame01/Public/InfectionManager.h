// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Core/CharacterBase.h"
#include "InfectionManager.generated.h"

/**
 * 
 */
USTRUCT()
struct FPods
{
	GENERATED_BODY()

	UPROPERTY()
	TArray<ACharacterBase*> Pods;
};

UCLASS()
class HALOFLOODFANGAME01_API UInfectionManager : public UWorldSubsystem
{

	GENERATED_BODY()
	
public:
	// UInfectionManager();
	// ~UInfectionManager();
//
// 	UFUNCTION(BlueprintCallable)
// 	ACharacterBase* CreateInfected(ACharacterBase* Host);
//
// 	void OnInfectedCharacterKilled( ACharacterBase* Host, AController* Controller, AActor* Causer);
//
// 	UFUNCTION(BlueprintImplementableEvent)
// 	void UnlatchInfectionForms(ACharacterBase* Host, TArray<ACharacterBase*> InfForms);
//
// 	UFUNCTION(BlueprintCallable)
// 	void AddLatchedInfectionForm(ACharacterBase* Host, ACharacterBase* InfForm);
//
// 	UFUNCTION(BlueprintCallable)
// 	void RemoveLatchedInfectionForm(ACharacterBase* Host, ACharacterBase* InfForm);
// public:
// 	//UPROPERTY()
// 	//TMap<AActor*, uint8> ManagedActors;
//
// 	UPROPERTY()
// 	TMap<ACharacterBase*, FPods> CharsBeingInfected;
};
