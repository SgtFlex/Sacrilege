// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "DamageableInterface.h"
#include "GameplayTagAssetInterface.h"
#include "InteractableInterface.h"
#include "GameFramework/Pawn.h"
#include "VehicleBase.generated.h"

class UCameraComponent;
class ACharacterBase;
class UHealthComponent;

UENUM(BlueprintType)
enum EDamageState : uint8
{
	Healthy = 0,
	Damaged = 1,
	Critical = 2,
	Destroyed = 3,
};

UENUM(BlueprintType)
enum EAnimType {
	Turret,
	Driver,
	Passenger,
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnDamageStateChanged, EDamageState, NewDamageState);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnEntered);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnExited);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnIgnoreListUpdated);

UCLASS(Abstract)
class HALOFLOODFANGAME01_API AVehicleBase : public APawn, public IDamageableInterface, public IInteractableInterface, public IGameplayTagAssetInterface
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

	UFUNCTION(Server, Reliable)
	void ServerUpdateAimRotation();

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

	virtual void PossessedBy(AController* NewController) override;

	UFUNCTION(Client, Reliable)
	void Client_PossessedBy(AController* NewController);

	virtual void UnPossessed() override;

	UFUNCTION(Client, Reliable)
	void Client_Unpossessed();

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	void Enter(ACharacterBase* NewPilot);

	UFUNCTION(Client, Reliable)
	void CL_Enter(ACharacterBase* NewPilot);

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	void Exit();

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	void DoVehicleUnpossession();

	UFUNCTION(Client, Reliable)
	void CL_Exit();

	UFUNCTION(Server, Reliable)
	void ServerResetPilot();

	UFUNCTION(NetMulticast, Reliable)
	void MulticastDetachPilot();

	UFUNCTION(Server, Reliable)
	void ServerSetPilotToPossess(ACharacterBase* NewPilot);
	
	UFUNCTION(NetMulticast, Reliable)
	void MulticastAttachPilot();

	UFUNCTION(BlueprintImplementableEvent)
	void LerpToSeat(const FVector& StartingPoint);

	UFUNCTION(BlueprintImplementableEvent)
	void LerpToExit();

	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable)
	void SpawnHUD();

	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable)
	void RemoveHUD();

	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable)
	void SpawnControls();

	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable)
	void RemoveControls();

	virtual FVector GetNavAgentLocation() const override;

	UFUNCTION()
	void OnPilotKilled(ACharacterBase* Character, AController* Killer, AActor* Causer);

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UFUNCTION(BlueprintCallable, BlueprintPure)
	USkeletalMeshComponent* GetVehicleMesh();

	UFUNCTION(BlueprintCallable)
	void AddChildVehicleToIgnoreList(AVehicleBase* Vehicle);

	UFUNCTION(BlueprintCallable)
	void UpdateIgnoreList();

	UFUNCTION(BlueprintCallable)
	void GetIgnoreActors(TArray<AActor*>& IgnoreActors);
	
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
	float CustomTakeRadialDamage(FVector Origin, float Radius, float Force, const FHitResult& HitInfo, FRadialDamageEvent const& RadialDamageEvent, float MinimumRadius, AController* EventInstigator = nullptr, AActor* DamageCauser = nullptr);

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, BlueprintPure)
	UHealthComponent* GetHealthComponent();

	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable)
	UCameraComponent* GetCamera();

	//IInteractableInterface

	virtual void OnInteract_Implementation(ACharacterBase* Character) override;

	virtual void GetOwnedGameplayTags(FGameplayTagContainer& TagContainer) const override { TagContainer = GameplayTags; return; }

public:
	UPROPERTY()
	FTimerHandle ExitDelayHandle;
	
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

	UPROPERTY(BlueprintReadWrite, ReplicatedUsing=MulticastAttachPilot)
	ACharacterBase* Pilot;

	UPROPERTY(BlueprintReadWrite)
	AController* PilotController;

	UPROPERTY(BlueprintReadOnly)
	bool bIsDestroyed = false;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	float AgentZOffset = 200.0f;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	float ExitTime = 1.0f;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	TEnumAsByte<EAnimType> AnimType = EAnimType::Driver;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	FVector LeftHandIKLocation;

	UPROPERTY(BlueprintReadOnly, Replicated)
	FRotator AimRotation;

	
	//Delegates
	UPROPERTY(BlueprintAssignable)
	FOnEntered	OnEntered;

	UPROPERTY(BlueprintAssignable)
	FOnExited	OnExited;

	UPROPERTY(BlueprintReadWrite)
	TArray<AVehicleBase*> ChildVehicles;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GameplayTags")
	FGameplayTagContainer GameplayTags;

	UPROPERTY(BlueprintAssignable)
	FOnIgnoreListUpdated IgnoreListUpdated;

};