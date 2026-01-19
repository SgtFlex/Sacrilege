// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "DamageableInterface.h"
#include "Components/ActorComponent.h"
#include "HealthComponent.generated.h"


class UNiagaraSystem;
DECLARE_DYNAMIC_MULTICAST_DELEGATE_SixParams(FOnHealthDepleted, float, Damage, FVector, Force, FVector, HitLocation,
                                              FName, HitBoneName, AController*, EventInstigator, AActor*, DamageCauser);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnHealthUpdate, UHealthComponent*, HealthComp);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnHealthDamaged, float, Health);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnShieldDamaged, float, Shields);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnShieldBreak);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnShieldStartRegen);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnShieldFinishRegen);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnHealthReplicated, float, Health);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnShieldsReplicated, float, Shields);



UCLASS(Blueprintable, ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
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

	UFUNCTION(Server, Reliable, BlueprintCallable)
	void TakeDamage(float Damage, FVector Force = FVector(0,0,0), FVector HitLocation = FVector(0,0,0), FName HitBoneName = "", AController* EventInstigator = nullptr, AActor* DamageCauser = nullptr, bool bIgnoreShields = false, bool bIgnoreHealthArmor = false, bool bIgnoreShieldArmor = false);

	UFUNCTION(NetMulticast, Reliable)
	void Multi_TakeDamage(float Damage, FVector Force = FVector(0,0,0), FVector HitLocation = FVector(0,0,0), FName HitBoneName = "", AController* EventInstigator = nullptr, AActor* DamageCauser = nullptr, bool bIgnoreShields = false, bool bIgnoreHealthArmor = false, bool bIgnoreShieldArmor = false);
	
	void HealthDepleted(float Damage, FVector Force, FVector HitLocation, FName HitBoneName, AController* EventInstigator = nullptr, AActor* DamageCauser = nullptr);

	UFUNCTION(NetMulticast, Unreliable)
	void Multi_HealthDepleted(float Damage, FVector Force, FVector HitLocation, FName HitBoneName, AController* EventInstigator = nullptr, AActor* DamageCauser = nullptr);

	UFUNCTION(BlueprintCallable, BlueprintPure)
	bool IsAlive();

	UFUNCTION()
	void OnRep_Health();

	UFUNCTION()
	void OnRep_Shields();
public:
	UPROPERTY(EditAnywhere, ReplicatedUsing=OnRep_Health, Category="Unit Information|Health", meta = (DisplayPriority=1))
	float Health = 100;
	
	UPROPERTY(EditAnywhere, Category="Unit Information|Health", meta = (DisplayPriority=1))
	float MaxHealth = 100;
	
	UPROPERTY(EditAnywhere, Category="Unit Information|Health", meta = (DisplayPriority=1))
	float MaxHealthRegenAmount = 0;
	
	UPROPERTY(EditAnywhere, Category="Unit Information|Health", meta = (DisplayPriority=1))
	float HealthRegenDelay = 0;
	
	UPROPERTY(EditAnywhere, Category="Unit Information|Health", meta = (DisplayPriority=1))
	float HealthRegenPerSec = 1;
	
	UPROPERTY(EditAnywhere, Category="Unit Information|Health", meta = (DisplayPriority=1))
	float HealthArmor = 0;
	
	UPROPERTY(EditAnywhere, Category="Unit Information|Health", meta = (DisplayPriority=1))
	float MaxHealthArmor = 100;
	
	UPROPERTY(EditAnywhere, ReplicatedUsing=OnRep_Shields, Category="Unit Information|Shields", meta = (DisplayPriority=1))
	float Shields = 100;
	
	UPROPERTY(EditAnywhere, Category="Unit Information|Shields", meta = (DisplayPriority=1))
	float MaxShields = 100;
	
	UPROPERTY(EditAnywhere, Category="Unit Information|Shields", meta = (DisplayPriority=1))
	float ShieldRegenDelay = 3;
	
	UPROPERTY(EditAnywhere, Category="Unit Information|Shields", meta = (DisplayPriority=1))
	float ShieldRegenRatePerSecond = 30;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	UNiagaraSystem* ShieldRegenVFX;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	USoundBase* ShieldBreakSFX;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	USoundBase* ShieldLowSFX;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	USoundBase* ShieldWarningSFX;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	UNiagaraSystem* ShieldHitFX;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	UNiagaraSystem* ShieldBreakFX;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	USoundBase* ShieldStartRegenSFX;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	USoundBase* ShieldFinishRegenSFX;

	UPROPERTY()
	UAudioComponent* ShieldAudioComponent;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	USoundAttenuation* ShieldAttenuationSettings;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	UMaterialInterface* ShieldMat;

	UPROPERTY()
	UMaterialInstanceDynamic* ShieldMatInstance;

	UPROPERTY(VisibleAnywhere, BlueprintAssignable)
	FOnHealthDepleted OnHealthDepleted;

	UPROPERTY(VisibleAnywhere, BlueprintAssignable)
	FOnHealthUpdate OnHealthUpdate;

	UPROPERTY()
	float HealthRegenTickRate = 0.01;
	
	UPROPERTY()
	float ShieldRegenTickRate = 0.01;

	UPROPERTY()
	UMeshComponent* MeshComp;

	//DELEGATES

	UPROPERTY(BlueprintAssignable)
	FOnHealthReplicated OnHealthReplicated;

	UPROPERTY(BlueprintAssignable)
	FOnShieldsReplicated OnShieldsReplicated;

	UPROPERTY(BlueprintAssignable)
	FOnShieldDamaged OnShieldDamaged;

	UPROPERTY(BlueprintAssignable)
	FOnShieldBreak OnShieldBreak;

	UPROPERTY(BlueprintAssignable)
	FOnHealthDamaged OnHealthDamaged;
	
private:
	UPROPERTY()
	FTimerHandle ShieldDelayTimerHandle;
	UPROPERTY()
	FTimerHandle ShieldRegenTimer;
	UPROPERTY()
	FTimerHandle ShieldMatTimer;
	UPROPERTY()
	float LastDamagedTime = -1;
	UPROPERTY(EditDefaultsOnly)
	float ShieldMatDuration = 0.5f;
	
public:
	UFUNCTION(BlueprintCallable)
	float GetHealth() const;
	UFUNCTION(BlueprintCallable)
	float SetHealth(float NewHealth);
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
	UFUNCTION(NetMulticast, Unreliable)
	void BreakShields();
	UFUNCTION()
	void StartShieldRegen();
	UFUNCTION()
	void RegenShields();
	UFUNCTION()
	void StopShieldRegen();
	UFUNCTION()
	void PlayShieldFX(bool Show);
};
