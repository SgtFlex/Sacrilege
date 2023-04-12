// Fill out your copyright notice in the Description page of Project Settings.


#include "Core/BaseCharacter.h"

#include "BaseGrenade.h"
#include "GunBase.h"
#include "HealthComponent.h"
#include "Components/CapsuleComponent.h"
#include "Engine/DamageEvents.h"
#include "Kismet/GameplayStatics.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraComponent.h"

// Sets default values
ABaseCharacter::ABaseCharacter()
{
 	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	HealthComponent = CreateDefaultSubobject<UHealthComponent>(TEXT("HealthComp"));
	
}

// Called when the game starts or when spawned
void ABaseCharacter::BeginPlay()
{
	Super::BeginPlay();

	if (SpawnWeapon)
	{
		AGunBase* Weapon = Cast<AGunBase>(GetWorld()->SpawnActor(SpawnWeapon));
		PickupWeapon(Weapon);
	}
}

// Called every frame
void ABaseCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

// Called to bind functionality to input
void ABaseCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}

float ABaseCharacter::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator,
	AActor* DamageCauser)
{
	HealthComponent->TakeDamage(DamageAmount);
	return Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);
}

float ABaseCharacter::TakePointDamage(float Damage, FVector Force, FPointDamageEvent const& PointDamageEvent,
	AController* EventInstigator, AActor* DamageCauser)
{
	HealthComponent->TakeDamage(PointDamageEvent.Damage, Force, PointDamageEvent.HitInfo.Location, PointDamageEvent.HitInfo.BoneName, EventInstigator, DamageCauser);
	if (BloodPFX) UNiagaraFunctionLibrary::SpawnSystemAtLocation(GetWorld(), BloodPFX, PointDamageEvent.HitInfo.Location, PointDamageEvent.HitInfo.Normal.Rotation());
	return Super::InternalTakePointDamage(Damage, PointDamageEvent, EventInstigator, DamageCauser);
}

float ABaseCharacter::TakeRadialDamage(float Damage, FVector Force, FRadialDamageEvent const& RadialDamageEvent,
	AController* EventInstigator, AActor* DamageCauser)
{
	HealthComponent->TakeDamage(Damage, Force);
	return Super::InternalTakeRadialDamage(Damage, RadialDamageEvent, EventInstigator, DamageCauser);
}

// void ABaseCharacter::TakeDamage(float DamageAmount)
// {
// 	IDamageableInterface::TakeDamage(DamageAmount);
// 	//HealthComponent->TakeDamage(DamageAmount, false, false, false);
// }

void ABaseCharacter::HealthDepleted(float Damage, FVector DamageForce, FVector HitLocation, FName HitBoneName)
{
	if (BloodDecalMaterial) UGameplayStatics::SpawnDecalAtLocation(GetWorld(), BloodDecalMaterial, FVector(10, 10, 10), GetActorLocation(), FRotator(90,0,0));
	GetCapsuleComponent()->DestroyComponent();
	GetMesh()->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	GetMesh()->SetSimulatePhysics(true);
	GetMesh()->AddImpulseAtLocation(DamageForce, HitLocation, HitBoneName);
	if (GetController()) GetController()->UnPossess();
	if (EquippedWep)
		DropWeapon();
	
}

UHealthComponent* ABaseCharacter::GetHealthComponent()
{
	return HealthComponent;
}

void ABaseCharacter::EquipGrenadeType_Implementation(TSubclassOf<ABaseGrenade> Grenade)
{
	EquippedGrenadeClass = Grenade;
}

void ABaseCharacter::Melee_Implementation()
{
	//GetMesh()->GetAnimInstance()->Montage_Play(MeleeAnim);
	UE_LOG(LogTemp, Warning, TEXT("Melee"));
	FCollisionShape BoxShape = FCollisionShape::MakeBox(FVector(50, 50, 50));
	TArray<FHitResult> SweepResult;
	FCollisionQueryParams CollisionParams;
	CollisionParams.AddIgnoredActor(this);
	GetWorld()->SweepMultiByChannel(SweepResult, GetActorLocation(), GetActorLocation() + GetActorForwardVector()*100, FQuat(0,0,0,0), ECollisionChannel::ECC_Pawn, BoxShape, CollisionParams);
	for (auto Result : SweepResult)
	{
		FDamageEvent DamageEvent;
		Result.GetActor()->TakeDamage(MeleeDamage, DamageEvent, nullptr, this);
		UPrimitiveComponent* HitComp = Result.GetComponent();
		
		if (HitComp->IsSimulatingPhysics())
		{
			FVector ForceVector = (HitComp->GetComponentLocation() - GetActorLocation());
			ForceVector.Normalize();
			HitComp->AddImpulse(ForceVector*MeleeForce);
		}
	}
}

void ABaseCharacter::ThrowEquippedGrenade_Implementation()
{
	if (!EquippedGrenadeClass) return;
	FVector EyesLoc;
	FRotator EyesRot;
	GetActorEyesViewPoint(EyesLoc, EyesRot);
	ABaseGrenade* Grenade = Cast<ABaseGrenade>(GetWorld()->SpawnActor(EquippedGrenadeClass, &EyesLoc));
	Grenade->SetArmed(true);
	FVector Force = GetBaseAimRotation().Vector() + FVector(0,0,0.1);
	Grenade->Mesh->AddImpulse(Force*20000);
}

void ABaseCharacter::UseEquipment()
{
	UE_LOG(LogTemp, Warning, TEXT("Used Equipment"));
}

void ABaseCharacter::PrimaryAttack_Pull()
{
	if (EquippedWep)
		EquippedWep->PullTrigger();
}

void ABaseCharacter::PrimaryAttack_Release()
{
	if (EquippedWep)
		EquippedWep->ReleaseTrigger();
}

void ABaseCharacter::ReloadInput()
{
	if (EquippedWep) EquippedWep->Reload();
}

void ABaseCharacter::PickupWeapon(AGunBase* Gun)
{
	if (!EquippedWep)
	{
		EquippedWep = Gun;
	} else if (!HolsteredWeapon)
	{
		HolsteredWeapon = Gun;
		Gun->SetActorHiddenInGame(true);
	} else
	{
		DropWeapon();
		EquippedWep = Gun;
	}
	Gun->Mesh->SetSimulatePhysics(false);
	Gun->SetActorEnableCollision(false);
	Gun->AttachToComponent(GetMesh(), FAttachmentTransformRules::SnapToTargetNotIncludingScale, "GripPoint");
	Gun->Pickup(this);
}

void ABaseCharacter::DropWeapon()
{
	EquippedWep->ReleaseTrigger();
	EquippedWep->Drop();
	EquippedWep->DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
	//EquippedWep->SetActorLocation(this->Mesh1P->GetSocketLocation("GripPoint"));
	EquippedWep->SetActorEnableCollision(true);
	EquippedWep->Mesh->SetSimulatePhysics(true);
	//EquippedWep->Mesh->AddImpulse(FirstPersonCameraComponent->GetForwardVector()*5000);
	EquippedWep = nullptr;
}


