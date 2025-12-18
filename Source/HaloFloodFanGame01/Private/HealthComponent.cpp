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

	ShieldAudioComponent = CreateDefaultSubobject<UAudioComponent>("AudioComp");
	
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

	MeshComp = Cast<UMeshComponent>(GetOwner()->GetComponentByClass(UMeshComponent::StaticClass()));
	ShieldAudioComponent->AttachToComponent(GetOwner()->GetRootComponent(), FAttachmentTransformRules::SnapToTargetIncludingScale);
	if (ShieldAttenuationSettings)
	{
		ShieldAudioComponent->AttenuationSettings = ShieldAttenuationSettings;
	} 
}


// Called every frame
void UHealthComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}

void UHealthComponent::TakeDamage_Implementation(float Damage, FVector Force, FVector HitLocation, FName HitBoneName, AController* EventInstigator, AActor* DamageCauser, bool bIgnoreShields, bool bIgnoreHealthArmor, bool bIgnoreShieldArmor)
{
	if (!IsAlive()) return;
	
	float DamageLeft = Damage;
	
	if (Shields > 0)
	{
		DamageLeft = Damage - Shields;
		if (SetShields(Shields - Damage) <= 0) //After setting our new shields, check if our shields are now 0
		{
			BreakShields();
			OnShieldBreak.Broadcast();
		}
			
	}
	if (DamageLeft > 0 && Shields <= 0)
	{
		if (SetHealth(Health - DamageLeft) <= 0)
			HealthDepleted(DamageLeft, Force, HitLocation, HitBoneName, EventInstigator, DamageCauser);
	}		
	
	Multi_TakeDamage(Damage, Force, HitLocation, HitBoneName, EventInstigator, DamageCauser);
	OnHealthUpdate.Broadcast(this);
}

void UHealthComponent::Multi_TakeDamage_Implementation(float Damage, FVector Force, FVector HitLocation,
	FName HitBoneName, AController* EventInstigator, AActor* DamageCauser, bool bIgnoreShields, bool bIgnoreHealthArmor,
	bool bIgnoreShieldArmor)
{
	if (!IsAlive()) return;
	
	if (MaxShields > 0)
	{
		// if (Shields <= 0)
		// 	BreakShields();
		GetOwner()->GetWorldTimerManager().ClearTimer(ShieldRegenTimer);
		GetOwner()->GetWorldTimerManager().SetTimer(ShieldDelayTimerHandle, this, &UHealthComponent::StartShieldRegen, ShieldRegenDelay);
	}
	if (ShieldWarningSFX && MaxShields > 0 && (Shields <= 0) && (ShieldAudioComponent->GetSound() != ShieldWarningSFX))
	{
		ShieldAudioComponent->SetSound(ShieldWarningSFX);
		ShieldAudioComponent->Play();
	}
	else if (ShieldLowSFX && Shields <= (MaxShields * 0.25) && (Shields > 0) && (ShieldAudioComponent->GetSound() != ShieldLowSFX))
	{
		ShieldAudioComponent->SetSound(ShieldLowSFX);
		ShieldAudioComponent->Play();
	}
	if (Shields > 0) {
		PlayShieldFX(true);
		//@TODO there should not be any casting required
		if (ShieldHitFX) UNiagaraFunctionLibrary::SpawnSystemAttached(ShieldHitFX, Cast<ACharacter>(GetOwner())->GetMesh(),
		NAME_None, HitLocation, (Force*-1).Rotation(), EAttachLocation::KeepWorldPosition, true);
	}
	OnHealthUpdate.Broadcast(this);
}

void UHealthComponent::HealthDepleted(float Damage, FVector Force, FVector HitLocation, FName HitBoneName, AController* EventInstigator, AActor* DamageCauser)
{
	Deactivate();
	ShieldRegenTimer.Invalidate();
	ShieldDelayTimerHandle.Invalidate();
	GetWorld()->GetTimerManager().ClearTimer(ShieldDelayTimerHandle);
	Multi_HealthDepleted(Damage, Force, HitLocation, HitBoneName, EventInstigator, DamageCauser);
	OnHealthDepleted.Broadcast(Damage, Force, HitLocation, HitBoneName, EventInstigator, DamageCauser);
}

void UHealthComponent::Multi_HealthDepleted_Implementation(float Damage, FVector Force, FVector HitLocation,
	FName HitBoneName, AController* EventInstigator, AActor* DamageCauser)
{
	ShieldAudioComponent->Deactivate();
	MeshComp->SetOverlayMaterial(nullptr);
}

bool UHealthComponent::IsAlive()
{
	return (Health > 0);
}

void UHealthComponent::OnRep_Health()
{
}

void UHealthComponent::OnRep_Shields()
{
}

float UHealthComponent::GetHealth() const
{
	return Health;
}

float UHealthComponent::SetHealth(float NewHealth)
{
	Health = FMath::Clamp(NewHealth, 0, MaxHealth);
	OnHealthDamaged.Broadcast(Health);
	OnHealthUpdate.Broadcast(this);
	return Health;
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
	Shields = FMath::Clamp(NewShields, 0, MaxShields);
	OnShieldDamaged.Broadcast(Shields);
	return Shields;
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

//Perhaps move this stuff to delegates called inside characters?
void UHealthComponent::BreakShields_Implementation()
{
	PlayShieldFX(false);
	if (ShieldBreakFX) UNiagaraFunctionLibrary::SpawnSystemAttached(ShieldBreakFX, Cast<ACharacter>(GetOwner())->GetMesh(),
		NAME_None, FVector(0,0,0), FRotator(0,0,0),EAttachLocation::SnapToTarget, true);
	if (ShieldBreakSFX)
	{
		// ShieldAudioComponent->SetSound(ShieldBreakSFX);
		UGameplayStatics::PlaySoundAtLocation(GetWorld(), ShieldBreakSFX, GetOwner()->GetActorLocation());
		// ShieldAudioComponent->Play();
	}
}

void UHealthComponent::StartShieldRegen()
{
	if (!IsAlive()) return;
	if (ShieldStartRegenSFX)
	{
		ShieldAudioComponent->SetSound(ShieldStartRegenSFX);
		ShieldAudioComponent->Play(ShieldStartRegenSFX->GetDuration() * Shields/MaxShields);
	}
	GetOwner()->GetWorldTimerManager().SetTimer(ShieldRegenTimer, this, &UHealthComponent::RegenShields, ShieldRegenTickRate, true);
}

void UHealthComponent::RegenShields()
{
	float ShieldRegenAmount = ShieldRegenRatePerSecond*ShieldRegenTickRate;
	SetShields(FMath::Min(MaxShields, Shields + ShieldRegenAmount));
	if (Shields >= MaxShields)
	{
		PlayShieldFX(false);
		StopShieldRegen();
	} else
	{
		PlayShieldFX(true);
	};
	OnHealthUpdate.Broadcast(this);
}

void UHealthComponent::StopShieldRegen()
{
	if (ShieldFinishRegenSFX)
	{
		ShieldAudioComponent->SetSound(ShieldFinishRegenSFX);
		ShieldAudioComponent->Play();
	}
	GetOwner()->GetWorldTimerManager().ClearTimer(ShieldRegenTimer);
}

void UHealthComponent::PlayShieldFX(bool Show)
{
	if (MeshComp)
	{
		if (Show && ShieldMat)
			MeshComp->SetOverlayMaterial(ShieldMat);
		else
			MeshComp->SetOverlayMaterial(nullptr);
	}
	
}

