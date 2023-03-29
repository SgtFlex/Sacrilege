// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "DamageableInterface.h"
#include "GameFramework/Character.h"
#include "GameFramework/Pawn.h"
#include "BaseCharacter.generated.h"

class UHealthComponent;
UCLASS()
class HALOFLOODFANGAME01_API ABaseCharacter : public ACharacter, public IDamageableInterface
{
	GENERATED_BODY()

public:
	// Sets default values for this pawn's properties
	ABaseCharacter();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;


	virtual void TakeDamage(float DamageAmount) override;
	
	virtual void HealthDepleted() override;


public:
	UPROPERTY(BlueprintReadWrite, VisibleAnywhere,Category="Stats", meta=(AllowPrivateAccess=true))
	UHealthComponent* HealthComponent;

};
