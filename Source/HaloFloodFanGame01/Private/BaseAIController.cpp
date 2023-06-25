// Fill out your copyright notice in the Description page of Project Settings.


#include "BaseAIController.h"

#include "NavigationSystem.h"
#include "SmartObject.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Core/BaseCharacter.h"
#include "Core/BasePawn.h"
#include "Perception/AISenseConfig.h"
#include "Perception/AISenseConfig_Damage.h"
#include "Perception/AISenseConfig_Hearing.h"
#include "Perception/AISenseConfig_Sight.h"
#include "Perception/AISenseConfig_Team.h"

ABaseAIController::ABaseAIController()
{
	AIPerceptionComponent = CreateDefaultSubobject<UAIPerceptionComponent>(TEXT("PerceptionComp"));
	BlackboardComp = CreateDefaultSubobject<UBlackboardComponent>(TEXT("BlackboardComp"));
	BehaviorTreeComp = CreateDefaultSubobject<UBehaviorTreeComponent>(TEXT("Behavior Tree"));
	Sight = CreateDefaultSubobject<UAISenseConfig_Sight>(TEXT("Sight Config"));
	Hearing = CreateDefaultSubobject<UAISenseConfig_Hearing>(TEXT("Hearing Config"));
	Damage = CreateDefaultSubobject<UAISenseConfig_Damage>(TEXT("Damage Config"));
	Team = CreateDefaultSubobject<UAISenseConfig_Team>(TEXT("Team Config"));

	AIPerceptionComponent->ConfigureSense(*Sight);
	AIPerceptionComponent->ConfigureSense(*Hearing);
	AIPerceptionComponent->ConfigureSense(*Damage);
	AIPerceptionComponent->ConfigureSense(*Team);
	AIPerceptionComponent->SetDominantSense(*Sight->GetSenseImplementation());
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
			BlackboardComp->SetValueAsEnum(TEXT("AlertState"), SmartObj->AlertState);
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

void ABaseAIController::HearingStimulusUpdated(AActor* Actor, FAIStimulus Stimulus)
{
}

void ABaseAIController::SightStimulusUpdated(AActor* Actor, FAIStimulus Stimulus)
{
}

void ABaseAIController::DamageStimulusUpdated(AActor* Actor, FAIStimulus Stimulus)
{
}


void ABaseAIController::OnPerceptionUpdated_Implementation(AActor* Actor, FAIStimulus Stimulus)
{
	if (GetTeamAttitudeTowards(*Actor)!=ETeamAttitude::Hostile)
		return;

	TArray<AActor*> PerceivedActors;
	AIPerceptionComponent->GetCurrentlyPerceivedActors(Sight->GetSenseImplementation(), PerceivedActors);
	if (PerceivedActors.Contains(Actor))
	{
		BlackboardComp->SetValueAsObject(TEXT("Enemy"), Actor);
		BlackboardComp->SetValueAsEnum(TEXT("AlertState"), EAlertState::Alerted);

	} else
	{
		FVector Direction = Actor->GetVelocity();
		//Direction.Z = 0;
		Direction.Normalize();
		FVector PredictedLocation = Stimulus.StimulusLocation + Direction*300;
		BlackboardComp->SetValueAsObject(TEXT("Enemy"), nullptr);
		BlackboardComp->SetValueAsEnum(TEXT("AlertState"), EAlertState::Suspicious);
		BlackboardComp->SetValueAsVector(TEXT("StimulusLocation"), Stimulus.StimulusLocation);
	}
	//BlackboardComp->SetValueAsVector(TEXT("StimulusLocation"), StimulusLocation);
	//FNavLocation NavPoint;
	//UNavigationSystemV1::ProjectPointToNavigation(StimulusLocation, NavPoint, FVector(30,30,30), Cast<ANavigationData>(nullptr));
	//BlackboardComp->SetValueAsVector(TEXT("StimulusLocation"), StimulusLocation + Dir);
}
