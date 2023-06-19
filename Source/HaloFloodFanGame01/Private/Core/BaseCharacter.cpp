// Fill out your copyright notice in the Description page of Project Settings.


#include "Core/BaseCharacter.h"

#include "BaseAIController.h"
#include "BaseGrenade.h"
#include "GunBase.h"
#include "HealthComponent.h"
#include "Components/CapsuleComponent.h"
#include "Engine/DamageEvents.h"
#include "Kismet/GameplayStatics.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraComponent.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "GameFramework/PawnMovementComponent.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISense_Damage.h"
#include "Perception/AISense_Touch.h"

// Sets default values
ABaseCharacter::ABaseCharacter()
{
 	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	HealthComponent = CreateDefaultSubobject<UHealthComponent>(TEXT("HealthComp"));
	UE_LOG(LogTemp, Warning, TEXT("Char: %s %f"), *GetActorLabel(), GetHealthComponent()->GetHealth());
	GetCapsuleComponent()->SetNotifyRigidBodyCollision(true);
	
}

// Called when the game starts or when spawned
void ABaseCharacter::BeginPlay()
{
	Super::BeginPlay();
	GetCapsuleComponent()->OnComponentHit.AddDynamic(this, &ABaseCharacter::OnHit);
	if (SpawnWeapon)
	{
		PickupWeapon(Cast<AGunBase>(GetWorld()->SpawnActor(SpawnWeapon)));
	}
	HealthComponent->OnHealthDepleted.AddDynamic(this, &ABaseCharacter::HealthDepleted);
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

float ABaseCharacter::TakePointDamage_Implementation(float Damage, FVector Force, FPointDamageEvent const& PointDamageEvent,
	AController* EventInstigator, AActor* DamageCauser)
{
	if (EventInstigator && Cast<ABaseAIController>(GetController()))
	{
		UAISense_Damage::ReportDamageEvent(GetWorld(), this, EventInstigator->GetPawn(), Damage, PointDamageEvent.HitInfo.Location, PointDamageEvent.HitInfo.Location);
	}
	if (HurtAnim)
	{
		StunAmount = StunAmount + (Force.Length()/50);
		if (StunAmount >= 100  && !GetMesh()->GetAnimInstance()->Montage_IsPlaying(HurtAnim))
		{
			Stun();
		}
	}
	
	if (BloodPFX)
	{
		UNiagaraComponent* BloodNiagaraComponent = UNiagaraFunctionLibrary::SpawnSystemAttached(BloodPFX, GetMesh(), PointDamageEvent.HitInfo.BoneName, PointDamageEvent.HitInfo.Location, PointDamageEvent.HitInfo.Normal.Rotation(), EAttachLocation::KeepWorldPosition, true);
		BloodNiagaraComponent->SetNiagaraVariableActor("Character", this);
	}
	if (BloodSplatterMat)
	{
		UGameplayStatics::SpawnDecalAttached(BloodSplatterMat, FVector(10,10,10), GetMesh(),
		PointDamageEvent.HitInfo.BoneName, PointDamageEvent.HitInfo.Location, PointDamageEvent.HitInfo.Normal.Rotation() + FRotator(-90, 0, FMath::RandRange(-180, 180)), EAttachLocation::KeepWorldPosition, 0);
	}
	if (BloodDecalMaterial)
	{
		float DecalSize = FMath::RandRange(10, 130);
		if (GetHealthComponent()->GetHealth() > 0)
		{
			FHitResult HitResult;
			FCollisionQueryParams QueryParams;
			QueryParams.AddIgnoredActor(this);
			float BoxMax = GetMesh()->Bounds.GetBoxExtrema(1).Z;
			FVector Loc1 = GetMesh()->GetComponentLocation(); Loc1.Z = GetMesh()->GetComponentLocation().Z + (BoxMax/2);
			FVector Loc2 = PointDamageEvent.HitInfo.Location; Loc2.Z = Loc1.Z;
			FVector Line = (Loc1 - Loc2);
			Line.Normalize();
			Line = Line + FVector(0,0,FMath::RandRange(-0.15, 0.15));
			GetWorld()->LineTraceSingleByChannel(HitResult, PointDamageEvent.HitInfo.Location, PointDamageEvent.HitInfo.Location + (Line)*5000,ECollisionChannel::ECC_Visibility, QueryParams);
			if (HitResult.bBlockingHit)
			{
				UGameplayStatics::SpawnDecalAttached(BloodDecalMaterial, FVector(DecalSize,DecalSize,DecalSize), HitResult.GetComponent(), HitResult.BoneName, HitResult.Location, HitResult.Normal.Rotation() + FRotator(-180,0,FMath::RandRange(-180, 180)), EAttachLocation::KeepWorldPosition);
			}
		}
		else
		{
			UGameplayStatics::SpawnDecalAtLocation(GetWorld(), BloodDecalMaterial, FVector(DecalSize, DecalSize, DecalSize), PointDamageEvent.HitInfo.Location + FVector(FMath::RandRange(-50, 50), FMath::RandRange(-50, 50), 0), FRotator(-90,0,FMath::RandRange(-180, 180)));
		}
	}
	return IDamageableInterface::TakePointDamage(Damage, Force, PointDamageEvent, EventInstigator, DamageCauser);
}

// void ABaseCharacter::TakeDamage(float DamageAmount)
// {
// 	IDamageableInterface::TakeDamage(DamageAmount);
// 	//HealthComponent->TakeDamage(DamageAmount, false, false, false);
// }

void ABaseCharacter::HealthDepleted(float Damage, FVector DamageForce, FVector HitLocation, FName HitBoneName)
{
	if (BloodDecalMaterial) UGameplayStatics::SpawnDecalAtLocation(GetWorld(), BloodDecalMaterial, FVector(100, 100, 100), GetActorLocation(), FRotator(-90,0,0));
	if (DeathSound) UGameplayStatics::PlaySoundAtLocation(GetWorld(), DeathSound, GetActorLocation());
	GetMesh()->GetAnimInstance()->Montage_Play(DeathAnim);
	GetCapsuleComponent()->DestroyComponent();
	GetMesh()->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	GetMesh()->SetSimulatePhysics(true);
	GetMesh()->AddImpulseAtLocation(DamageForce, HitLocation, HitBoneName);
	OnKilled.Broadcast();
	if (GetController()) GetController()->Destroy();
	if (EquippedWep)
		DropWeapon();
	
}

void ABaseCharacter::OnHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp,
	FVector NormalImpulse, const FHitResult& Hit)
{
	//GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	UAISense_Touch::ReportTouchEvent(GetWorld(), this, OtherActor, Hit.Location);
	LaunchCharacter(NormalImpulse/100, true, true);
	if (OtherActor)
	{
		float VelocityDifference = FMath::Abs(OtherActor->GetVelocity().Length() - this->GetVelocity().Length());
		float Mass = OtherComp->IsSimulatingPhysics() ? (OtherComp->GetMass()/300) : 1;
		float DamageCalculation = FMath::Pow(VelocityDifference, 1.0f/3.0f) * Mass;
		
		if (DamageCalculation > 5)
		{
			FPointDamageEvent PointDamageEvent;
			TakePointDamage(DamageCalculation, NormalImpulse, PointDamageEvent, nullptr, nullptr);
			//if (BloodDecalMaterial) UGameplayStatics::SpawnDecalAttached(BloodDecalMaterial, FVector(50,50,50), OtherComp, Hit.BoneName, Hit.Location, Hit.Normal.Rotation(), EAttachLocation::KeepWorldPosition);
		}
	}
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
	if (EquippedWep) EquippedWep->StartReload();
}

void ABaseCharacter::PickupWeapon(AGunBase* Gun)
{
	Gun->Mesh->SetSimulatePhysics(false);
	Gun->SetActorEnableCollision(false);
	Gun->AttachToComponent(GetMesh(), FAttachmentTransformRules::SnapToTargetNotIncludingScale, "GripPoint");
	Gun->Pickup(this);
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

void ABaseCharacter::Stun()
{
	StunAmount = 0;
	float StunDuration = HurtAnim->CalculateSequenceLength();
	GetMesh()->GetAnimInstance()->Montage_Play(HurtAnim);
	if (ABaseAIController* AIC = Cast<ABaseAIController>(GetController()))
	{
		AIC->BehaviorTreeComp->PauseLogic(FString("Stunned"));
		AIC->StopMovement();
		AIC->BlackboardComp->SetValueAsBool("IsStunned", true);
		AIC->ClearFocus(EAIFocusPriority::Gameplay);
		GetWorld()->GetTimerManager().SetTimer(StunTimer, this, &ABaseCharacter::Unstun, StunDuration, false);
	}
}

void ABaseCharacter::Unstun()
{
	if (ABaseAIController* AIC = Cast<ABaseAIController>(GetController()))
	{
		AIC->BehaviorTreeComp->ResumeLogic(FString("Unstunned"));
		AIC->BlackboardComp->SetValueAsBool("IsStunned", false);
	}
}


