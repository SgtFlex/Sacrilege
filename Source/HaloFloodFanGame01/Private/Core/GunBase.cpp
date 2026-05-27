// Fill out your copyright notice in the Description page of Project Settings.

#include "Core/GunBase.h"

#include "Bullet.h"
#include "Components/BulletFiringComponent.h"
#include "NiagaraComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "Core/PlayerControllerBase.h"
#include "PlayerCharacter.h"
#include "Core/ProjectileBase.h"
#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"
#include "Perception/AISense_Hearing.h"


// Sets default values
AGunBase::AGunBase()
{
	BulletFiringComponent = CreateDefaultSubobject<UBulletFiringComponent>("BulletFiringComponent");
	BulletFiringComponent->SetupAttachment(Mesh, "Muzzle");
	CurMagazine = MaxMagazine;
	CurReserve = MaxReserve;
	InteractIcon = WeaponIcon;
	
}

void AGunBase::SecondaryFire_Start_Implementation()
{
	Super::SecondaryFire_Start_Implementation();

	if (ScopeActive)
	{
		ScopeOut();
	} else if (!ScopeActive && ScopeFOV != 0.0f)
	{
		ScopeIn();
	}
}

void AGunBase::PrimaryFire_Start_Implementation()
{
	Super::PrimaryFire_Start_Implementation();
	PullTrigger();
}

void AGunBase::PrimaryFire_End_Implementation()
{
	Super::PrimaryFire_End_Implementation();
	ReleaseTrigger();
}

void AGunBase::Reload_Implementation()
{
	Super::Reload_Implementation();
	StartReload();
}

void AGunBase::StartReload_Implementation()
{
	if (bReloading || CurReserve <= 0 || CurMagazine == MaxMagazine) return;
	ScopeOut();
	bReloading = true;
	//if (BurstAmount > 0) GetWorldTimerManager().ClearTimer(FireHandle);
	GetWorld()->GetTimerManager().SetTimer(ReloadTimer, this, &AGunBase::FinishReload, ReloadSpeed, false);
	Multi_StartReload();
}

void AGunBase::Server_StartReload_Implementation()
{
	//Multi_StartReload();
}

void AGunBase::Multi_StartReload_Implementation()
{
	if (CharacterOwner)
		CharacterOwner->GetMesh1P()->GetAnimInstance()->Montage_Play(ReloadAnimation1P,   ReloadAnimation1P->GetPlayLength() / ReloadSpeed);
	if (ReloadSound) UGameplayStatics::SpawnSoundAttached(ReloadSound, GetRootComponent());
}

void AGunBase::PullTrigger_Implementation()
{

	if (GetWorldTimerManager().TimerExists(BurstRetriggerHandle))
		return;
	BulletsFired = 0;
	GetWorldTimerManager().SetTimer(FireHandle, this, &AGunBase::Fire, 60/FireRate, true, 0);
	GetWorldTimerManager().SetTimer(BurstRetriggerHandle, BurstRetriggerDelay, false);
}
//
// void AGunBase::Server_PullTrigger_Implementation()
// {
// 	Multi_PullTrigger();
// }
//
// void AGunBase::Multi_PullTrigger_Implementation()
// {
// 	if (GetWorldTimerManager().TimerExists(BurstRetriggerHandle) || CurMagazine <= 0 || bReloading)
// 		return;
// 	BulletsFired = 0;
// 	GetWorldTimerManager().SetTimer(FireHandle, this, &AGunBase::Fire, 60/FireRate, true, 0);
// 	GetWorldTimerManager().SetTimer(BurstRetriggerHandle, BurstRetriggerDelay, false);
// }

void AGunBase::ReleaseTrigger_Implementation()
{
	if (BulletsFired >= BurstAmount)
		GetWorldTimerManager().ClearTimer(FireHandle);
}
//
// void AGunBase::Server_ReleaseTrigger_Implementation()
// {
// 	Multi_ReleaseTrigger();
// }
//
// void AGunBase::Multi_ReleaseTrigger_Implementation()
// {
// 	if (BulletsFired >= BurstAmount)
// 		GetWorldTimerManager().ClearTimer(FireHandle);
// }



// void AGunBase::OnInteract_Implementation(ACharacterBase* Character)
// {
// 	IInteractableInterface::OnInteract_Implementation(Character);
// 	Character->PickupWeapon(this);
// }

// void AGunBase::GetInteractInfo_Implementation(FText& Text, UTexture2D*& Icon, ACharacterBase* InteractingCharacter)
// {
// 	IInteractableInterface::GetInteractInfo_Implementation(Text, Icon, InteractingCharacter);
//
// 	Text = InteractText;
// 	Icon = InteractIcon;
// 	
// }

void AGunBase::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(AGunBase, CurMagazine);
	DOREPLIFETIME(AGunBase, CurReserve);
	DOREPLIFETIME(AGunBase, bFiring);
}

bool AGunBase::CanFire()
{
	return !(bReloading || CurMagazine <= 0);
}

AActor* AGunBase::SpawnProjectileInDirection_Implementation(TSubclassOf<AActor> ProjToSpawn, FVector Direction)
{
	FVector AimLocation;
	FVector AimDirection;
	GetAim(AimLocation, AimDirection);

	AController* EventInstigator = nullptr;
	if (CharacterOwner)
	{
		EventInstigator = CharacterOwner->GetController();
	}
	//TArray<AActor*> ActorsToIgnore;
	return BulletFiringComponent->FireProjectile(TSubclassOf<AProjectileBase>(ProjToSpawn), Direction, this, EventInstigator, ActorsToIgnore);
}

// void AGunBase::GetAim(FVector& AimLocation, FVector& AimDirection)
// {
// 	if (CharacterOwner)
// 	{
// 		FRotator Rot;
// 		CharacterOwner->GetActorEyesViewPoint(AimLocation, Rot);
// 		
// 		AimDirection = CharacterOwner->GetBaseAimRotation().Vector();
// 	} else
// 	{
// 		AimLocation = BulletFiringComponent->GetComponentLocation();
// 		AimDirection = GetActorRotation().Vector();
// 	}
// 	
// }


void AGunBase::UpdateMagazineElements()
{
	OnAmmoUpdated.Broadcast();
}

void AGunBase::PlayFireFX_Implementation()
{
	if (CharacterOwner)
	{
		CharacterOwner->GetMesh()->GetAnimInstance()->Montage_Play(CharacterOwner->FiringAnim);
		if (FireAnimation1P) CharacterOwner->GetMesh1P()->GetAnimInstance()->Montage_Play(FireAnimation1P);
		if (CharacterOwner->PlayerController) CharacterOwner->PlayerController->ClientPlayForceFeedback(FireFeedback);
	}
}

void AGunBase::SpawnTrailFX_Implementation(FHitResult Hit)
{
	K2_SpawnTrailFX(Hit);
	if (Mesh->DoesSocketExist("Muzzle") && TrailPFX)
	{
		FVector TrailEnd = (Hit.bBlockingHit) ? Hit.ImpactPoint : Hit.TraceEnd;
		UNiagaraComponent* TrailPFXComponent = UNiagaraFunctionLibrary::SpawnSystemAttached(TrailPFX, Mesh, "Muzzle", FVector(0,0,0), FRotator(0,0,0), EAttachLocation::SnapToTarget, true);
		if (TrailPFX)
			TrailPFXComponent->SetVectorParameter("BeamEnd", TrailEnd);
	}
	if (Hit.bBlockingHit)
	{
		//if (HitSound) UGameplayStatics::PlaySoundAtLocation(GetWorld(), HitSound, Hit.Location);
		if (const TSubclassOf<AActor>* ImpactFXClass = BulletFiringComponent->BulletInfo.GetDefaultObject()->SurfaceResponsesFX.Find(Hit.PhysMaterial->SurfaceType))
		{
			const FVector Location = Hit.ImpactPoint;
			const FRotator Rotation = Hit.Normal.Rotation() + FRotator(-90, 0, 0);
			if (AActor* Decal = GetWorld()->SpawnActor(*ImpactFXClass, &Location, &Rotation))
				Decal->AttachToComponent(Hit.GetComponent(), FAttachmentTransformRules::KeepWorldTransform);
		}
		// else
		// {
		// 	if (ImpactDecal)
		// 	{
		// 		FVector Location = Hit.ImpactPoint;
		// 		FRotator Rotation = Hit.Normal.Rotation() + FRotator(-90, 0, 0);
		// 		AActor* Decal = GetWorld()->SpawnActor(ImpactDecal, &Location, &Rotation);
		// 		if (Decal)
		// 			Decal->AttachToComponent(Hit.GetComponent(), FAttachmentTransformRules::KeepWorldTransform);
		// 		//UGameplayStatics::SpawnDecalAttached(GetWorld(), FVector(10,10,10), ) //Perhaps optimize this in the future
		// 	}
		// }
	}
}

void AGunBase::SpawnBullet_Implementation()
{
	UAISense_Hearing::ReportNoiseEvent(GetWorld(), GetActorLocation(), 1.0f, GetOwner(), 0.0f);
	
	AController* EventInstigator = nullptr;
	PlayFireFX();
	// if (ACharacterBase* OwningChar = Cast<ACharacterBase>(GetOwner()))
	// {
	// 	OwningChar->GetMesh()->GetAnimInstance()->Montage_Play(OwningChar->FiringAnim);
	// }
	// if (ACharacterBase* PlayerChar = Cast<ACharacterBase>(GetOwner()))
	// {
	// 	if (FireAnimation1P) PlayerChar->GetMesh1P()->GetAnimInstance()->Montage_Play(FireAnimation1P);
	// 	if (APlayerController* PC = Cast<APlayerController>(PlayerChar->GetController())) PC->ClientPlayForceFeedback(FireFeedback);
	// }
	
	for (int i = 0; i < MultiShot; ++i)
	{
		if (ProjectileClass)
		{
			SpawnProjectile(ProjectileClass);
		} else
		{
			FHitResult Hit;
			FRotator EyeRotation;
			FVector AimLocation;
			FVector AimDirection;
			GetAim(AimLocation, AimDirection);
			if (CharacterOwner)
			{
				EventInstigator = CharacterOwner->GetController();
			}
			EyeRotation = AimDirection.Rotation() + FRotator(FMath::RandRange(-VerticalSpread, VerticalSpread), FMath::RandRange(-HorizontalSpread, HorizontalSpread),0);
			
			ActorsToIgnore.Add(this);
			ActorsToIgnore.Add(GetOwner());
			
			BulletFiringComponent->FireBullet(Hit, AimLocation,EyeRotation.Vector(), ActorsToIgnore, this, EventInstigator);
			SpawnTrailFX(Hit);

		}
	}
	BulletsFired++;
	if (BulletsFired==BurstAmount)
	{
		ReleaseTrigger();
	}
	SpawnMuzzleFX();
}

AActor* AGunBase::SpawnProjectile_Implementation(TSubclassOf<AActor> ProjToSpawn)
{
	FVector AimLocation;
	FVector AimDirection;
	GetAim(AimLocation, AimDirection);

	AController* EventInstigator = nullptr;
	if (CharacterOwner)
	{
		EventInstigator = CharacterOwner->GetController();
	}
	//TArray<AActor*> ActorsToIgnore;
	return BulletFiringComponent->FireProjectile(TSubclassOf<AProjectileBase>(ProjToSpawn), AimDirection, this, EventInstigator, ActorsToIgnore);
	//return GetWorld()->SpawnActor(ProjToSpawn, &Location, &Rotation, ActorSpawnParameters);
}

void AGunBase::SpawnMuzzleFX_Implementation()
{
	if (CharacterOwner)
		if (FiringCameraShake && CharacterOwner->IsPlayerControlled() && CharacterOwner->IsLocallyControlled())
			Cast<APlayerController>(CharacterOwner->GetController())->PlayerCameraManager->StartCameraShake(FiringCameraShake, 1, ECameraShakePlaySpace::CameraLocal);
	if (FiringSound)
		UGameplayStatics::SpawnSoundAttached(FiringSound, GetRootComponent());
	if (Mesh->DoesSocketExist("Muzzle") && MuzzlePFX && !ScopeActive)
		UNiagaraFunctionLibrary::SpawnSystemAttached(MuzzlePFX, Mesh, "Muzzle", FVector(0,0,0), FRotator(0,0,0), EAttachLocation::SnapToTarget, true);
}

void AGunBase::Fire_Implementation()
{
	if (!CanFire())
	{
		//ReleaseTrigger();
		return;
	}
	CurMagazine--;
	
	
	SpawnBullet();
	OnAmmoUpdated.Broadcast();
	OnFire.Broadcast();
	if (CurMagazine <= 0)
	{
		StartReload();
	}
}

void AGunBase::FinishReload_Implementation()
{
	bReloading = false;
	int32 AmountNeed = MaxMagazine - CurMagazine; //32 - 27 gives 5 for example
	int32 AmountGrabbed = FMath::Min(AmountNeed, CurReserve);
	CurMagazine = CurMagazine + AmountGrabbed;
	CurReserve = CurReserve - AmountGrabbed;
	OnAmmoUpdated.Broadcast();
	OnReload.Broadcast();
}