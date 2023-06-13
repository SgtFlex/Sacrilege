// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "Perception/AIPerceptionComponent.h"
#include "AlertState.h"
#include "BaseAIController.generated.h"

class UBehaviorTreeComponent;
class ASmartObject;
/**
 * 
 */
UCLASS()
class HALOFLOODFANGAME01_API ABaseAIController : public AAIController
{
	GENERATED_BODY()
	ABaseAIController();

public:
	virtual void BeginPlay() override;

	void BeginPlayDelayed();
	
	virtual ETeamAttitude::Type GetTeamAttitudeTowards(const AActor& Other) const override;
	
	UFUNCTION(BlueprintNativeEvent)
	void OnPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus);

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TObjectPtr<UAIPerceptionComponent> AIPerceptionComponent;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	AActor* CurEnemy;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TEnumAsByte<EAlertState> AlertState = EAlertState::Relaxed;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector AimLocation;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UBehaviorTreeComponent* BehaviorTreeComp;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UBlackboardComponent* BlackboardComp;

	FTimerHandle Delay;
};
