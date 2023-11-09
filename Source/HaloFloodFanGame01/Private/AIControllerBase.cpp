// Fill out your copyright notice in the Description page of Project Settings.


#include "AIControllerBase.h"

#include "NavigationSystem.h"
#include "SmartObject.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Core/CharacterBase.h"
#include "Core/PawnBase.h"
#include "Kismet/GameplayStatics.h"
#include "Perception/AISenseConfig.h"
#include "Perception/AISenseConfig_Damage.h"
#include "Perception/AISenseConfig_Hearing.h"
#include "Perception/AISenseConfig_Sight.h"
#include "Perception/AISenseConfig_Team.h"
#include "Perception/AISenseConfig_Touch.h"

AAIControllerBase::AAIControllerBase()
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

void AAIControllerBase::BeginPlay()
{
	Super::BeginPlay();
	SetGenericTeamId(FGenericTeamId(TeamNumber));
	GetWorldTimerManager().SetTimer(Delay, this, &AAIControllerBase::BeginPlayDelayed, 0.1f, false);
	BehaviorTreeComp->StartLogic();
}

void AAIControllerBase::UpdateControlRotation(float DeltaTime, bool bUpdatePawn)
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

void AAIControllerBase::BeginPlayDelayed()
{
	AIPerceptionComponent->OnTargetPerceptionUpdated.AddDynamic(this, &AAIControllerBase::OnPerceptionUpdated);
	ACharacterBase* Char = Cast<ACharacterBase>(GetPawn());
	
	if (Char)
	{
		if (ASmartObject* SmartObj = Char->SmartObject) {
			SetSmartObject(SmartObj);
		}
	}
}

ETeamAttitude::Type AAIControllerBase::GetTeamAttitudeTowards(const AActor& Other) const
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

void AAIControllerBase::SetSmartObject(ASmartObject* SmartObject)
{
	FGameplayTag SubTag;
	BehaviorTreeComp->SetDynamicSubtree(SubTag, SmartObject->DynamicTree);
	BlackboardComp->SetValueAsBool(FName("HasSmartObject"), true);
	BlackboardComp->SetValueAsObject(FName("SmartObject"), SmartObject);
	BlackboardComp->SetValueAsEnum(TEXT("AlertState"), SmartObject->AlertState);
	// BlackboardComp->SetValueAsVector(FName("StimulusLocation"), SmartObject->GetActorLocation());
}

void AAIControllerBase::UpdatedPerception(AActor* Actor, FAIStimulus Stimulus, bool AlertedByAllies)
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

void AAIControllerBase::HearingStimulusUpdated(AActor* Actor, FAIStimulus Stimulus)
{
}

void AAIControllerBase::SightStimulusUpdated(AActor* Actor, FAIStimulus Stimulus)
{
}

void AAIControllerBase::DamageStimulusUpdated(AActor* Actor, FAIStimulus Stimulus)
{
}

void AAIControllerBase::AlertAllies(float AlertRadius, AActor* Actor, FAIStimulus Stimulus)
{
	TArray<AActor*> Actors;
	TArray<TEnumAsByte<EObjectTypeQuery>> Objects;
	TArray<AActor*> ActorsToIgnore;
	ActorsToIgnore.Add(this->GetPawn());
	UKismetSystemLibrary::SphereOverlapActors(GetWorld(), GetPawn()->GetActorLocation(), AlertRadius, Objects, ACharacterBase::StaticClass(), ActorsToIgnore, Actors);
	for (auto FoundActor : Actors)
	{
		if (ACharacterBase* BaseCharacter = Cast<ACharacterBase>(FoundActor))
		{
			if (AAIControllerBase* AIController = Cast<AAIControllerBase>(BaseCharacter->GetController()))
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


void AAIControllerBase::OnPerceptionUpdated_Implementation(AActor* Actor, FAIStimulus Stimulus)
{
	UpdatedPerception(Actor, Stimulus);
}
