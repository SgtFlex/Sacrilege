// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "HealthComponent.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class HALOFLOODFANGAME01_API UHealthComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UHealthComponent();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	float TakeDamage(float Damage, bool bIgnoreShields, bool bIgnoreHealthArmor, bool bIgnoreShieldArmor);

	void HealthDepleted();

public:
	UPROPERTY(EditAnywhere, meta = (Category="Health"))
	float Health = 100;
	UPROPERTY(EditAnywhere, meta = (Category="Health"))
	float MaxHealth = 100;
	UPROPERTY(EditAnywhere, meta = (Category="Health"))
	float MaxHealthRegenAmount = 0;
	UPROPERTY(EditAnywhere, meta = (Category="Health"))
	float HealthRegenDelay = 0;
	UPROPERTY(EditAnywhere, meta = (Category="Health"))
	float HealthRegenPerSec = 1;
	UPROPERTY(EditAnywhere, meta = (Category="Health"))
	float HealthArmor = 0;
	UPROPERTY(EditAnywhere, meta = (Category="Health"))
	float MaxHealthArmor = 100;
	UPROPERTY(EditAnywhere, meta = (Category="Shields"))
	float Shields = 100;
	UPROPERTY(EditAnywhere, meta = (Category="Shields"))
	float MaxShields = 100;
	UPROPERTY(EditAnywhere, meta = (Category="Shields"))
	float ShieldRegenDelay = 3;
	UPROPERTY(EditAnywhere, meta = (Category="Shields"))
	float ShieldRegenRatePerSecond = 30;
	UPROPERTY(EditAnywhere, meta = (Category="Shields"))
	float ShieldArmor = 0;
	UPROPERTY(EditAnywhere, meta = (Category="Shields"))
	float MaxShieldArmor = 100;

	float HealthRegenTickRate = 0.01;
	float ShieldRegenTickRate = 0.01;
public:
	float GetHealth() const;
	void SetHealth(float Health);
	float GetMaxHealth() const;
	void SetMaxHealth(float MaxHealth);
	float GetHealthArmor() const;
	void SetHealthArmor(float HealthArmor);
	float GetMaxHealthArmor() const;
	void SetMaxHealthArmor(float MaxHealthArmor);
	float GetShields() const;
	void SetShields(float Shields);
	float GetMaxShields() const;
	void SetMaxShields(float MaxShields);
	float GetShieldRegenDelay() const;
	void SetShieldRegenDelay(float ShieldRegenDelay);
	float GetShieldRegenRatePerSecond() const;
	void SetShieldRegenRatePerSecond(float ShieldRegenRatePerSecond);
	float GetShieldArmor() const;
	void SetShieldArmor(float ShieldArmor);
	float GetMaxShieldArmor() const;
	void SetMaxShieldArmor(float MaxShieldArmor);
};
