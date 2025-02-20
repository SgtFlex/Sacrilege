// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "DamageableInterface.h"
#include "InteractableInterface.h"
#include "GameFramework/Pawn.h"
#include "VehicleBase.generated.h"

class ACharacterBase;
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
	
	UFUNCTION(BlueprintCallable)
	void UpdateDamageState();
	
	UFUNCTION(BlueprintNativeEvent)
	void SetDamageState(EDamageState NewDamageState);

	UFUNCTION(BlueprintCallable)
	void SetIsDestroyed(bool bNewIsDestroyed);

	void NotifyRestarted() override;

	virtual void UnPossessed() override;

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	void Enter(ACharacterBase* NewPilot);

	UFUNCTION(Client, Reliable)
	void CL_Enter(ACharacterBase* NewPilot);

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	void Exit();

	UFUNCTION(Client, Reliable)
	void CL_Exit();

	UFUNCTION(Server, Reliable)
	void ResetPilot();

	UFUNCTION(NetMulticast, Reliable)
	void DetachPilot();

	UFUNCTION(Server, Reliable)
	void SetPilotToPossess(ACharacterBase* NewPilot);
	
	UFUNCTION(NetMulticast, Reliable)
	void AttachPilot();

	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable)
	void SpawnHUD();

	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable)
	void RemoveHUD();

	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable)
	void SpawnControls();

	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable)
	void RemoveControls();

	UFUNCTION()
	void OnPilotKilled(ACharacterBase* Character, AController* Killer, AActor* Causer);

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UFUNCTION(BlueprintCallable, BlueprintPure)
	USkeletalMeshComponent* GetVehicleMesh();
	
	//Delegate Binds

	UFUNCTION()
	void OnHealthUpdated(UHealthComponent* HealthComp);

	UFUNCTION()
	void OnHealthDepleted(float Damage, FVector Force, FVector HitLocation,
											  FName HitBoneName, AController* EventInstigator, AActor* DamageCauser);
	
	//IDamageableInterface

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	float CustomTakeDamage(float Damage, FVector Force, AController* EventInstigator = nullptr, AActor* DamageCauser = nullptr);

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	float CustomTakePointDamage(float Damage, FVector Direction, const FHitResult& HitInfo, float Force, AController* EventInstigator = nullptr, AActor* DamageCauser = nullptr);
	
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	float CustomTakeRadialDamage(float Force, FRadialDamageEvent const& RadialDamageEvent, AController* EventInstigator = nullptr, AActor* DamageCauser = nullptr);

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	UHealthComponent* GetHealthComponent();

	//IInteractableInterface

	virtual void OnInteract_Implementation(ACharacterBase* Character) override;
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSubclassOf<UUserWidget> VehicleHUDClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UUserWidget* VehicleHUD;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UHealthComponent* HealthComponent;

	UPROPERTY(BlueprintReadOnly)
	TEnumAsByte<EDamageState> DamageState;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	UStaticMeshComponent* VehicleMesh;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	USkeletalMeshComponent* VehicleSkeletalMesh;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	USceneComponent* ExitPoint;

	UPROPERTY(BlueprintAssignable)
	FOnDamageStateChanged OnDamageStateChanged;

	UPROPERTY(BlueprintReadWrite, ReplicatedUsing=AttachPilot)
	ACharacterBase* Pilot;

	UPROPERTY(BlueprintReadWrite)
	AController* PilotController;

	UPROPERTY(BlueprintReadOnly)
	bool bIsDestroyed = false;

};
