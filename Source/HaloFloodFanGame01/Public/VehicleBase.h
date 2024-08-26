// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "DamageableInterface.h"
#include "InteractableInterface.h"
#include "GameFramework/Pawn.h"
#include "VehicleBase.generated.h"

class UHealthComponent;

UENUM(BlueprintType)
enum EDamageState : uint8
{
	Healthy = 0,
	Damaged = 1,
	Critical = 2,
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnDamageStateChanged, EDamageState, NewDamageState);

UCLASS()
class HALOFLOODFANGAME01_API AVehicleBase : public APawn, public IDamageableInterface, public IInteractableInterface
{
	GENERATED_BODY()

public:
	// Sets default values for this pawn's properties
	AVehicleBase();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	UFUNCTION(BlueprintCallable)
	void SpawnDefaultControllerWithTeam(uint8 TeamId);

	UFUNCTION()
	void OnHealthUpdated(UHealthComponent* HealthComp);

	UFUNCTION()
	void OnHealthDepleted(float Damage, FVector Force, FVector HitLocation,
											  FName HitBoneName, AController* EventInstigator, AActor* DamageCauser);

	UFUNCTION(BlueprintNativeEvent)
	void SetDamageState(EDamageState NewDamageState);

	UFUNCTION(BlueprintCallable)
	void SetIsDestroyed(bool bNewIsDestroyed);

	UFUNCTION(BlueprintCallable)
	void SpawnHUD();

	//IDamageableInterface
	
	virtual float CustomTakePointDamage_Implementation(FPointDamageEvent const& PointDamageEvent, float Force, AController* EventInstigator, AActor* DamageCauser) override;

	virtual float CustomTakeRadialDamage_Implementation(float Force, FRadialDamageEvent const& RadialDamageEvent, AController* EventInstigator, AActor* DamageCauser) override;

	//IInteractableInterface

	virtual void OnInteract_Implementation(ACharacterBase* Character) override;
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSubclassOf<UUserWidget> VehicleHUD;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UHealthComponent* HealthComponent;

	UPROPERTY(BlueprintReadOnly)
	TEnumAsByte<EDamageState> DamageState;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	UStaticMeshComponent* VehicleMesh;

	UPROPERTY(BlueprintAssignable)
	FOnDamageStateChanged OnDamageStateChanged;

	UPROPERTY(BlueprintReadOnly)
	bool bIsDestroyed = false;

};
