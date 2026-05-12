// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Interfaces/InteractableInterface.h"
#include "Components/ActorComponent.h"
#include "SeatComponent.generated.h"


class ACharacterBase;
class UBoxComponent;

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class HALOFLOODFANGAME01_API USeatComponent : public USceneComponent, public IInteractableInterface
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	USeatComponent();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	
	//virtual void OnInteract_Implementation(APlayerCharacter* Character) override;

	virtual void EnterSeat(ACharacterBase* Character);

	virtual void ExitSeat(ACharacterBase* Character);

public:
	UPROPERTY(EditDefaultsOnly, meta = (MakeEditWidget))
	FTransform EnterPosition;

	UPROPERTY(EditDefaultsOnly, meta = (MakeEditWidget))
	FTransform ExitPosition;

	UPROPERTY(EditDefaultsOnly)
	UBoxComponent* InteractBox;

	UPROPERTY(EditDefaultsOnly)
	APawn* VehicleSeat;
		
};
