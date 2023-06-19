// Fill out your copyright notice in the Description page of Project Settings.


#include "HealthComponent.h"
#include "GameFramework/Actor.h"
#include "Kismet/GameplayStatics.h"

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
	UE_LOG(LogTemp, Warning, TEXT("%f"), Health);
	//if (MaxShields > 0 && GetWorld()) GetOwner()->GetWorldTimerManager();
	if (Shields > 0)
	{
		DamageLeft = Damage - Shields;
		
		// if (ShieldMat)
		// {
		// 	for (auto Mesh : MeshArray)
		// 	{
		// 		Mesh->OverlayMaterial = ShieldMat;
		// 		UMaterialInstanceDynamic* ShieldMatDynamic = UMaterialInstanceDynamic::Create(ShieldMat, this->GetOwner());
		// 		ShieldMatDynamic->SetScalarParameterValue(FName("Intensity"), 50 * (Shields/MaxShields));
		// 	}
		// }
		if (SetShields(Shields - Damage) <= 0)
		{
			BreakShields();
		}
	} 
	if (Shields <= 0 && Health > 0)
	{
		//Health -= DamageLeft;
		SetHealth(Health - DamageLeft);
		if (Health <= 0)
		{
			this->HealthDepleted(Damage, Force, HitLocation, HitBoneName);
		}
	}
	//OnTakeDamage.Broadcast(Damage, Force, HitLocation, HitBoneName);
	OnHealthUpdate.Broadcast(this);
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
	this->Health = FMath::Clamp(NewHealth, 0, MaxHealth);
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

float UHealthComponent::SetShields(float NewShields)
{
	return this->Shields = FMath::Clamp(NewShields, 0, MaxShields);
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

void UHealthComponent::BreakShields()
{
	if (ShieldBreakSFX) UGameplayStatics::SpawnSoundAttached(ShieldBreakSFX, GetOwner()->GetRootComponent());
}

void UHealthComponent::RegenShields()
{
	OnHealthUpdate.Broadcast(this);
	if (ShieldStartRegenSFX) UGameplayStatics::SpawnSoundAttached(ShieldStartRegenSFX, GetOwner()->GetRootComponent());
	float ShieldRegenAmount = ShieldRegenRatePerSecond*ShieldRegenTickRate;
	SetShields(FMath::Min(MaxShields, Shields + ShieldRegenAmount));
	if (Shields >= MaxShields) GetOwner()->GetWorldTimerManager().ClearTimer(ShieldDelayTimerHandle);
	
}

