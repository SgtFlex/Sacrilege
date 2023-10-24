// Fill out your copyright notice in the Description page of Project Settings.


#include "HaloPDA.h"

#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Components/WidgetComponent.h"
#include "HaloFloodFanGame01/HaloFloodFanGame01Character.h"
#include "HaloFloodFanGame01/HaloFloodFanGame01GameMode.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"

// Sets default values
AHaloPDA::AHaloPDA()
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
void AHaloPDA::BeginPlay()
{
	Super::BeginPlay();
	Pawn = Cast<AHaloFloodFanGame01Character>(GetOwner());
	PlayerController = Cast<APlayerController>(Pawn->GetController());
	
	if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
	{
		Subsystem->AddMappingContext(MappingContext, 2);
		EnableInput(PlayerController);
		UE_LOG(LogTemp, Warning, TEXT("Added mapping context"));
		UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerController->InputComponent);
		if (EnhancedInputComponent)
		{
			EnhancedInputComponent->BindAction(NavigateInputAction, ETriggerEvent::Triggered, this, &AHaloPDA::Navigate);

			EnhancedInputComponent->BindAction(SelectInputAction, ETriggerEvent::Triggered, this, &AHaloPDA::Select);

			EnhancedInputComponent->BindAction(ClosePDAAction, ETriggerEvent::Triggered, this, &AHaloPDA::ClosePDA);

			UE_LOG(LogTemp, Warning, TEXT("Added binds"));
		}
	}
}

void AHaloPDA::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);

	if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
	{
		Subsystem->RemoveMappingContext(MappingContext);		
	}
	StopBuildPreview();
}

// Called every frame
void AHaloPDA::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	
	if (bIsPreviewing && PreviewActor)
	{
		FVector Location = Pawn->GetActorLocation() + (UKismetMathLibrary::GetForwardVector(Pawn->GetBaseAimRotation())*500);
		PreviewActor->SetActorTransform(UKismetMathLibrary::MakeTransform(Location, FRotator(0,0,0)));
	}
}

AActor* AHaloPDA::StartBuildPreview(TSubclassOf<AActor> ActorToPreview)
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

void AHaloPDA::StopBuildPreview()
{
	if (!bIsPreviewing) return;
	bIsPreviewing = false;
	if (PreviewActor) PreviewActor->Destroy();
}

bool AHaloPDA::CanBuildItem(FBuyable Buyable)
{
	AHaloFloodFanGame01GameMode* GameMode = Cast<AHaloFloodFanGame01GameMode>(UGameplayStatics::GetGameMode(GetWorld()));
	return GameMode->GetPlayerResource(PlayerController) >= Buyable.Cost;
}

AActor* AHaloPDA::BuildItem(FBuyable Buyable)
{
	UE_LOG(LogTemp, Warning, TEXT("build item request"));
	AHaloFloodFanGame01GameMode* GameMode = Cast<AHaloFloodFanGame01GameMode>(UGameplayStatics::GetGameMode(GetWorld()));
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

void AHaloPDA::ClosePDA(const FInputActionValue& InputActionValue)
{
	Destroy();
}

void AHaloPDA::Navigate_Implementation(const FInputActionValue& InputActionValue)
{
	
}

void AHaloPDA::Select_Implementation(const FInputActionValue& InputActionValue)
{
	
}

