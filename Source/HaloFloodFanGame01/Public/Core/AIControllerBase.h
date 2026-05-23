// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "Perception/AIPerceptionComponent.h"
#include "AlertState.h"
#include "AIControllerBase.generated.h"

class AWeaponBase;
class UBehaviorTreeComponent;
class ASmartObject;
class ACharacterBase;
/**
 * 
 */

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnEnemyUpdated);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnAlertStateChanged, EAlertState, NewAlertState);

UCLASS()
class HALOFLOODFANGAME01_API AAIControllerBase : public AAIController
{
	GENERATED_BODY()
	AAIControllerBase();

public:
	virtual void BeginPlay() override;

	virtual void UpdateControlRotation(float DeltaTime, bool bUpdatePawn) override;

	virtual void OnPossess(APawn* InPawn) override;

	void BeginPlayDelayed();
	
	virtual ETeamAttitude::Type GetTeamAttitudeTowards(const AActor& Other) const override;

	UFUNCTION(BlueprintCallable)
	void SetSmartObject(ASmartObject* NewSmartObject);
	
	UFUNCTION(BlueprintNativeEvent)
	void OnPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus);

	UFUNCTION()
	void UpdatedPerception(AActor* Actor, FAIStimulus Stimulus, bool AlertedByAllies = false);

	UFUNCTION(BlueprintCallable)
	void UpdateTargetedEnemy(AActor* Actor);

	UFUNCTION(BlueprintCallable)
	void TargetKilled(ACharacterBase* KilledCharacter, AController* InstigatorController, AActor* Causer);

	UFUNCTION()
	void HearingStimulusUpdated(AActor* Actor, FAIStimulus Stimulus);

	UFUNCTION()
	void SightStimulusUpdated(AActor* Actor, FAIStimulus Stimulus);

	UFUNCTION()
	void DamageStimulusUpdated(AActor* Actor, FAIStimulus Stimulus);

	UFUNCTION()
	void AlertAllies(float AlertRadius, AActor* Actor, FAIStimulus Stimulus);

	void SetAlertState(TEnumAsByte<EAlertState> AlertState);

	void SetGenericTeamId(const FGenericTeamId& NewTeamID) override;
public:
	UPROPERTY(BlueprintAssignable)
	FOnEnemyUpdated OnEnemyUpdated;

	UPROPERTY(BlueprintAssignable)
	FOnAlertStateChanged OnAlertStateChanged;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	ACharacterBase* PawnChar;
	
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

	UPROPERTY(BlueprintReadOnly)
	FVector StimulusLocation;

	UPROPERTY()
	FTimerHandle Delay;

	UPROPERTY()
	TSet<AActor*> Targets;

	UPROPERTY()
	class UAISenseConfig_Sight* Sight;
	UPROPERTY()
	class UAISenseConfig_Hearing* Hearing;
	UPROPERTY()
	class UAISenseConfig_Damage* Damage;
	UPROPERTY()
	class UAISenseConfig_Team* Team;
	UPROPERTY()
	class UAISenseConfig_Touch* Touch;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (DeprecatedProperty))
	uint8 TeamNumber = 2;

	UPROPERTY(EditAnywhere)
	TArray<AActor*> KnownEnemies;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	ASmartObject* SmartObject;
};