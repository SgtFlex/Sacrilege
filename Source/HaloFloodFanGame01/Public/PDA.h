// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "InputActionValue.h"
#include "InputMappingQuery.h"
#include "GameFramework/Actor.h"
#include "PDA.generated.h"
struct FInputActionValue;
class APlayerCharacter;
USTRUCT(BlueprintType)
struct FBuyable
{
	GENERATED_BODY()
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	FText DisplayName;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	TSubclassOf<AActor> SpawnableActor;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	int Cost;
	
};
class UWidgetComponent;
UCLASS()
class HALOFLOODFANGAME01_API APDA : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	APDA();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	//variables

	UFUNCTION(BlueprintNativeEvent)
	void Navigate(const FInputActionValue& InputActionValue);

	UFUNCTION(BlueprintNativeEvent)
	void Select(const FInputActionValue& InputActionValue);

private:
	UFUNCTION(BlueprintCallable)
	AActor* StartBuildPreview(TSubclassOf<AActor> ActorToPreview);

	UFUNCTION(BlueprintCallable)
	void StopBuildPreview();

	UFUNCTION(BlueprintCallable)
	bool CanBuildItem(FBuyable Buyable);

	UFUNCTION(BlueprintCallable)
	AActor* BuildItem(FBuyable Buyable);

	UFUNCTION()
	void ClosePDA(const FInputActionValue& InputActionValue);

public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	USceneComponent* SceneComponent;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	UWidgetComponent* ScreenWidgetComponent;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	UStaticMeshComponent* Mesh;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	TArray<FBuyable> Buyables;

	UPROPERTY(EditDefaultsOnly)
	UMaterialInterface* PreviewMaterial;

	UPROPERTY(BlueprintReadOnly)
	APlayerController* PlayerController;

	UPROPERTY(BlueprintReadOnly)
	APlayerCharacter* Pawn;

private:
	UPROPERTY(EditDefaultsOnly, Category=Input)
	UInputMappingContext* MappingContext;

	UPROPERTY(EditDefaultsOnly, Category=Input)
	UInputAction* NavigateInputAction;

	UPROPERTY(EditDefaultsOnly, Category=Input)
	UInputAction* SelectInputAction;

	UPROPERTY(EditDefaultsOnly, Category=Input)
	UInputAction* ClosePDAAction;

	UPROPERTY()
	UUserWidget* ScreenWidget;

	UPROPERTY()
	AActor* PreviewActor;

	UPROPERTY()
	bool bIsPreviewing = false;
	
};
