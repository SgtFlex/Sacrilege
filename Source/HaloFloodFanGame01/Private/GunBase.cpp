// Fill out your copyright notice in the Description page of Project Settings.

#include "GunBase.h"

#include "BulletFiringComponent.h"
#include "GrenadeWidget.h"
#include "HaloGameState.h"
#include "MyCustomBlueprintFunctionLibrary.h"
#include "NiagaraComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "PlayerControllerBase.h"
#include "VectorTypes.h"
#include "WorldCleanupManager.h"
#include "Camera/CameraComponent.h"
#include "Components/Image.h"
#include "Engine/DamageEvents.h"
#include "HaloFloodFanGame01/PlayerCharacter.h"
#include "HaloFloodFanGame01/ProjectileBase.h"
#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"
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

	BulletFiringComponent = CreateDefaultSubobject<UBulletFiringComponent>("BulletFiringComponent");
	BulletFiringComponent->SetupAttachment(Mesh, "Muzzle");

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
	GetWorld()->GetSubsystem<UWorldCleanupManager>()->StopManagingWeapon(this);
	//Cast<AHaloGameState>(GetWorld()->GetGameState())->StopManagingWeapon(this);
}

void AGunBase::OnEquipped()
{
	// if (DrawSFX) UGameplayStatics::PlaySoundAtLocation(GetWorld(), DrawSFX, GetActorLocation());
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
	GetWorld()->GetSubsystem<UWorldCleanupManager>()->ManageWeapon(this);
	//Cast<AHaloGameState>(GetWorld()->GetGameState())->ManageWeapon(this);
}

void AGunBase::StartReload_Implementation()
{
	if (bReloading || CurReserve <= 0 || CurMagazine == MaxMagazine) return;
	ScopeOut();
	bReloading = true;
	if (BurstAmount > 0) GetWorldTimerManager().ClearTimer(FireHandle);
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

	if (GetWorldTimerManager().TimerExists(BurstRetriggerHandle) || CurMagazine <= 0 || bReloading)
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



void AGunBase::OnInteract_Implementation(ACharacterBase* Character)
{
	IInteractableInterface::OnInteract_Implementation(Character);
	Character->PickupWeapon(this);
}

void AGunBase::GetInteractInfo_Implementation(FText& Text, UTexture2D*& Icon, ACharacterBase* InteractingCharacter)
{
	IInteractableInterface::GetInteractInfo_Implementation(Text, Icon, InteractingCharacter);

	Text = InteractText;
	Icon = InteractIcon;
	
}

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

FVector AGunBase::GetAim()
{
	if (CharacterOwner)
	{
		return CharacterOwner->GetBaseAimRotation().Vector();
	} else
	{
		return GetActorRotation().Vector();
	}
	
}


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
		TSubclassOf<AActor>* ImpactFXClass = ImpactFXMap.Find(Hit.PhysMaterial->SurfaceType);
		UE_LOG(LogTemp, Warning, TEXT("Hit: %s"), *Hit.PhysMaterial->GetFName().ToString());
		if (ImpactFXClass)
		{
			FVector Location = Hit.ImpactPoint;
			FRotator Rotation = Hit.Normal.Rotation() + FRotator(-90, 0, 0);
			AActor* Decal = GetWorld()->SpawnActor(*ImpactFXClass, &Location, &Rotation);
			if (Decal)
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
			FVector TraceStart;
			FRotator EyeRotation;
			if (CharacterOwner)
			{
				CharacterOwner->GetActorEyesViewPoint(TraceStart, EyeRotation);
				EventInstigator = CharacterOwner->GetController();
			} else
			{
				TraceStart = GetActorLocation();
			}
			EyeRotation = GetAim().Rotation() + FRotator(FMath::RandRange(-VerticalSpread, VerticalSpread), FMath::RandRange(-HorizontalSpread, HorizontalSpread),0);
			
			TArray<AActor*> ActorsToIgnore;
			ActorsToIgnore.Add(this);
			ActorsToIgnore.Add(GetOwner());
			// The actual bullet trace, with a width for accuracy forgiveness
			//UKismetSystemLibrary::SphereTraceSingle(GetWorld(), TraceStart, TraceEnd, 20, UEngineTypes::ConvertToTraceType(ECollisionChannel::ECC_Camera), false, ActorsToIgnore, EDrawDebugTrace::None, Hit, true, FLinearColor::Red, FLinearColor::Green, 5);
			BulletFiringComponent->FireBullet(Hit, EyeRotation.Vector(), ActorsToIgnore, this, EventInstigator);
			//UMyCustomBlueprintFunctionLibrary::FireHitScanBullet(Hit, GetWorld(), ActorsToIgnore, TraceStart, EyeRotation.Vector(), Range, FalloffCurve, Damage, Force, this, EventInstigator);
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
	FVector Location = Mesh->DoesSocketExist("Muzzle") ? Mesh->GetSocketLocation("Muzzle") : GetActorLocation() + GetActorForwardVector()*50000.0f;
	FRotator Rotation = GetAim().Rotation() + FRotator(FMath::RandRange(-VerticalSpread, VerticalSpread), FMath::RandRange(-HorizontalSpread, HorizontalSpread),0);
	FActorSpawnParameters ActorSpawnParameters;
	ActorSpawnParameters.Owner = this;
	ActorSpawnParameters.Instigator = CharacterOwner;
	return GetWorld()->SpawnActor(ProjToSpawn, &Location, &Rotation, ActorSpawnParameters);
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

bool AGunBase::ScopeIn_Implementation()
{
	if (ScopeActive || ZoomFOV == 0) return false;
	if (CharacterOwner)
	{
		ScopeActive = true;
		ScopeOverlay = CreateWidget<UUserWidget>(CharacterOwner->PlayerController, ScopeWidget);
		ScopeOverlay->AddToPlayerScreen();
		//PlayerChar->GetFirstPersonCameraComponent()->SetFieldOfView(10);
		if (ScopeInSFX) UGameplayStatics::PlaySound2D(GetWorld(), ScopeInSFX);
	}
	return true;
}

void AGunBase::ScopeOut_Implementation()
{
	if (!ScopeActive) return;
	if (CharacterOwner)
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
	OnAmmoUpdated.Broadcast();
	OnFire.Broadcast();
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