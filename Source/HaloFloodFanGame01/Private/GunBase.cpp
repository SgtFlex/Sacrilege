// Fill out your copyright notice in the Description page of Project Settings.

#include "GunBase.h"

#include "GrenadeWidget.h"
#include "HaloGameState.h"
#include "MyCustomBlueprintFunctionLibrary.h"
#include "NiagaraComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "VectorTypes.h"
#include "Camera/CameraComponent.h"
#include "Components/Image.h"
#include "Engine/DamageEvents.h"
#include "HaloFloodFanGame01/PlayerCharacter.h"
#include "Kismet/GameplayStatics.h"
#include "Perception/AISense_Hearing.h"
#include "Perception/AISense_Sight.h"


// Sets default values
AGunBase::AGunBase()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	
	Mesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("CharacterMesh1P"));
	Mesh->SetSimulatePhysics(true);
	RootComponent = Mesh;

	bReplicates = true;
	
}

// Called when the game starts or when spawned
void AGunBase::BeginPlay()
{
	Super::BeginPlay();
	CurMagazine = MaxMagazine;
	CurReserve = MaxReserve;
	InteractIcon = WeaponIcon;
}

// Called every frame
void AGunBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void AGunBase::OnPickup(ACharacterBase* Char)
{
	SetOwner(Char);
	CharacterOwner = Char;
	Cast<AHaloGameState>(GetWorld()->GetGameState())->StopManagingWeapon(this);
}

void AGunBase::OnEquipped()
{
	if (DrawSFX) UGameplayStatics::PlaySoundAtLocation(GetWorld(), DrawSFX, GetActorLocation());
}

void AGunBase::OnDropped()
{
	
	GetWorldTimerManager().ClearTimer(ReloadTimer);
	ScopeOut();
	bReloading = false;
	SetOwner(nullptr);
	CharacterOwner = nullptr;
	if (CurMagazine + CurReserve <= 0)
	{
		//Disable collision query responses to prevent being picked up.
		Mesh->SetCollisionEnabled(ECollisionEnabled::PhysicsOnly);
	}
	Cast<AHaloGameState>(GetWorld()->GetGameState())->ManageWeapon(this);
}

void AGunBase::StartReload_Implementation()
{
	Server_StartReload();
}

void AGunBase::Server_StartReload_Implementation()
{
	Multi_StartReload();
}

void AGunBase::Multi_StartReload_Implementation()
{
	if (bReloading || CurReserve <= 0 || CurMagazine == MaxMagazine) return;
	ScopeOut();
	bReloading = true;
	if (BurstAmount > 0) GetWorldTimerManager().ClearTimer(FireHandle);
	GetWorld()->GetTimerManager().SetTimer(ReloadTimer, this, &AGunBase::FinishReload, ReloadSpeed, false);
	if (APlayerCharacter* PlayerChar = Cast<APlayerCharacter>(CharacterOwner))
		PlayerChar->GetMesh1P()->GetAnimInstance()->Montage_Play(ReloadAnimation1P,   ReloadAnimation1P->GetPlayLength() / ReloadSpeed);
	if (ReloadSound) UGameplayStatics::SpawnSoundAttached(ReloadSound, GetRootComponent());
}

void AGunBase::PullTrigger_Implementation()
{

	if (GetWorldTimerManager().TimerExists(BurstRetriggerHandle) || CurMagazine <= 0 || bReloading)
		return;
	BulletsFired = 0;
	GetWorldTimerManager().SetTimer(FireHandle, this, &AGunBase::Fire, 60/FireRate, true, 0);
	GetWorldTimerManager().SetTimer(BurstRetriggerHandle, BurstRetriggerDelay, false);
}

void AGunBase::Server_PullTrigger_Implementation()
{
	Multi_PullTrigger();
}

void AGunBase::Multi_PullTrigger_Implementation()
{
	if (GetWorldTimerManager().TimerExists(BurstRetriggerHandle) || CurMagazine <= 0 || bReloading)
		return;
	BulletsFired = 0;
	GetWorldTimerManager().SetTimer(FireHandle, this, &AGunBase::Fire, 60/FireRate, true, 0);
	GetWorldTimerManager().SetTimer(BurstRetriggerHandle, BurstRetriggerDelay, false);
}


void AGunBase::Server_ReleaseTrigger_Implementation()
{
	Multi_ReleaseTrigger();
}

void AGunBase::Multi_ReleaseTrigger_Implementation()
{
	if (BulletsFired >= BurstAmount)
		GetWorldTimerManager().ClearTimer(FireHandle);
}

void AGunBase::ReleaseTrigger_Implementation()
{
	Server_ReleaseTrigger();
}

void AGunBase::OnInteract_Implementation(ACharacterBase* Character)
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

bool AGunBase::CanFire()
{
	return !(bReloading || CurMagazine <= 0);
}

void AGunBase::SpawnBullet_Implementation()
{
	Server_SpawnBullet();
	
}

void AGunBase::Server_SpawnBullet_Implementation()
{
	Multi_SpawnBullet();
}

void AGunBase::Multi_SpawnBullet_Implementation()
{
	UAISense_Hearing::ReportNoiseEvent(GetWorld(), GetActorLocation(), 1.0f, GetOwner(), 0.0f);
	APawn* OwningPawn = Cast<APawn>(GetOwner());
	if (ACharacterBase* OwningChar = Cast<ACharacterBase>(GetOwner()))
	{
		OwningChar->GetMesh()->GetAnimInstance()->Montage_Play(OwningChar->FiringAnim);
	}
	
	if (APlayerCharacter* PlayerChar = Cast<APlayerCharacter>(GetOwner()))
	{
		if (FireAnimation1P) PlayerChar->GetMesh1P()->GetAnimInstance()->Montage_Play(FireAnimation1P);
		if (APlayerController* PC = Cast<APlayerController>(PlayerChar->GetController())) PC->ClientPlayForceFeedback(FireFeedback);
	}
	
	for (int i = 0; i < MultiShot; ++i)
	{
		if (ProjectileClass)
		{
			FVector Location = Mesh->DoesSocketExist("Muzzle") ? Mesh->GetSocketLocation("Muzzle") : GetActorLocation() + GetActorForwardVector()*50000.0f;
			FRotator Rotation;
			if (OwningPawn)
			{
				Rotation = OwningPawn->GetBaseAimRotation() + FRotator(FMath::RandRange(-VerticalSpread, VerticalSpread), FMath::RandRange(-HorizontalSpread, HorizontalSpread),0);
			} else
			{
				Rotation = GetActorRotation();
			}
			FActorSpawnParameters ActorSpawnParameters;
			ActorSpawnParameters.Owner = this;
			ActorSpawnParameters.Instigator = Cast<ACharacterBase>(this->GetOwner());
			GetWorld()->SpawnActor(ProjectileClass, &Location, &Rotation, ActorSpawnParameters);
		} else
		{
			FHitResult Hit;
			FVector TraceStart;
			FRotator EyeRotation;
			if (OwningPawn && OwningPawn->GetController())
			{
				OwningPawn->GetController()->GetPlayerViewPoint(TraceStart, EyeRotation);
				EyeRotation = EyeRotation + FRotator(FMath::RandRange(-VerticalSpread, VerticalSpread), FMath::RandRange(-HorizontalSpread, HorizontalSpread),0);
				//OwningPawn->GetActorEyesViewPoint(TraceStart, EyeRotation);
				//EyeRotation = OwningPawn->GetBaseAimRotation() + FRotator(FMath::RandRange(-VerticalSpread, VerticalSpread), FMath::RandRange(-HorizontalSpread, HorizontalSpread),0);

			} else
			{
				TraceStart = GetActorLocation();
				EyeRotation = GetActorRotation();
			}
			
			TArray<AActor*> ActorsToIgnore;
			ActorsToIgnore.Add(this);
			ActorsToIgnore.Add(GetOwner());
			// The actual bullet trace, with a width for accuracy forgiveness
			//UKismetSystemLibrary::SphereTraceSingle(GetWorld(), TraceStart, TraceEnd, 20, UEngineTypes::ConvertToTraceType(ECollisionChannel::ECC_Camera), false, ActorsToIgnore, EDrawDebugTrace::None, Hit, true, FLinearColor::Red, FLinearColor::Green, 5);
			AController* EventInstigator = nullptr;
			if (OwningPawn)
			{
				EventInstigator = OwningPawn->GetController();
			}
			UMyCustomBlueprintFunctionLibrary::FireHitScanBullet(Hit, GetWorld(), ActorsToIgnore, TraceStart, EyeRotation.Vector(), Range, FalloffCurve, Damage, Force, this, EventInstigator);
			if (Mesh->DoesSocketExist("Muzzle") && TrailPFX)
			{
				FVector TrailEnd = (Hit.bBlockingHit) ? Hit.ImpactPoint : Hit.TraceEnd;
				UNiagaraComponent* TrailPFXComponent = UNiagaraFunctionLibrary::SpawnSystemAttached(TrailPFX, Mesh, "Muzzle", FVector(0,0,0), FRotator(0,0,0), EAttachLocation::SnapToTarget, true);
				TrailPFXComponent->SetVectorParameter("BeamEnd", TrailEnd);
			}
			if (Hit.bBlockingHit)
			{
				if (HitSound) UGameplayStatics::PlaySoundAtLocation(GetWorld(), HitSound, Hit.Location);
				if (ImpactDecal && !Cast<IDamageableInterface>(Hit.GetActor()))
				{
					FVector Location = Hit.ImpactPoint;
					FRotator Rotation = Hit.Normal.Rotation() + FRotator(-90, 0, 0);
					AActor* Decal = GetWorld()->SpawnActor(ImpactDecal, &Location, &Rotation);
					Decal->AttachToComponent(Hit.GetComponent(), FAttachmentTransformRules::KeepWorldTransform);
					//UGameplayStatics::SpawnDecalAttached(GetWorld(), FVector(10,10,10), ) //Perhaps optimize this in the future
				}
			}
		}
	}
	BulletsFired++;
	if (BulletsFired==BurstAmount)
	{
		ReleaseTrigger();
	}
	SpawnMuzzleFX();
}

void AGunBase::SpawnMuzzleFX_Implementation()
{
	if (APlayerCharacter* Char = Cast<APlayerCharacter>(GetOwner()))
		if (FiringCameraShake && Char->IsLocallyControlled())
			Cast<APlayerController>(Char->GetController())->PlayerCameraManager->StartCameraShake(FiringCameraShake, 1, ECameraShakePlaySpace::CameraLocal);
	if (FiringSound)
		UGameplayStatics::SpawnSoundAttached(FiringSound, GetRootComponent());
	if (Mesh->DoesSocketExist("Muzzle") && MuzzlePFX && !ScopeActive)
		UNiagaraFunctionLibrary::SpawnSystemAttached(MuzzlePFX, Mesh, "Muzzle", FVector(0,0,0), FRotator(0,0,0), EAttachLocation::SnapToTarget, true);
}

void AGunBase::ScopeIn_Implementation()
{
	if (ScopeActive) return;
	if (APlayerCharacter* PlayerChar = Cast<APlayerCharacter>(GetOwner()))
	{
		ScopeActive = true;
		ScopeOverlay = CreateWidget<UUserWidget>(Cast<APlayerController>(PlayerChar->GetController()), ScopeWidget);
		ScopeOverlay->AddToPlayerScreen();
		//PlayerChar->GetFirstPersonCameraComponent()->SetFieldOfView(10);
		if (ScopeInSFX) UGameplayStatics::PlaySound2D(GetWorld(), ScopeInSFX);
	}
}

void AGunBase::ScopeOut_Implementation()
{
	if (!ScopeActive) return;
	if (APlayerCharacter* PlayerChar = Cast<APlayerCharacter>(GetOwner()))
	{
		ScopeOverlay->RemoveFromParent();
		//PlayerChar->GetFirstPersonCameraComponent()->SetFieldOfView(90);
		ScopeActive = false;
		if (ScopeOutSFX) UGameplayStatics::PlaySound2D(GetWorld(), ScopeOutSFX);
	}
}

void AGunBase::Fire_Implementation()
{
	if (!CanFire())
	{
		ReleaseTrigger();
		return;
	}
	CurMagazine--;
	
	
	SpawnBullet();
	
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