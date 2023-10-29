// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "PawnBase.generated.h"

class ASmartObject;
class UCapsuleComponent;
UCLASS()
class HALOFLOODFANGAME01_API APawnBase : public APawn
{
	GENERATED_BODY()

protected:
	UPROPERTY(Category=Character, VisibleAnywhere, BlueprintReadOnly, meta=(AllowPrivateAccess = "true"))
	USkeletalMeshComponent* MainSkelMesh;

	UPROPERTY(Category=Character, VisibleAnywhere, BlueprintReadOnly, meta=(AllowPrivateAccess = "true"))
	UCapsuleComponent* CapsuleCollider;

public:
	// Sets default values for this pawn's properties
	APawnBase();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	void Death();

	virtual void DeathCleanup();

private:
	virtual float TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser) override;

public:
	float MaxHealth;
	float CurHealth;
	float HealthRegenDelay;
	float HealthRegenPerSec;
	float MinHealth;
	float Armor;
	float MaxShields;
	float CurShields;
	float ShieldRegenDelay;
	float ShieldRegenPerSec;
};
