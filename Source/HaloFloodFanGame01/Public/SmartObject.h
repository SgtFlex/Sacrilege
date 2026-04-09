// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "AlertState.h"
#include "SmartObject.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnCommandStarted);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnCommandComplete);

class UBehaviorTree;
UCLASS(Abstract)
class HALOFLOODFANGAME01_API ASmartObject : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ASmartObject();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UBehaviorTree* DynamicTree;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TEnumAsByte<EAlertState> AlertState;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	ASmartObject* NextSmartObject;

	UPROPERTY(BlueprintAssignable, BlueprintCallable)
	FOnCommandStarted OnCommandStarted;

	UPROPERTY(BlueprintAssignable, BlueprintCallable)
	FOnCommandComplete OnCommandComplete;
};
