// Fill out your copyright notice in the Description page of Project Settings.


#include "PDA.h"

#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Components/WidgetComponent.h"
#include "HaloFloodFanGame01/PlayerCharacter.h"
#include "HaloFloodFanGame01/FirefightGamemode.h"
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
	ScreenWidgetComponent->SetupAttachment(Mesh);
	Mesh->SetupAttachment(SceneComponent);
	SetRootComponent(SceneComponent);
}

// Called when the game starts or when spawned
void APDA::BeginPlay()
{
	Super::BeginPlay();
	Pawn = Cast<APlayerCharacter>(GetOwner());
	PlayerController = Cast<APlayerController>(Pawn->GetController());
	
	if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
	{
		Subsystem->AddMappingContext(MappingContext, 2);
		EnableInput(PlayerController);
		UE_LOG(LogTemp, Warning, TEXT("Added mapping context"));
		UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerController->InputComponent);
		if (EnhancedInputComponent)
		{
			EnhancedInputComponent->BindAction(NavigateInputAction, ETriggerEvent::Triggered, this, &APDA::Navigate);

			EnhancedInputComponent->BindAction(SelectInputAction, ETriggerEvent::Triggered, this, &APDA::Select);

			EnhancedInputComponent->BindAction(ClosePDAAction, ETriggerEvent::Triggered, this, &APDA::ClosePDA);

			UE_LOG(LogTemp, Warning, TEXT("Added binds"));
		}
	}
}

void APDA::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);

	if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
	{
		Subsystem->RemoveMappingContext(MappingContext);		
	}
	StopBuildPreview();
}

// Called every frame
void APDA::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	
	if (bIsPreviewing && PreviewActor)
	{
		FVector Location = Pawn->GetActorLocation() + (UKismetMathLibrary::GetForwardVector(Pawn->GetBaseAimRotation())*500);
		PreviewActor->SetActorTransform(UKismetMathLibrary::MakeTransform(Location, FRotator(0,0,0)));
	}
}

AActor* APDA::StartBuildPreview(TSubclassOf<AActor> ActorToPreview)
{
	if (bIsPreviewing) StopBuildPreview();
	bIsPreviewing = true;
	FVector SpawnLoc = UKismetMathLibrary::GetForwardVector(Pawn->GetBaseAimRotation()) * 500.0f;
	AActor* SpawnedActor = GetWorld()->SpawnActor(ActorToPreview, &SpawnLoc);
	if (SpawnedActor)
	{
		TArray<UActorComponent*> SpawnedActorComponenets;
		SpawnedActor->GetComponents(SpawnedActorComponenets);
		for (auto SpawnedActorComponenet : SpawnedActorComponenets)
		{
			SpawnedActorComponenet->Deactivate();
			if (UPrimitiveComponent* PrimComp = Cast<UPrimitiveComponent>(SpawnedActorComponenet))
			{
				PrimComp->SetSimulatePhysics(false);
				PrimComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
				for (int i = 0; i < PrimComp->GetNumMaterials(); ++i)
				{
					PrimComp->SetMaterial(i, PreviewMaterial);
				}
			}			
		}
	}
	
	PreviewActor = SpawnedActor;
	return PreviewActor;
}

void APDA::StopBuildPreview()
{
	if (!bIsPreviewing) return;
	bIsPreviewing = false;
	if (PreviewActor) PreviewActor->Destroy();
}

bool APDA::CanBuildItem(FBuyable Buyable)
{
	AFirefightGameMode* GameMode = Cast<AFirefightGameMode>(UGameplayStatics::GetGameMode(GetWorld()));
	return GameMode->GetPlayerResource(PlayerController) >= Buyable.Cost;
}

AActor* APDA::BuildItem(FBuyable Buyable)
{
	UE_LOG(LogTemp, Warning, TEXT("build item request"));
	AFirefightGameMode* GameMode = Cast<AFirefightGameMode>(UGameplayStatics::GetGameMode(GetWorld()));
	if (CanBuildItem(Buyable))
	{
		
		AActor* SpawnedBuyable = nullptr;
		if (PreviewActor)
		{
			FTransform Transform = PreviewActor->GetTransform();
			SpawnedBuyable = GetWorld()->SpawnActor(Buyable.SpawnableActor, &Transform);
		}
		if (SpawnedBuyable) GameMode->SetPlayerResource(PlayerController, GameMode->GetPlayerResource(PlayerController) - Buyable.Cost);
		return SpawnedBuyable;
	}
	UE_LOG(LogTemp, Warning, TEXT("Too broke!"));
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

