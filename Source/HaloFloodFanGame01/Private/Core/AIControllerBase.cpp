// Fill out your copyright notice in the Description page of Project Settings.


#include "Core/AIControllerBase.h"

#include "Bullet.h"
#include "Components/BulletFiringComponent.h"
#include "Core/GunBase.h"
#include "SmartObject.h"
#include "Core/WeaponBase.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Core/CharacterBase.h"
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

	bStartAILogicOnPossess = true;
}

void AAIControllerBase::BeginPlay()
{
	Super::BeginPlay();
	if (ACharacterBase* Char = Cast<ACharacterBase>(GetPawn()))
	{
		SetGenericTeamId(FGenericTeamId(Char->TeamId));
	}
	else
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

void AAIControllerBase::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);
	if (ACharacterBase* Char = Cast<ACharacterBase>(InPawn))
	{
		PawnChar = Char;
		GetBlackboardComponent()->SetValueAsObject("Character", Char);
		SetGenericTeamId(Char->TeamId);
	} else
	{
		SetGenericTeamId(FGenericTeamId(TeamNumber));
	}
}

void AAIControllerBase::BeginPlayDelayed()
{
	AIPerceptionComponent->OnTargetPerceptionUpdated.AddDynamic(this, &AAIControllerBase::OnPerceptionUpdated);
	
	
	if (ACharacterBase* Char = Cast<ACharacterBase>(GetPawn()))
	{
		SetGenericTeamId(Char->TeamId);
		if (ASmartObject* SmartObj = Char->SmartObject) {
			SetSmartObject(SmartObj);
		}
	}
}

ETeamAttitude::Type AAIControllerBase::GetTeamAttitudeTowards(const AActor& Other) const
{
	//return Super::GetTeamAttitudeTowards(Other);
	if (const APawn* OtherPawn = Cast<APawn>(&Other)) {
		if (const IGenericTeamAgentInterface* TeamAgent = Cast<IGenericTeamAgentInterface>(OtherPawn->GetController()))
		{
			//UE_LOG(LogTemp, Warning, TEXT("%s get attitude"), *Other.GetActorLabel());
			if (TeamAgent->GetGenericTeamId().GetId() == 0 || TeamAgent->GetGenericTeamId() == 255)
				return ETeamAttitude::Neutral;
			else if (TeamAgent->GetGenericTeamId().GetId() == GetGenericTeamId().GetId())
				return ETeamAttitude::Friendly;
			else
				return ETeamAttitude::Hostile;
			//return Super::GetTeamAttitudeTowards(Other);
		}
		
	}
	return ETeamAttitude::Neutral;
}

void AAIControllerBase::SetSmartObject(ASmartObject* NewSmartObject)
{
	
	SmartObject = NewSmartObject;
	//GEngine->AddOnScreenDebugMessage(-1, 1, FColor::Red, TEXT("Adding smart object"));
	if (!SmartObject) return;
	FGameplayTag SubTag;
	BehaviorTreeComp->SetDynamicSubtree(SubTag, SmartObject->DynamicTree);
	BlackboardComp->SetValueAsBool(FName("HasSmartObject"), true);
	BlackboardComp->SetValueAsObject(FName("SmartObject"), SmartObject);
	//BlackboardComp->SetValueAsEnum(TEXT("AlertState"), SmartObject->AlertState);
	SetAlertState(SmartObject->AlertState);
	// BlackboardComp->SetValueAsVector(FName("StimulusLocation"), SmartObject->GetActorLocation());
}

void AAIControllerBase::UpdatedPerception(AActor* Actor, FAIStimulus Stimulus, bool AlertedByAllies)
{
	//UE_LOG(LogTemp, Warning, TEXT("Attitude towards: %s %s"), *Actor->GetActorLabel(), *UEnum::GetValueAsString(GetTeamAttitudeTowards(*Actor)));
	if (GetTeamAttitudeTowards(*Actor)!=ETeamAttitude::Hostile)
	{
		return;
	}

	//TODO this may be causing errors in AI. Needs fixing
	// TArray<AActor*> PerceivedActors;
	// if (AIPerceptionComponent && Sight) AIPerceptionComponent->GetCurrentlyPerceivedActors(Sight->GetSenseImplementation(), PerceivedActors);
	// if (PerceivedActors.Contains(Actor))
	// {
	// 	KnownEnemies.Add(Actor);
	// 	BlackboardComp->SetValueAsEnum(TEXT("AlertState"), EAlertState::Alerted);
	// } else
	// {
	// 	if (Stimulus.Type == Sight->GetSenseID() && KnownEnemies.Contains(Actor)) KnownEnemies.Remove(Actor);
	// 	if (PerceivedActors.IsEmpty()) BlackboardComp->SetValueAsEnum(TEXT("AlertState"), EAlertState::Suspicious);
	// 	BlackboardComp->SetValueAsVector(TEXT("StimulusLocation"), Stimulus.StimulusLocation);
	// }
	//
	if (Stimulus.Type == Sight->GetSenseID() && !AlertedByAllies)
	{
		UpdateTargetedEnemy(Actor);
	} else
	{
		if (BlackboardComp->GetValueAsEnum(TEXT("AlertState")) != EAlertState::Alerted) SetAlertState(Suspicious);
		BlackboardComp->SetValueAsVector(TEXT("StimulusLocation"), Stimulus.StimulusLocation);
	}
	if (!AlertedByAllies) AlertAllies(3000, Actor, Stimulus);
}

void AAIControllerBase::UpdateTargetedEnemy(AActor* Actor)
{
	
	TArray<AActor*> SightedActors;
	if (AIPerceptionComponent && Sight) AIPerceptionComponent->GetCurrentlyPerceivedActors(Sight->GetSenseImplementation(), SightedActors);
	AActor* ClosestEnemy = nullptr;
	if (!SightedActors.IsEmpty())
	{
		float ClosestDist = -1;
		
		for (auto SightedActor : SightedActors)
		{
			if (GetTeamAttitudeTowards(*SightedActor)==ETeamAttitude::Hostile && (ClosestDist == -1 || GetPawn()->GetDistanceTo(SightedActor) < ClosestDist))
			{
				ClosestDist = GetPawn()->GetDistanceTo(SightedActor);
				ClosestEnemy = SightedActor;
			}
		}
		if (ClosestEnemy)
		{
			//BlackboardComp->SetValueAsEnum(TEXT("AlertState"), EAlertState::Alerted);
			SetAlertState(Alerted);
			BlackboardComp->SetValueAsObject(TEXT("Enemy"), ClosestEnemy);
			// if (ACharacterBase* EnemyChar = Cast<ACharacterBase>(ClosestEnemy))
			// {
			// 	EnemyChar->OnKilled.AddDynamic(this, &AAIControllerBase::UpdateTargetedEnemy);
			// }
			BlackboardComp->SetValueAsVector(TEXT("StimulusLocation"), ClosestEnemy->GetActorLocation());
			OnEnemyUpdated.Broadcast();
			return;
		}
	}
	if (Actor) BlackboardComp->SetValueAsVector(TEXT("StimulusLocation"), Actor->GetActorLocation());
	//BlackboardComp->SetValueAsEnum(TEXT("AlertState"), EAlertState::Suspicious);
	SetAlertState(Suspicious);
	BlackboardComp->SetValueAsObject(TEXT("Enemy"), nullptr);
	
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
	ActorsToIgnore.Add(GetPawn());
	UKismetSystemLibrary::SphereOverlapActors(GetWorld(), GetPawn()->GetActorLocation(), AlertRadius, Objects, APawn::StaticClass(), ActorsToIgnore, Actors);
	for (auto FoundActor : Actors)
	{
		
		if (APawn* BaseCharacter = Cast<APawn>(FoundActor))
		{
			if (AAIControllerBase* AIController = Cast<AAIControllerBase>(BaseCharacter->GetController()))
			{
				if (GetTeamAttitudeTowards(*FoundActor) == ETeamAttitude::Friendly)
				{
					AIController->UpdatedPerception(Actor, Stimulus, true);
				}
			}
		}
	}
}

void AAIControllerBase::SetAlertState(TEnumAsByte<EAlertState> NewAlertState)
{
	if (NewAlertState != BlackboardComp->GetValueAsEnum("AlertState"))
		OnAlertStateChanged.Broadcast(NewAlertState);
	BlackboardComp->SetValueAsEnum(TEXT("AlertState"), NewAlertState);
	
	if (PawnChar)
	{
		//GEngine->AddOnScreenDebugMessage(-1, 2, FColor(255,255,255,255), "Setting alert state");
		PawnChar->AlertState = NewAlertState;
	}
}

void AAIControllerBase::SetGenericTeamId(const FGenericTeamId& NewTeamID)
{
	Super::SetGenericTeamId(NewTeamID);
	TeamNumber = NewTeamID.GetId();
}


void AAIControllerBase::OnPerceptionUpdated_Implementation(AActor* Actor, FAIStimulus Stimulus)
{
	if (!GetPawn()) return;
	UpdatedPerception(Actor, Stimulus);
}
