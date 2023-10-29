// Fill out your copyright notice in the Description page of Project Settings.


#include "HealthComponent.h"

#include "NiagaraFunctionLibrary.h"
#include "TimerManager.h"
#include "Components/AudioComponent.h"
#include "Core/CharacterBase.h"
#include "GameFramework/Actor.h"
#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"

// Sets default values for this component's properties
UHealthComponent::UHealthComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
}

void UHealthComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	// Here we list the variables we want to replicate
	DOREPLIFETIME(UHealthComponent, Health);
	DOREPLIFETIME(UHealthComponent, Shields);
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
	if (GetHealth() <= 0) return 0;

	
		float DamageLeft = Damage;
		// if (MaxShields > 0)
		// {
		// 	if (ShieldAudioComponent) ShieldAudioComponent->Stop();
		// 	GetOwner()->GetWorldTimerManager().ClearTimer(ShieldRegenTimer);
		// 	GetOwner()->GetWorldTimerManager().SetTimer(ShieldDelayTimerHandle, this, &UHealthComponent::StartShieldRegen, ShieldRegenDelay);
		// }
		if (Shields > 0)
		{
			DamageLeft = Damage - Shields;
			SetShields(Shields - Damage);
			// if (Shields <= 0)
			// {
			// 	BreakShields();
			// } else
			// {
			// 	if (ShieldMat)
			// 	{
			// 		UMeshComponent* MeshComp = Cast<UMeshComponent>(GetOwner()->GetComponentByClass(UMeshComponent::StaticClass()));
			// 		MeshComp->SetOverlayMaterial(ShieldMat);
			// 		UMaterialInstanceDynamic* ShieldMatDynamic = UMaterialInstanceDynamic::Create(ShieldMat, this->GetOwner());
			// 		ShieldMatDynamic->SetScalarParameterValue(FName("Intensity"), 40 * (Shields/MaxShields));
			// 	}
			// }
		}
		if (Health > 0 && Shields <= 0)
		{
			if (SetHealth(Health - DamageLeft) <= 0) //New health value set.
			{
				HealthDepleted(Damage, Force, HitLocation, HitBoneName, EventInstigator, DamageCauser);
			}
		}
		
		OnHealthUpdate.Broadcast(this);
		UE_LOG(LogTemp, Warning, TEXT("-------------------------------"));
		Multi_TakeDamage(Damage, Force, HitLocation, HitBoneName, EventInstigator, DamageCauser);
	
	
	return Damage;
}

void UHealthComponent::Multi_TakeDamage_Implementation(float Damage, FVector Force, FVector HitLocation,
	FName HitBoneName, AController* EventInstigator, AActor* DamageCauser, bool bIgnoreShields, bool bIgnoreHealthArmor,
	bool bIgnoreShieldArmor)
{
	//if (GetHealth() <= 0) return;
	UE_LOG(LogTemp, Warning, TEXT("Called TakeDamage on: %s. Health: %f"), *UEnum::GetValueAsString(GetOwnerRole()), GetHealth());
	if (MaxShields > 0)
	{
		if (ShieldAudioComponent) ShieldAudioComponent->Stop();
		GetOwner()->GetWorldTimerManager().ClearTimer(ShieldRegenTimer);
		GetOwner()->GetWorldTimerManager().SetTimer(ShieldDelayTimerHandle, this, &UHealthComponent::StartShieldRegen, ShieldRegenDelay);
	}

	if (GetHealth() <= 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("Health reached 0 on: %s"), *UEnum::GetValueAsString(GetOwnerRole()));
		//HealthDepleted(Damage, Force, HitLocation, HitBoneName, EventInstigator, DamageCauser);
	}
	OnHealthUpdate.Broadcast(this);
}

void UHealthComponent::HealthDepleted(float Damage, FVector Force, FVector HitLocation, FName HitBoneName, AController* EventInstigator, AActor* DamageCauser)
{
	UE_LOG(LogTemp, Warning, TEXT("Called HealthDepleted on: %s"), *UEnum::GetValueAsString(GetOwnerRole()));
	GetWorld()->GetTimerManager().ClearTimer(ShieldDelayTimerHandle);
	OnHealthDepleted.Broadcast(Damage, Force, HitLocation, HitBoneName, EventInstigator, DamageCauser);
}

bool UHealthComponent::IsAlive()
{
	return (Health > 0);
}

float UHealthComponent::GetHealth() const
{
	return Health;
}

float UHealthComponent::SetHealth(float NewHealth)
{
	return this->Health = FMath::Clamp(NewHealth, 0, MaxHealth);
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
	if (ShieldMat) {
		UMeshComponent* MeshComp = Cast<UMeshComponent>(GetOwner()->GetComponentByClass(UMeshComponent::StaticClass()));
		MeshComp->SetOverlayMaterial(nullptr);
	}
	if (ShieldBreakFX) UNiagaraFunctionLibrary::SpawnSystemAttached(ShieldBreakFX, Cast<ACharacterBase>(GetOwner())->GetMesh(),
		NAME_None, FVector(0,0,0), FRotator(0,0,0),EAttachLocation::SnapToTarget, true);
	if (ShieldBreakSFX) UGameplayStatics::SpawnSoundAttached(ShieldBreakSFX, GetOwner()->GetRootComponent());
}

void UHealthComponent::StartShieldRegen()
{
	if (ShieldStartRegenSFX) ShieldAudioComponent = UGameplayStatics::SpawnSoundAttached(ShieldStartRegenSFX, GetOwner()->GetRootComponent(), NAME_None,
		FVector(ForceInit), FRotator::ZeroRotator, EAttachLocation::KeepRelativeOffset, false, 1, 1, ShieldStartRegenSFX->GetDuration() * Shields/MaxShields);
	GetOwner()->GetWorldTimerManager().SetTimer(ShieldRegenTimer, this, &UHealthComponent::RegenShields, ShieldRegenTickRate, true);
}

void UHealthComponent::RegenShields()
{
	float ShieldRegenAmount = ShieldRegenRatePerSecond*ShieldRegenTickRate;
	SetShields(FMath::Min(MaxShields, Shields + ShieldRegenAmount));
	if (Shields >= MaxShields) StopShieldRegen();
	OnHealthUpdate.Broadcast(this);
}

void UHealthComponent::StopShieldRegen()
{
	if (ShieldFinishRegenSFX) UGameplayStatics::SpawnSound2D(GetWorld(), ShieldFinishRegenSFX);
	GetOwner()->GetWorldTimerManager().ClearTimer(ShieldRegenTimer);
}

