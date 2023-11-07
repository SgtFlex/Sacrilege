// Fill out your copyright notice in the Description page of Project Settings.


#include "Core/CharacterBase.h"

#include "AIControllerBase.h"
#include "GrenadeBase.h"
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
ACharacterBase::ACharacterBase()
{
 	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	HealthComponent = CreateDefaultSubobject<UHealthComponent>(TEXT("HealthComponent"));
	
	SetReplicates(true);
	SetReplicateMovement(true);
}

// Called when the game starts or when spawned
void ACharacterBase::BeginPlay()
{
	Super::BeginPlay();
	GetCapsuleComponent()->OnComponentHit.AddDynamic(this, &ACharacterBase::OnHit);

	if (SpawnWeaponClass && HasAuthority())
	{
		UE_LOG(LogTemp, Warning, TEXT("Spawned weapon!"));
		PickupWeapon(Cast<AGunBase>(GetWorld()->SpawnActor(SpawnWeaponClass)));
	}

	//UE_LOG(LogTemp, Warning, TEXT("Char: %s %f"), *GetActorLabel(), GetHealthComponent()->GetHealth());
	//if (GetHealthComponent()) UE_LOG(LogTemp, Warning, TEXT("%s's Health component is owned by %s (Should be %s)"), *GetActorLabel(), *GetHealthComponent()->GetOwner()->GetActorLabel(), *GetActorLabel());
	if (GetHealthComponent()) GetHealthComponent()->OnHealthDepleted.AddDynamic(this, &ACharacterBase::OnHealthDepleted);	
}

// Called every frame
void ACharacterBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void ACharacterBase::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ACharacterBase, EquippedWeapon);
}

// Called to bind functionality to input
void ACharacterBase::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}

float ACharacterBase::CustomOnTakeAnyDamage_Implementation(float DamageAmount, FVector Force,
	AController* EventInstigator, AActor* DamageCauser)
{
	return IDamageableInterface::CustomOnTakeAnyDamage(DamageAmount, Force, EventInstigator, DamageCauser);
}

float ACharacterBase::CustomTakePointDamage_Implementation(FPointDamageEvent const& PointDamageEvent, float Force, AController* EventInstigator, AActor* DamageCauser)
{
	float x = IDamageableInterface::CustomTakePointDamage(PointDamageEvent, Force, EventInstigator, DamageCauser);
	if (EventInstigator && Cast<AAIControllerBase>(GetController()))
	{
		UE_LOG(LogTemp, Warning, TEXT("Reported damage event"));
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

void ACharacterBase::OnHealthDepleted_Implementation(float Damage, FVector DamageForce, FVector HitLocation, FName HitBoneName, AController* EventInstigator, AActor* DamageCauser)
{
	GetHealthComponent()->Deactivate();
	for (auto GrenadeStruct : GrenadeInventory)
	{
		for (int i = 0; i < FMath::RandRange(0, GrenadeStruct.GrenadeAmount); i++)
		{
			FActorSpawnParameters ActorSpawnParameters = FActorSpawnParameters();
			ActorSpawnParameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
			FVector Loc = GetActorLocation();
			if (AGrenadeBase* Grenade = Cast<AGrenadeBase>(GetWorld()->SpawnActor(GrenadeStruct.GrenadeClass, &Loc)))
			{
				Grenade->Mesh->SetSimulatePhysics(true);
				Grenade->Mesh->AddImpulse(DamageForce * 0.025);
			}
				
		}
	}
	if (BloodDecalMaterial) UGameplayStatics::SpawnDecalAtLocation(GetWorld(), BloodDecalMaterial, FVector(100, 100, 100), GetActorLocation(), FRotator(-90,0,0));
	if (DeathSound) UGameplayStatics::PlaySoundAtLocation(GetWorld(), DeathSound, GetActorLocation());
	//GetMesh()->OnComponentHit.AddDynamic(this, &ACharacterBase::OnHit);
	GetMesh()->GetAnimInstance()->Montage_Play(DeathAnim);
	GetCapsuleComponent()->DestroyComponent();
	SetRootComponent(GetMesh());
	GetMesh()->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	GetMesh()->SetSimulatePhysics(true);
	GetMesh()->AddImpulseAtLocation(DamageForce, HitLocation, HitBoneName);
	OnKilled.Broadcast(EventInstigator, DamageCauser);
	
	GetWorld()->GetTimerManager().SetTimer(RagdollTimer, this, &ACharacterBase::RagdollSettled, 1);
	if (GetController()) GetController()->Destroy();
	//TestDelegate.Execute(this);
	if (EquippedWeapon)
		DropWeapon();
	
}

void ACharacterBase::OnHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp,
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

UHealthComponent* ACharacterBase::GetHealthComponent()
{
	return HealthComponent;
}

void ACharacterBase::EquipGrenadeType_Implementation(TSubclassOf<AGrenadeBase> Grenade)
{
	// EquippedGrenadeClass = Grenade;
}

void ACharacterBase::Melee_Implementation()
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

void ACharacterBase::ThrowEquippedGrenade_Implementation()
{
	if (GrenadeInventory[CurGrenadeTypeI].GrenadeAmount <= 0) return;
	OnGrenadeInvetoryUpdated.Broadcast(GrenadeInventory);
	FVector EyesLoc;
	FRotator EyesRot;
	GetActorEyesViewPoint(EyesLoc, EyesRot);
	AGrenadeBase* Grenade = Cast<AGrenadeBase>(GetWorld()->SpawnActor(GrenadeInventory[CurGrenadeTypeI].GrenadeClass, &EyesLoc));
	Grenade->SetInstigator(this);
	Grenade->SetArmed(true);
	FVector Force = GetBaseAimRotation().Vector() + FVector(0,0,0.1);
	Grenade->Mesh->AddImpulse(Force*20000);
}

void ACharacterBase::UseEquipment()
{
	UE_LOG(LogTemp, Warning, TEXT("Used Equipment"));
}

void ACharacterBase::PrimaryAttack_Pull()
{
	if (EquippedWeapon)
		EquippedWeapon->PullTrigger();
}

void ACharacterBase::PrimaryAttack_Release()
{
	if (EquippedWeapon)
		EquippedWeapon->ReleaseTrigger();
}

void ACharacterBase::ReloadInput()
{
	if (EquippedWeapon) EquippedWeapon->StartReload();
}

void ACharacterBase::PickupWeapon(AGunBase* Gun)
{
	Server_PickupWeapon(Gun);
}

void ACharacterBase::Server_PickupWeapon_Implementation(AGunBase* Gun)
{
	Multi_PickupWeapon(Gun);
}

bool ACharacterBase::Server_PickupWeapon_Validate(AGunBase* Gun)
{
	return true;
}

void ACharacterBase::Multi_PickupWeapon_Implementation(AGunBase* Gun)
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

void ACharacterBase::DropWeapon()
{
	EquippedWeapon->ReleaseTrigger();
	EquippedWeapon->DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
	EquippedWeapon->SetActorEnableCollision(true);
	EquippedWeapon->Mesh->SetSimulatePhysics(true);
	EquippedWeapon->Mesh->AddImpulse(GetControlRotation().Vector() * 300, NAME_None, true);
	EquippedWeapon->Drop();
	EquippedWeapon = nullptr;
}

void ACharacterBase::RagdollSettled()
{
	TArray<FName> BoneNames;
	GetMesh()->GetBoneNames(BoneNames);
	if (GetMesh()->GetPhysicsLinearVelocity(BoneNames[1]).Length() <= 1)
	{
		GetMesh()->PutAllRigidBodiesToSleep();
		GetMesh()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Ignore);
	} else
	{
		GetWorld()->GetTimerManager().SetTimer(RagdollTimer, this, &ACharacterBase::RagdollSettled, 1);
	}
}

void ACharacterBase::Stun()
{
	StunAmount = 0;
	float StunDuration = HurtAnim->CalculateSequenceLength();
	GetMesh()->GetAnimInstance()->Montage_Play(HurtAnim);
	if (AAIControllerBase* AIC = Cast<AAIControllerBase>(GetController()))
	{
		AIC->BehaviorTreeComp->PauseLogic(FString("Stunned"));
		AIC->StopMovement();
		AIC->BlackboardComp->SetValueAsBool("IsStunned", true);
		AIC->ClearFocus(EAIFocusPriority::Gameplay);
		GetWorld()->GetTimerManager().SetTimer(StunTimer, this, &ACharacterBase::Unstun, StunDuration, false);
	}
}

void ACharacterBase::Unstun()
{
	if (AAIControllerBase* AIC = Cast<AAIControllerBase>(GetController()))
	{
		AIC->BehaviorTreeComp->ResumeLogic(FString("Unstunned"));
		AIC->BlackboardComp->SetValueAsBool("IsStunned", false);
	}
}


