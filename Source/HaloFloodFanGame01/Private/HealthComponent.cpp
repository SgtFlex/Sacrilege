// Fill out your copyright notice in the Description page of Project Settings.


#include "HealthComponent.h"

#include "Core/BaseCharacter.h"

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

float UHealthComponent::TakeDamage(float Damage, bool bIgnoreShields, bool bIgnoreHealthArmor, bool bIgnoreShieldArmor)
{
	float DamageLeft = Damage;
	//GetWorldTimerManager().SetTimer(ShieldDelayTimerHandle, this, &AHaloFloodFanGame01Character::RegenShield, ShieldRegenTickRate, true, ShieldRegenDelay);
	if (Shields > 0)
	{
		DamageLeft = Damage - Shields;
		Shields -= Damage;
	} 
	if (Shields <= 0 && MaxHealth > 0)
	{
		MaxHealth -= DamageLeft;
		if (MaxHealth <= 0)
		{
			this->HealthDepleted();
		}
	}
	return Damage;
}

void UHealthComponent::HealthDepleted()
{
	IDamageableInterface* Owner = Cast<IDamageableInterface>(GetOwner());
	Owner->HealthDepleted();
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

float UHealthComponent::GetShieldArmor() const
{
	return ShieldArmor;
}

void UHealthComponent::SetShieldArmor(float NewShieldArmor)
{
	this->ShieldArmor = NewShieldArmor;
}

float UHealthComponent::GetMaxShieldArmor() const
{
	return MaxShieldArmor;
}

void UHealthComponent::SetMaxShieldArmor(float NewMaxShieldArmor)
{
	this->MaxShieldArmor = NewMaxShieldArmor;
}

