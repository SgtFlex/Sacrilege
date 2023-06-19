// Fill out your copyright notice in the Description page of Project Settings.

#include "GunBase.h"
#include "HaloHUDWidget.h"
#include "NiagaraComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "VectorTypes.h"
#include "Components/Image.h"
#include "Engine/DamageEvents.h"
#include "HaloFloodFanGame01/HaloFloodFanGame01Character.h"
#include "Perception/AISense_Hearing.h"


// Sets default values
AGunBase::AGunBase()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	
	Mesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("CharacterMesh1P"));
	Mesh->SetSimulatePhysics(true);
}

// Called when the game starts or when spawned
void AGunBase::BeginPlay()
{
	Super::BeginPlay();

	CurMagazine = MaxMagazine;
	CurReserve = MaxReserve;
	
}

// Called every frame
void AGunBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void AGunBase::Pickup(ABaseCharacter* Char)
{
	SetOwner(Char);
	if (AHaloFloodFanGame01Character* PlayerChar = Cast<AHaloFloodFanGame01Character>(Char))
	{
		PlayerHUD = PlayerChar->GetPlayerHUD();
	}
}

void AGunBase::Equip()
{
	PlayerHUD->SetCrosshairTexture(CrosshairTexture);
}

void AGunBase::Drop()
{
	SetOwner(nullptr);
	PlayerHUD = nullptr;
}

void AGunBase::StartReload()
{
	if (bReloading || CurReserve <= 0 || CurMagazine == MaxMagazine) return;
	bReloading = true;
	GetWorld()->GetTimerManager().SetTimer(ReloadTimer, this, &AGunBase::FinishReload, ReloadSpeed, false);
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

void AGunBase::ReleaseTrigger_Implementation()
{
	if (BulletsFired >= BurstAmount)
		GetWorldTimerManager().ClearTimer(FireHandle);
}

void AGunBase::OnInteract_Implementation(AHaloFloodFanGame01Character* Character)
{
	IInteractableInterface::OnInteract_Implementation(Character);
	Character->PickupWeapon(this);
}

void AGunBase::GetInteractInfo_Implementation(FText& Text, UTexture2D*& Icon)
{
	IInteractableInterface::GetInteractInfo_Implementation(Text, Icon);

	Text = InteractText;
	Icon = InteractIcon;
}

void AGunBase::Fire_Implementation()
{
	if (bReloading || CurMagazine <= 0)
	{
		ReleaseTrigger();
		return;
	}
	CurMagazine--;
	ABaseCharacter* OwningChar = Cast<ABaseCharacter>(GetOwner());
	if (!OwningChar) return;
	UAISense_Hearing::ReportNoiseEvent(GetWorld(), GetActorLocation(), 1.0f, OwningChar);
	if (OwningChar->FiringAnim) OwningChar->GetMesh()->GetAnimInstance()->Montage_Play(OwningChar->FiringAnim);
	if (AHaloFloodFanGame01Character* Char = Cast<AHaloFloodFanGame01Character>(GetOwner()))
	{
		if (FiringCameraShake)
			Cast<APlayerController>(Char->GetController())->PlayerCameraManager->StartCameraShake(FiringCameraShake, 1, ECameraShakePlaySpace::CameraLocal);
	}
	if (FiringSound) UGameplayStatics::SpawnSoundAttached(FiringSound, GetRootComponent());
	if (Mesh->DoesSocketExist("Muzzle") && MuzzlePFX)
	{
		UNiagaraComponent* BulletPFX = UNiagaraFunctionLibrary::SpawnSystemAttached(MuzzlePFX, Mesh, "Muzzle", FVector(0,0,0), FRotator(0,0,0), EAttachLocation::SnapToTarget, true);
	}
	for (int i = 0; i < Multishot; ++i)
	{
		if (ProjectileClass)
		{
			FVector Location = GetActorLocation() + GetActorForwardVector()*50.0f;
			FRotator Rotation = OwningChar->GetBaseAimRotation();
			GetWorld()->SpawnActor(ProjectileClass, &Location, &Rotation);
		} else
		{
			bool TraceHit = false;
			FHitResult Hit;
			FVector TraceStart;
			FVector TraceEnd;
			FRotator EyeRotation;
			OwningChar->GetActorEyesViewPoint(TraceStart, EyeRotation);
			EyeRotation = EyeRotation + FRotator(FMath::RandRange(-VerticalSpread, VerticalSpread), FMath::RandRange(-HorizontalSpread, HorizontalSpread),0);
			TraceEnd = TraceStart + EyeRotation.Vector()*Range;
			FCollisionQueryParams CollisionParameters;
			CollisionParameters.AddIgnoredActor(this);
			CollisionParameters.AddIgnoredActor(GetAttachParentActor());
			GetWorld()->LineTraceSingleByChannel(Hit, TraceStart, TraceEnd, ECollisionChannel::ECC_Visibility, CollisionParameters);
			TraceHit = Hit.bBlockingHit;
			//DrawDebugLine(GetWorld(), TraceStart, TraceEnd, FColor(255, 0, 0), false, 3);
			IDamageableInterface* HitActor = Cast<IDamageableInterface>(Hit.GetActor());
			if (Mesh->DoesSocketExist("Muzzle") && TrailPFX)
			{
				FVector TrailEnd = (TraceHit) ? Hit.ImpactPoint : TraceEnd;
				UNiagaraComponent* TrailPFXComponent = UNiagaraFunctionLibrary::SpawnSystemAttached(TrailPFX, Mesh, "Muzzle", FVector(0,0,0), FRotator(0,0,0), EAttachLocation::SnapToTarget, true);
				TrailPFXComponent->SetVectorParameter("BeamEnd", TrailEnd);
			}
			if (Hit.bBlockingHit)
			{
				
				FVector ForceVector = (TraceEnd-TraceStart);
				ForceVector.Normalize();
			
				if (Hit.GetComponent()->IsSimulatingPhysics())
				{
					Hit.GetComponent()->AddImpulse(ForceVector*Force);
					
					
				}
				if (BulletImpactActor && !HitActor)
				{
					FVector Location = Hit.Location;
					FRotator Rotation = Hit.Normal.Rotation() + FRotator(-90, 0, 0);
					AActor* Decal = GetWorld()->SpawnActor(BulletImpactActor, &Location, &Rotation);
					Decal->AttachToComponent(Hit.GetComponent(), FAttachmentTransformRules::KeepWorldTransform);
				}
					
			
				if (HitActor)
				{
					FVector HitDir = Hit.Location - TraceStart;
					HitDir.Normalize();
					FPointDamageEvent PointDamageEvent;
					PointDamageEvent.Damage = Damage * FalloffCurve->GetFloatValue(Hit.Distance/Range);
					PointDamageEvent.HitInfo = Hit;
					
					if (APawn* OwningPawn = Cast<APawn>(GetOwner()))
						HitActor->TakePointDamage(Damage, HitDir*Force, PointDamageEvent, OwningPawn->GetController(), this);
				}
			}
			
		}
	}
	BulletsFired++;
	if (BulletsFired==BurstAmount)
	{
		ReleaseTrigger();
	}
	OnFire.Broadcast();
}

void AGunBase::FinishReload_Implementation()
{
	bReloading = false;
	int32 AmountNeed = MaxMagazine - CurMagazine; //32 - 27 gives 5 for example
	int32 AmountGrabbed = FMath::Min(AmountNeed, CurReserve);
	CurMagazine = CurMagazine + AmountGrabbed;
	CurReserve = CurReserve - AmountGrabbed;
	OnReload.Broadcast();
}

