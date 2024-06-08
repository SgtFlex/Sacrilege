// Fill out your copyright notice in the Description page of Project Settings.


#include "PDA.h"

#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Components/WidgetComponent.h"
#include "HaloFloodFanGame01/PlayerCharacter.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"

// Sets default values
APDA::APDA()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	SceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Scene"));
	Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	ScreenWidgetComponent = CreateDefaultSubobject<UWidgetComponent>(TEXT("ScreenWidget"));
	ScreenWidgetComponent->SetupAttachment(Mesh, "Screen");
	Mesh->SetupAttachment(SceneComponent);
	SetRootComponent(SceneComponent);
}

// Called when the game starts or when spawned
void APDA::BeginPlay()
{
	Super::BeginPlay();
	//PlayerController = Cast<APlayerController>(GetOwner());
	// Pawn = Cast<ACharacterBase>(PlayerController->GetPawn());
	if (GetOwner())
	{
		if (HasAuthority())
			CL_BeginPlay();
	}
	
	
}

void APDA::CL_BeginPlay_Implementation()
{
	// GEngine->AddOnScreenDebugMessage(-1, 3, FColor::Black, GetOwner()->GetActorLabel());
	// if (!PlayerController) return;
	APlayerController* PC = Cast<APlayerController>(GetOwner());
	if (!PC) return;
	if (PC->IsLocalController())
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PC->GetLocalPlayer()))
		{
			Subsystem->AddMappingContext(MappingContext, 2);
			EnableInput(PC);
			UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PC->InputComponent);
			if (EnhancedInputComponent)
			{
				EnhancedInputComponent->BindAction(NavigateInputAction, ETriggerEvent::Triggered, this, &APDA::Navigate);

				EnhancedInputComponent->BindAction(SelectInputAction, ETriggerEvent::Triggered, this, &APDA::Select);

				EnhancedInputComponent->BindAction(ClosePDAAction, ETriggerEvent::Triggered, this, &APDA::ClosePDA);
			}
		}
	}
}

void APDA::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);
	StopBuildPreview();
	CL_EndPlay();
}

void APDA::CL_EndPlay_Implementation()
{
	APlayerController* PC = Cast<APlayerController>(GetOwner());
	if (!PC) return;
	if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PC->GetLocalPlayer()))
	{
		Subsystem->RemoveMappingContext(MappingContext);		
	}
}

// Called every frame
void APDA::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	
	// if (bIsPreviewing && PreviewActor)
	// {
	// 	FVector Location = Pawn->GetActorLocation() + (UKismetMathLibrary::GetForwardVector(Pawn->GetBaseAimRotation())*500);
	// 	float Yaw = Pawn->GetControlRotation().Yaw;
	// 	PreviewActor->SetActorTransform(UKismetMathLibrary::MakeTransform(Location, FRotator(0, Yaw, 0)));
	// }
}

void APDA::StartBuildPreview_Implementation(TSubclassOf<AActor> ActorToPreview)
{
	// if (bIsPreviewing) StopBuildPreview();
	// bIsPreviewing = true;
	// FVector SpawnLoc = UKismetMathLibrary::GetForwardVector(Pawn->GetBaseAimRotation()) * 500.0f;
	// FActorSpawnParameters ActorSpawnParameters = FActorSpawnParameters();
	// FRotator Rotation = GetOwner()->GetActorRotation() + FRotator(0, 90, 0);
	// ActorSpawnParameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	// AActor* SpawnedActor = GetWorld()->SpawnActor(ActorToPreview, &SpawnLoc, &Rotation, ActorSpawnParameters);
	// if (SpawnedActor)
	// {
	// 	TArray<UActorComponent*> SpawnedActorComponenets;
	// 	SpawnedActor->GetComponents(SpawnedActorComponenets);
	// 	for (auto SpawnedActorComponenet : SpawnedActorComponenets)
	// 	{
	// 		SpawnedActorComponenet->Deactivate();
	// 		if (UPrimitiveComponent* PrimComp = Cast<UPrimitiveComponent>(SpawnedActorComponenet))
	// 		{
	// 			PrimComp->SetSimulatePhysics(false);
	// 			PrimComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	// 			for (int i = 0; i < PrimComp->GetNumMaterials(); ++i)
	// 			{
	// 				PrimComp->SetMaterial(i, PreviewMaterial);
	// 			}
	// 		}			
	// 	}
	// }
	//
	// PreviewActor = SpawnedActor;
}

void APDA::StopBuildPreview_Implementation()
{
	// if (!bIsPreviewing) return;
	// bIsPreviewing = false;
	// if (PreviewActor) PreviewActor->Destroy();
}

bool APDA::CanBuildItem(FBuyable Buyable)
{
	// AFirefightGameMode* GameMode = Cast<AFirefightGameMode>(UGameplayStatics::GetGameMode(GetWorld()));
	// return GameMode->GetPlayerResource(PlayerController) >= Buyable.Cost;


	return true;
}

AActor* APDA::BuildItem_Implementation(FBuyable Buyable)
{
	// UE_LOG(LogTemp, Warning, TEXT("build item request"));
	// //AFirefightGameMode* GameMode = Cast<AFirefightGameMode>(UGameplayStatics::GetGameMode(GetWorld()));
	// if (CanBuildItem(Buyable))
	// {
	// 	
	// 	AActor* SpawnedBuyable = nullptr;
	// 	if (PreviewActor)
	// 	{
	// 		FTransform Transform = PreviewActor->GetTransform();
	// 		FActorSpawnParameters ActorSpawnParameters = FActorSpawnParameters();
	// 		ActorSpawnParameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	// 		SpawnedBuyable = GetWorld()->SpawnActor(Buyable.SpawnableActor, &Transform, ActorSpawnParameters);
	// 	}
	// 	//if (SpawnedBuyable) GameMode->SetPlayerResource(PlayerController, GameMode->GetPlayerResource(PlayerController) - Buyable.Cost);
	// 	return SpawnedBuyable;
	// }
	// UE_LOG(LogTemp, Warning, TEXT("Too broke!"));
	return nullptr;
}

void APDA::ClosePDA(const FInputActionValue& InputActionValue)
{
	Destroy();
}

void APDA::Navigate_Implementation(const FInputActionValue& InputActionValue)
{
	
}

void APDA::Select_Implementation(const FInputActionValue& InputActionValue)
{
	
}

