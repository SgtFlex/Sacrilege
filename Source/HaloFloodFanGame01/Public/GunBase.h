// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "InteractableInterface.h"
#include "GameFramework/Actor.h"
#include "GunBase.generated.h"

UCLASS()
class HALOFLOODFANGAME01_API AGunBase : public AActor, public IInteractableInterface
{
	GENERATED_BODY()
public:
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category=Mesh)
	USkeletalMeshComponent* Mesh;
	
public:	
	// Sets default values for this actor's properties
	AGunBase();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	UFUNCTION(BlueprintNativeEvent)
	void PrimaryAttack();

	UFUNCTION(BlueprintNativeEvent)
	void SecondaryAttack();

	UFUNCTION(BlueprintNativeEvent)
	void Fire();

	UFUNCTION(BlueprintNativeEvent)
	void Reload();
	
	virtual void OnInteract_Implementation(AHaloFloodFanGame01Character* Character) override;

public:
	UPROPERTY(EditAnywhere)
		TSubclassOf<AActor> PrimProj;

	UPROPERTY(EditAnywhere)
		TSubclassOf<AActor> SecProj;
	
	UPROPERTY(EditAnywhere)
	float FireRate = 10;

	UPROPERTY(EditAnywhere)
	float ReloadSpeed = 3;

	UPROPERTY(EditAnywhere)
	float HorizontalRecoil = 1;

	UPROPERTY(EditAnywhere)
	float VerticalRecoil = 1;

	UPROPERTY(EditAnywhere)
	float Accuracy = 100;

	UPROPERTY(EditAnywhere)
	int32 MaxMagazine = 32;

	int32 CurMagazine;
	UPROPERTY(EditAnywhere)
	int32 MaxReserve = 160;

	int32 CurReserve;

	UPROPERTY(EditAnywhere)
	TSubclassOf<UUserWidget> BulletWidget;
	
	USceneComponent* Camera;

	ACharacter* Wielder;
};
