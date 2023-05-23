// Fill out your copyright notice in the Description page of Project Settings.


#include "HealthComponent.h"

#include "Core/BaseCharacter.h"
#include "Engine/DamageEvents.h"
#include "GameFramework/Actor.h"

// Sets default values for this component's properties
UHealthComponent::UHealthComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
}


// Called when the game starts
void UHealthComponent::BeginPlay()
{
	Super::BeginPlay();

	// ...
	
}


// Called every frame
void UHealthComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}

float UHealthComponent::TakeDamage(float Damage, FVector Force, FVector HitLocation, FName HitBoneName, AController* EventInstigator, AActor* DamageCauser, bool bIgnoreShields, bool bIgnoreHealthArmor, bool bIgnoreShieldArmor)
{
	float DamageLeft = Damage;
	GetOwner()->GetWorldTimerManager().SetTimer(ShieldDelayTimerHandle, this, &UHealthComponent::RegenShields, ShieldRegenTickRate, true, ShieldRegenDelay);
	if (Shields > 0)
	{
		DamageLeft = Damage - Shields;
		Shields -= Damage;
	} 
	if (Shields <= 0 && Health > 0)
	{
		Health -= DamageLeft;
		if (Health <= 0)
		{
			this->HealthDepleted(Damage, Force, HitLocation, HitBoneName);
		}
	}
	OnTakeDamage.Broadcast(Damage, Force, HitLocation, HitBoneName);
	return Damage;
}

void UHealthComponent::HealthDepleted(float Damage, FVector Force, FVector HitLocation, FName HitBoneName)
{
	OnHealthDepleted.Broadcast(Damage, Force, HitLocation, HitBoneName);
}

float UHealthComponent::GetHealth() const
{
	return Health;
}

void UHealthComponent::SetHealth(float NewHealth)
{
	this->Health = NewHealth;
}

float UHealthComponent::GetMaxHealth() const
{
	return MaxHealth;
}

void UHealthComponent::SetMaxHealth(float NewMaxHealth)
{
	this->MaxHealth = NewMaxHealth;
}

float UHealthComponent::GetHealthArmor() const
{
	return HealthArmor;
}

void UHealthComponent::SetHealthArmor(float NewHealthArmor)
{
	this->HealthArmor = NewHealthArmor;
}

float UHealthComponent::GetMaxHealthArmor() const
{
	return MaxHealthArmor;
}

void UHealthComponent::SetMaxHealthArmor(float NewMaxHealthArmor)
{
	this->MaxHealthArmor = NewMaxHealthArmor;
}

float UHealthComponent::GetShields() const
{
	return Shields;
}

void UHealthComponent::SetShields(float NewShields)
{
	this->Shields = NewShields;
}

float UHealthComponent::GetMaxShields() const
{
	return MaxShields;
}

void UHealthComponent::SetMaxShields(float NewMaxShields)
{
	this->MaxShields = NewMaxShields;
}

float UHealthComponent::GetShieldRegenDelay() const
{
	return ShieldRegenDelay;
}

void UHealthComponent::SetShieldRegenDelay(float NewShieldRegenDelay)
{
	this->ShieldRegenDelay = NewShieldRegenDelay;
}

float UHealthComponent::GetShieldRegenRatePerSecond() const
{
	return ShieldRegenRatePerSecond;
}

void UHealthComponent::SetShieldRegenRatePerSecond(float NewShieldRegenRatePerSecond)
{
	this->ShieldRegenRatePerSecond = NewShieldRegenRatePerSecond;
}

void UHealthComponent::RegenShields()
{
	Shields += (ShieldRegenRatePerSecond*ShieldRegenTickRate);
	if (Shields >= MaxShields) GetOwner()->GetWorldTimerManager().ClearTimer(ShieldDelayTimerHandle);
	
}

