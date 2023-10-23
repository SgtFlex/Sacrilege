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
#include "TimerManager.h"
#include "Animation/AnimInstance.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "GameFramework/PawnMovementComponent.h"
#include "Net/UnrealNetwork.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISense_Damage.h"
#include "Perception/AISense_Touch.h"

// Sets default values
ABaseCharacter::ABaseCharacter()
{
 	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	HealthComponent = CreateDefaultSubobject<UHealthComponent>(TEXT("HealthComponent"));
	
	GetCapsuleComponent()->SetNotifyRigidBodyCollision(true);

	SetReplicates(true);
	SetReplicateMovement(true);
}

// Called when the game starts or when spawned
void ABaseCharacter::BeginPlay()
{
	Super::BeginPlay();
	GetCapsuleComponent()->OnComponentHit.AddDynamic(this, &ABaseCharacter::OnHit);

	if (SpawnWeaponClass && HasAuthority())
	{
		UE_LOG(LogTemp, Warning, TEXT("Spawned weapon!"));
		PickupWeapon(Cast<AGunBase>(GetWorld()->SpawnActor(SpawnWeaponClass)));
	}

	//UE_LOG(LogTemp, Warning, TEXT("Char: %s %f"), *GetActorLabel(), GetHealthComponent()->GetHealth());
	//if (GetHealthComponent()) UE_LOG(LogTemp, Warning, TEXT("%s's Health component is owned by %s (Should be %s)"), *GetActorLabel(), *GetHealthComponent()->GetOwner()->GetActorLabel(), *GetActorLabel());
	if (GetHealthComponent()) GetHealthComponent()->OnHealthDepleted.AddDynamic(this, &ABaseCharacter::OnHealthDepleted);	
}

// Called every frame
void ABaseCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void ABaseCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ABaseCharacter, EquippedWeapon);
}

// Called to bind functionality to input
void ABaseCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}

float ABaseCharacter::CustomOnTakeAnyDamage_Implementation(float DamageAmount, FVector Force,
	AController* EventInstigator, AActor* DamageCauser)
{
	return IDamageableInterface::CustomOnTakeAnyDamage(DamageAmount, Force, EventInstigator, DamageCauser);
}

float ABaseCharacter::CustomTakePointDamage_Implementation(FPointDamageEvent const& PointDamageEvent, float Force, AController* EventInstigator, AActor* DamageCauser)
{
	float x = IDamageableInterface::CustomTakePointDamage(PointDamageEvent, Force, EventInstigator, DamageCauser);
	if (EventInstigator && Cast<ABaseAIController>(GetController()))
	{
		UAISense_Damage::ReportDamageEvent(GetWorld(), this, EventInstigator->GetPawn(), PointDamageEvent.Damage, PointDamageEvent.HitInfo.Location, PointDamageEvent.HitInfo.Location);
	}
	
	if (GetHealthComponent()->GetShields() <= 0)
	{
		if (HurtAnim)
		{
			StunAmount = StunAmount + (Force/50);
			if (StunAmount >= 100  && !GetMesh()->GetAnimInstance()->Montage_IsPlaying(HurtAnim))
			{
				Stun();
			}
		}
		if (BloodPFX)
		{
			UNiagaraComponent* BloodNiagaraComponent = UNiagaraFunctionLibrary::SpawnSystemAttached(BloodPFX, GetMesh(), PointDamageEvent.HitInfo.BoneName, PointDamageEvent.HitInfo.ImpactPoint, PointDamageEvent.HitInfo.Normal.Rotation(), EAttachLocation::KeepWorldPosition, true);
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
				GetWorld()->LineTraceSingleByChannel(HitResult, PointDamageEvent.HitInfo.Location, PointDamageEvent.HitInfo.Location + (PointDamageEvent.ShotDirection * 4000),ECollisionChannel::ECC_Visibility, QueryParams);
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
	}
	
	return x;
}



// float ABaseCharacter::CustomTakeDamage_Implementation(float DamageAmount, FVector Force, FDamageEvent const& DamageEvent,
// 	AController* EventInstigator, AActor* DamageCauser)
// {
// 	return IDamageableInterface::CustomTakeDamage(DamageAmount, Force, DamageEvent, EventInstigator, DamageCauser);
// }

void ABaseCharacter::OnHealthDepleted_Implementation(float Damage, FVector DamageForce, FVector HitLocation, FName HitBoneName, AController* EventInstigator, AActor* DamageCauser)
{
	if (BloodDecalMaterial) UGameplayStatics::SpawnDecalAtLocation(GetWorld(), BloodDecalMaterial, FVector(100, 100, 100), GetActorLocation(), FRotator(-90,0,0));
	if (DeathSound) UGameplayStatics::PlaySoundAtLocation(GetWorld(), DeathSound, GetActorLocation());
	GetMesh()->SetNotifyRigidBodyCollision(true);
	GetMesh()->OnComponentHit.AddDynamic(this, &ABaseCharacter::OnHit);
	GetMesh()->GetAnimInstance()->Montage_Play(DeathAnim);
	GetCapsuleComponent()->DestroyComponent();
	GetMesh()->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	GetMesh()->SetSimulatePhysics(true);
	GetMesh()->AddImpulseAtLocation(DamageForce, HitLocation, HitBoneName);
	OnKilled.Broadcast(EventInstigator, DamageCauser);
	
	GetWorld()->GetTimerManager().SetTimer(RagdollTimer, this, &ABaseCharacter::RagdollSettled, 1);
	if (GetController()) GetController()->Destroy();
	//TestDelegate.Execute(this);
	if (EquippedWeapon)
		DropWeapon();
	
}

void ABaseCharacter::OnHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp,
	FVector NormalImpulse, const FHitResult& Hit)
{
	LaunchCharacter(NormalImpulse/100, true, true);
	float DamageCalculation;
	if (OtherActor && OtherComp)
	{
		UAISense_Touch::ReportTouchEvent(GetWorld(), this, OtherActor, Hit.Location);
		float VelocityDifference = FMath::Abs(OtherActor->GetVelocity().Length() - this->GetVelocity().Length());
		float Mass = OtherComp->IsSimulatingPhysics() ? (OtherComp->GetMass()/300) : 1;
		DamageCalculation = FMath::Pow(VelocityDifference, 1.0f/3.0f) * Mass;
	} else {
		DamageCalculation = FMath::Pow(this->GetVelocity().Length(), 1.0f/5.0f);
	}
	if (DamageCalculation > 5)
	{
		//FPointDamageEvent PointDamageEvent = FPointDamageEvent(DamageCalculation, Hit, FVector(0,0,0), UDamageType::StaticClass());
		FDamageEvent DamageEvent = FDamageEvent(UDamageType::StaticClass());
		//TakePointDamage(PointDamageEvent, NormalImpulse, nullptr, nullptr);
		
		CustomTakeDamage(DamageCalculation, NormalImpulse, DamageEvent, nullptr, nullptr);
		float DecalSize = 100;
		UGameplayStatics::SpawnDecalAtLocation(GetWorld(), BloodDecalMaterial, FVector(DecalSize, DecalSize, DecalSize), GetMesh()->GetComponentLocation() + FVector(FMath::RandRange(-50, 50), FMath::RandRange(-50, 50), 0), FRotator(-90,0,FMath::RandRange(-180, 180)));
	}
}

UHealthComponent* ABaseCharacter::GetHealthComponent()
{
	return HealthComponent;
}

void ABaseCharacter::EquipGrenadeType_Implementation(TSubclassOf<ABaseGrenade> Grenade)
{
	// EquippedGrenadeClass = Grenade;
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
	if (GrenadeInventory[CurGrenadeTypeI].GrenadeAmount <= 0) return;
	OnGrenadeInvetoryUpdated.Broadcast(GrenadeInventory);
	FVector EyesLoc;
	FRotator EyesRot;
	GetActorEyesViewPoint(EyesLoc, EyesRot);
	ABaseGrenade* Grenade = Cast<ABaseGrenade>(GetWorld()->SpawnActor(GrenadeInventory[CurGrenadeTypeI].GrenadeClass, &EyesLoc));
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
	if (EquippedWeapon)
		EquippedWeapon->PullTrigger();
}

void ABaseCharacter::PrimaryAttack_Release()
{
	if (EquippedWeapon)
		EquippedWeapon->ReleaseTrigger();
}

void ABaseCharacter::ReloadInput()
{
	if (EquippedWeapon) EquippedWeapon->StartReload();
}

void ABaseCharacter::PickupWeapon(AGunBase* Gun)
{
	Server_PickupWeapon(Gun);
}

void ABaseCharacter::Server_PickupWeapon_Implementation(AGunBase* Gun)
{
	Multi_PickupWeapon(Gun);
}

bool ABaseCharacter::Server_PickupWeapon_Validate(AGunBase* Gun)
{
	return true;
}

void ABaseCharacter::Multi_PickupWeapon_Implementation(AGunBase* Gun)
{
	Gun->Mesh->SetSimulatePhysics(false);
	Gun->SetActorEnableCollision(false);
	Gun->AttachToComponent(GetMesh(), FAttachmentTransformRules::SnapToTargetNotIncludingScale, "GripPoint");
	Gun->Pickup(this);
	if (!EquippedWeapon)
	{
		EquippedWeapon = Gun;
	} else if (!HolsteredWeapon)
	{
		HolsteredWeapon = Gun;
		Gun->SetActorHiddenInGame(true);
	} else
	{
		DropWeapon();
		EquippedWeapon = Gun;
	}
}

void ABaseCharacter::DropWeapon()
{
	UE_LOG(LogTemp, Warning, TEXT("Dropped weapon"));
	EquippedWeapon->ReleaseTrigger();
	EquippedWeapon->Drop();
	EquippedWeapon->DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
	//EquippedWep->SetActorLocation(this->Mesh1P->GetSocketLocation("GripPoint"));
	EquippedWeapon->SetActorEnableCollision(true);
	EquippedWeapon->Mesh->SetSimulatePhysics(true);
	//EquippedWep->Mesh->AddImpulse(FirstPersonCameraComponent->GetForwardVector()*5000);
	EquippedWeapon = nullptr;
}

void ABaseCharacter::RagdollSettled()
{
	TArray<FName> BoneNames;
	GetMesh()->GetBoneNames(BoneNames);
	if (GetMesh()->GetPhysicsLinearVelocity(BoneNames[1]).Length() <= 1)
	{
		GetMesh()->PutAllRigidBodiesToSleep();
		GetMesh()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Ignore);
	} else
	{
		GetWorld()->GetTimerManager().SetTimer(RagdollTimer, this, &ABaseCharacter::RagdollSettled, 1);
	}
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


