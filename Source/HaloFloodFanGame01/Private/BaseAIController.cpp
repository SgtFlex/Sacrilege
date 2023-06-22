// Fill out your copyright notice in the Description page of Project Settings.


#include "BaseAIController.h"

#include "SmartObject.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Core/BaseCharacter.h"
#include "Core/BasePawn.h"

ABaseAIController::ABaseAIController()
{
	AIPerceptionComponent = CreateDefaultSubobject<UAIPerceptionComponent>(TEXT("PerceptionComp"));
	BlackboardComp = CreateDefaultSubobject<UBlackboardComponent>(TEXT("BlackboardComp"));
	BehaviorTreeComp = CreateDefaultSubobject<UBehaviorTreeComponent>(TEXT("Behavior Tree"));
}

void ABaseAIController::BeginPlay()
{
	Super::BeginPlay();
	SetGenericTeamId(FGenericTeamId(2));
	AIPerceptionComponent->OnTargetPerceptionUpdated.AddDynamic(this, &ABaseAIController::OnPerceptionUpdated);
	GetWorldTimerManager().SetTimer(Delay, this, &ABaseAIController::BeginPlayDelayed, 0.1f, false);
	BehaviorTreeComp->StartLogic();
}

void ABaseAIController::BeginPlayDelayed()
{
	ABaseCharacter* Char = Cast<ABaseCharacter>(GetPawn());
	if (Char)
	{
		if (ASmartObject* SmartObj = Char->SmartObject) {
			FGameplayTag SubTag;
			BehaviorTreeComp->SetDynamicSubtree(SubTag, SmartObj->DynamicTree);
			BlackboardComp->SetValueAsBool(FName("HasSmartObject"), true);
			AlertState = SmartObj->AlertState;
		}
	}
}

ETeamAttitude::Type ABaseAIController::GetTeamAttitudeTowards(const AActor& Other) const
{
	if (const APawn* OtherPawn = Cast<APawn>(&Other)) {

		if (const IGenericTeamAgentInterface* TeamAgent = Cast<IGenericTeamAgentInterface>(OtherPawn->GetController()))
		{
			if (TeamAgent->GetGenericTeamId()==0)
				return ETeamAttitude::Neutral;
			return Super::GetTeamAttitudeTowards(*OtherPawn->GetController());
		}
	}
	return ETeamAttitude::Neutral;
	//return Super::GetTeamAttitudeTowards(Other);
}



void ABaseAIController::OnPerceptionUpdated_Implementation(AActor* Actor, FAIStimulus Stimulus)
{
	if (GetTeamAttitudeTowards(*Actor)!=ETeamAttitude::Hostile)
		return;
	CurEnemy = Actor;
	AlertState = Alerted;
}
