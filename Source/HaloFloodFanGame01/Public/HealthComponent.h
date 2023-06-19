// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "DamageableInterface.h"
#include "Components/ActorComponent.h"
#include "HealthComponent.generated.h"


DECLARE_DYNAMIC_MULTICAST_DELEGATE_FourParams(FOnTakeDamage, float, Damage, FVector, Force, FVector, HitLocation, FName, HitBoneName);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_FourParams(FOnHealthDepleted, float, Damage, FVector, Force, FVector, HitLocation, FName, HitBoneName);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnHealthUpdate, UHealthComponent*, HealthComp);

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
	
	float TakeDamage(float Damage, FVector Force = FVector(0,0,0), FVector HitLocation = FVector(0,0,0), FName HitBoneName = "", AController* EventInstigator = nullptr, AActor* DamageCauser = nullptr, bool bIgnoreShields = false, bool bIgnoreHealthArmor = false, bool bIgnoreShieldArmor = false);

	
	void HealthDepleted(float Damage, FVector Force, FVector HitLocation, FName HitBoneName);

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

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	USoundBase* ShieldBreakSFX;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	USoundBase* ShieldStartRegenSFX;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	USoundBase* ShieldRegenSFX;

	UPROPERTY()
	FOnTakeDamage OnTakeDamage;

	UPROPERTY()
	FOnHealthDepleted OnHealthDepleted;

	UPROPERTY(VisibleAnywhere, BlueprintAssignable)
	FOnHealthUpdate OnHealthUpdate;

	float HealthRegenTickRate = 0.01;
	float ShieldRegenTickRate = 0.01;
	
private:
	UPROPERTY()
	FTimerHandle ShieldDelayTimerHandle;
	
public:
	UFUNCTION(BlueprintCallable)
	float GetHealth() const;
	UFUNCTION(BlueprintCallable)
	void SetHealth(float NewHealth);
	UFUNCTION(BlueprintCallable)
	float GetMaxHealth() const;
	UFUNCTION(BlueprintCallable)
	void SetMaxHealth(float NewMaxHealth);
	UFUNCTION(BlueprintCallable)
	float GetHealthArmor() const;
	UFUNCTION(BlueprintCallable)
	void SetHealthArmor(float NewHealthArmor);
	UFUNCTION(BlueprintCallable)
	float GetMaxHealthArmor() const;
	UFUNCTION(BlueprintCallable)
	void SetMaxHealthArmor(float NewMaxHealthArmor);
	UFUNCTION(BlueprintCallable)
	float GetShields() const;
	UFUNCTION(BlueprintCallable)
	float SetShields(float NewShields);
	UFUNCTION(BlueprintCallable)
	float GetMaxShields() const;
	UFUNCTION(BlueprintCallable)
	void SetMaxShields(float NewMaxShields);
	UFUNCTION(BlueprintCallable)
	float GetShieldRegenDelay() const;
	UFUNCTION(BlueprintCallable)
	void SetShieldRegenDelay(float NewShieldRegenDelay);
	float GetShieldRegenRatePerSecond() const;
	UFUNCTION(BlueprintCallable)
	void SetShieldRegenRatePerSecond(float NewShieldRegenRatePerSecond);
	UFUNCTION()
	void BreakShields();
	UFUNCTION()
	void RegenShields();	
};
