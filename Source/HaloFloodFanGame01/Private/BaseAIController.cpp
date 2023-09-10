// Fill out your copyright notice in the Description page of Project Settings.


#include "BaseAIController.h"

#include "NavigationSystem.h"
#include "SmartObject.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Core/BaseCharacter.h"
#include "Core/BasePawn.h"
#include "Kismet/GameplayStatics.h"
#include "Perception/AISenseConfig.h"
#include "Perception/AISenseConfig_Damage.h"
#include "Perception/AISenseConfig_Hearing.h"
#include "Perception/AISenseConfig_Sight.h"
#include "Perception/AISenseConfig_Team.h"
#include "Perception/AISenseConfig_Touch.h"

ABaseAIController::ABaseAIController()
{
	AIPerceptionComponent = CreateDefaultSubobject<UAIPerceptionComponent>(TEXT("PerceptionComp"));
	BlackboardComp = CreateDefaultSubobject<UBlackboardComponent>(TEXT("BlackboardComp"));
	BehaviorTreeComp = CreateDefaultSubobject<UBehaviorTreeComponent>(TEXT("Behavior Tree"));
	Sight = CreateDefaultSubobject<UAISenseConfig_Sight>(TEXT("Sight Config"));
	Hearing = CreateDefaultSubobject<UAISenseConfig_Hearing>(TEXT("Hearing Config"));
	Damage = CreateDefaultSubobject<UAISenseConfig_Damage>(TEXT("Damage Config"));
	Team = CreateDefaultSubobject<UAISenseConfig_Team>(TEXT("Team Config"));
	Touch = CreateDefaultSubobject<UAISenseConfig_Touch>(TEXT("Touch Config"));

	AIPerceptionComponent->ConfigureSense(*Sight);
	AIPerceptionComponent->ConfigureSense(*Hearing);
	AIPerceptionComponent->ConfigureSense(*Damage);
	AIPerceptionComponent->ConfigureSense(*Team);
	AIPerceptionComponent->ConfigureSense(*Touch);
	AIPerceptionComponent->SetDominantSense(*Sight->GetSenseImplementation());
}

void ABaseAIController::BeginPlay()
{
	Super::BeginPlay();
	AIPerceptionComponent->OnTargetPerceptionUpdated.AddDynamic(this, &ABaseAIController::OnPerceptionUpdated);
	GetWorldTimerManager().SetTimer(Delay, this, &ABaseAIController::BeginPlayDelayed, 0.1f, false);
	BehaviorTreeComp->StartLogic();
}

void ABaseAIController::UpdateControlRotation(float DeltaTime, bool bUpdatePawn)
{
	//Super::UpdateControlRotation(DeltaTime, bUpdatePawn);

	APawn* const MyPawn = GetPawn();
	if (MyPawn)
	{
		FRotator NewControlRotation = GetControlRotation();

		// Look toward focus
		const FVector FocalPoint = GetFocalPoint();
		if (FAISystem::IsValidLocation(FocalPoint))
		{
			NewControlRotation = (FocalPoint - MyPawn->GetPawnViewLocation()).Rotation();
		}
		else if (bSetControlRotationFromPawnOrientation)
		{
			NewControlRotation = MyPawn->GetActorRotation();
		}

		// Don't pitch view unless looking at another pawn
		// if (NewControlRotation.Pitch != 0 && Cast<APawn>(GetFocusActor()) == nullptr)
		// {
		// 	NewControlRotation.Pitch = 0.f;
		// }

		SetControlRotation(NewControlRotation);

		if (bUpdatePawn)
		{
			const FRotator CurrentPawnRotation = MyPawn->GetActorRotation();

			if (CurrentPawnRotation.Equals(NewControlRotation, 1e-3f) == false)
			{
				MyPawn->FaceRotation(NewControlRotation, DeltaTime);
			}
		}
	}
}

void ABaseAIController::BeginPlayDelayed()
{
	ABaseCharacter* Char = Cast<ABaseCharacter>(GetPawn());
	SetGenericTeamId(FGenericTeamId(TeamNumber));
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

void ABaseAIController::UpdatedPerception(AActor* Actor, FAIStimulus Stimulus, bool AlertedByAllies)
{
	
	if (GetTeamAttitudeTowards(*Actor)!=ETeamAttitude::Hostile)
		return;

	//UE_LOG(LogTemp, Warning, TEXT("%s"), *Stimulus.Type.Name.ToString());

	TArray<AActor*> PerceivedActors;
	if (AIPerceptionComponent && Sight) AIPerceptionComponent->GetCurrentlyPerceivedActors(Sight->GetSenseImplementation(), PerceivedActors);
	if (PerceivedActors.Contains(Actor))
	{
		KnownEnemies.Add(Actor);
		BlackboardComp->SetValueAsEnum(TEXT("AlertState"), EAlertState::Alerted);
		//BlackboardComp->SetValueAsObject(TEXT("Enemy"), Actor);
	} else
	{
		FVector Direction = Actor->GetVelocity();
		if (KnownEnemies.Contains(Actor)) KnownEnemies.Remove(Actor);
		Direction.Normalize();
		FVector PredictedLocation = Stimulus.StimulusLocation + Direction*300;
		//BlackboardComp->SetValueAsObject(TEXT("Enemy"), nullptr);
		BlackboardComp->SetValueAsEnum(TEXT("AlertState"), EAlertState::Suspicious);
		BlackboardComp->SetValueAsVector(TEXT("StimulusLocation"), Stimulus.StimulusLocation);
	}
	float ClosestDist = -1;
	AActor* ClosestEnemy = nullptr;
	for (auto KnownEnemy : KnownEnemies)
	{
		if (ClosestDist == -1 || ClosestDist > GetPawn()->GetDistanceTo(KnownEnemy))
		{
			ClosestDist = GetPawn()->GetDistanceTo(KnownEnemy);
			ClosestEnemy = KnownEnemy;
		}
	}
	BlackboardComp->SetValueAsObject(TEXT("Enemy"), ClosestEnemy);
	if (!AlertedByAllies) AlertAllies(3000, Actor, Stimulus);
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

void ABaseAIController::AlertAllies(float AlertRadius, AActor* Actor, FAIStimulus Stimulus)
{
	TArray<AActor*> Actors;
	TArray<TEnumAsByte<EObjectTypeQuery>> Objects;
	TArray<AActor*> ActorsToIgnore;
	ActorsToIgnore.Add(this->GetPawn());
	UKismetSystemLibrary::SphereOverlapActors(GetWorld(), GetPawn()->GetActorLocation(), AlertRadius, Objects, ABaseCharacter::StaticClass(), ActorsToIgnore, Actors);
	for (auto FoundActor : Actors)
	{
		if (ABaseCharacter* BaseCharacter = Cast<ABaseCharacter>(FoundActor))
		{
			if (ABaseAIController* AIController = Cast<ABaseAIController>(BaseCharacter->GetController()))
			{
				if (AIController->TeamNumber == TeamNumber)
				{
					AIController->UpdatedPerception(Actor, Stimulus, true);
					//UE_LOG(LogTemp, Warning, TEXT("%s"), *FoundActor->GetActorLabel())
				}
			}
		}
	}
}


void ABaseAIController::OnPerceptionUpdated_Implementation(AActor* Actor, FAIStimulus Stimulus)
{
	UpdatedPerception(Actor, Stimulus);
}
