// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "DamageableInterface.h"
#include "GameplayTagAssetInterface.h"
#include "PhysicsInterface.h"
#include "Components/TimelineComponent.h"
#include "GameFramework/Character.h"
#include "CharacterBase.generated.h"

enum EAnimType : int;
class AVehicleBase;
class AGunBase;
class AEquipmentBase;
class APlayerControllerBase;
class UBlendSpace1D;
class AAIControllerBase;
struct FInputActionValue;
enum EAlertState : uint8;
enum EEmotion : uint8;
class ASmartObject;
class AGrenadeBase;
class ADecalActor;
class AWeaponBase;
class AWeaponBase;
class UHealthComponent;

//player class
enum class EHardwareDevicePrimaryType : uint8;
class UInteractableInterface;
class IInteractableInterface;
class USphereComponent;
class UBoxComponent;
class UPlayerHUD;
class AGrenadeBase;
class UInputComponent;
class USkeletalMeshComponent;
class USceneComponent;
class UCameraComponent;
class UAnimMontage;
class USoundBase;

USTRUCT(BlueprintType)
struct FGrenadeStruct
{
	GENERATED_BODY()
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	TSubclassOf<AGrenadeBase> GrenadeClass;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int GrenadeAmount = 0;
	
};

USTRUCT(BlueprintType)
struct FLoadoutStruct
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSubclassOf<AWeaponBase> PrimaryWeaponClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSubclassOf<AWeaponBase> SecondaryWeaponClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSubclassOf<AEquipmentBase> EquipmentClass;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FGrenadeStruct> Grenades;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnInteractableChanged, AActor*, Interactable);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnPickupWeapon);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnDropWeapon);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnTakeCustomPointDamage, float, Damage);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FWeaponsUpdated, AWeaponBase*, NewGun, AWeaponBase*, OldGun);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnKilled, ACharacterBase*, Character, AController*, Instigator, AActor*, Causer);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnGrenadeInvetoryUpdated, TArray<FGrenadeStruct>&, UpdatedGrenadeInventory);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnGrenadeTypeSwitched, TSubclassOf<AGrenadeBase>, GrenadeClass, int, Index);

UCLASS(Abstract)
class HALOFLOODFANGAME01_API ACharacterBase : public ACharacter, public IDamageableInterface, public IGameplayTagAssetInterface, public IPhysicsInterface
{
	GENERATED_BODY()

public:
	// Sets default values for this pawn's properties
	ACharacterBase();
	
	/** Returns Mesh1P subobject **/
	UFUNCTION(BlueprintCallable)
	virtual USkeletalMeshComponent* GetMesh1P() const { return Mesh1P; }
	
	/** Returns FirstPersonCameraComponent subobject **/
	UFUNCTION(BlueprintCallable)
	virtual UCameraComponent* GetFirstPersonCameraComponent() const { return FirstPersonCameraComponent; }

	virtual UUserWidget* GetPlayerHUD() const { return PlayerHUD; }

	UFUNCTION(BlueprintCallable)
	void GetPlayerAim(FHitResult& HitResult) const;
	
	float AimAssist() const;
	

	virtual void Restart() override;

	UFUNCTION()
	virtual void SpawnLoadout();

	UFUNCTION(Server, Reliable)
	virtual void Server_SpawnLoadout();

	UFUNCTION(NetMulticast, Reliable)
	virtual void Multi_SpawnLoadout();
	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	virtual void SpawnDefaultController() override;

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	float CustomTakeDamage(float DamageAmount, FVector Force, AController* EventInstigator = nullptr, AActor* DamageCauser = nullptr);

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	float CustomTakePointDamage(float Damage, FVector Direction, const FHitResult& HitInfo, float Force, AController* EventInstigator = nullptr, AActor* DamageCauser = nullptr);

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	float CustomTakeRadialDamage(FVector Origin, float Radius, float Force, const FHitResult& HitInfo, FRadialDamageEvent const& RadialDamageEvent, float MinimumRadius, AController* EventInstigator = nullptr, AActor* DamageCauser = nullptr);

	UFUNCTION(NetMulticast, Unreliable)
	void MulticastSpawnBloodFX(FVector Direction, const FHitResult& HitInfo);
	
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	UHealthComponent* GetHealthComponent();
	
	UFUNCTION(BlueprintNativeEvent)
	void OnHealthDepleted(float Damage, FVector Force, FVector HitLocation = FVector(0,0,0), FName HitBoneName = "", AController* EventInstigator = nullptr, AActor* DamageCauser = nullptr);
	
	UFUNCTION(Server, Reliable)
	void SV_OnHealthDepleted(float Damage, FVector Force, FVector HitLocation = FVector(0,0,0), FName HitBoneName = "", AController* EventInstigator = nullptr, AActor* DamageCauser = nullptr);

	UFUNCTION(NetMulticast, Reliable)
	void MC_OnHealthDepleted(float Damage, FVector Force, FVector HitLocation = FVector(0,0,0), FName HitBoneName = "", AController* EventInstigator = nullptr, AActor* DamageCauser = nullptr);

	UFUNCTION()
	void CreateRagdollCorpse() const;
	
	UFUNCTION(BlueprintNativeEvent)
	void DropGrenades();
	
	UFUNCTION()
	virtual void OnHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit );

	UFUNCTION(BlueprintCallable)
	void SetSmartObject(ASmartObject* NewSmartObject);
	
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	void Melee();

	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable)
	void Lunge(AActor* Target, const FVector& ImpactPoint);

	UFUNCTION(BlueprintCallable, Server, Reliable)
	void SV_Melee();

	// UFUNCTION(BlueprintImplementableEvent)
	// void SlideMelee(AActor* Actor);

	UFUNCTION(BlueprintCallable)
	virtual void MeleeActor(AActor* Actor);
	
	UFUNCTION(BlueprintCallable)
	void MeleeUpdate(float Alpha);

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	void PlayerMelee();



	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	void NPCMelee();
	
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	bool CanMelee();

	UFUNCTION(NetMulticast, Unreliable)
	void MulticastPlayMeleeFX();

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	void EquipGrenadeType(TSubclassOf<AGrenadeBase> Grenade);

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	bool AddGrenade(TSubclassOf<AGrenadeBase> GrenadeType, int Amount);

	UFUNCTION(BlueprintCallable)
	void ThrowEquippedGrenade();
	
	UFUNCTION(BlueprintCallable)
	void ThrowGrenade(int GrenadeTypeIndex);

	UFUNCTION(BlueprintCallable)
	void SubtractGrenade(int GrenadeTypeIndex);

	UFUNCTION(BlueprintCallable)
	void RemoveGrenadeStruct(int GrenadeTypeIndex);

	UFUNCTION(Server, Reliable)
	void ServerThrowGrenade(int GrenadeTypeIndex);

	UFUNCTION()
	void SpawnGrenade(const TSubclassOf<AGrenadeBase>& GrenadeType);

	UFUNCTION(BlueprintCallable, NetMulticast, Unreliable)
	void MulticastThrowGrenade(TSubclassOf<AGrenadeBase> GrenadeType);

	UFUNCTION(BlueprintCallable, BlueprintCosmetic)
	void ThrowGrenadeFX(TSubclassOf<AGrenadeBase> GrenadeType);

	UFUNCTION(BlueprintCallable)
	TArray<FGrenadeStruct>& GetGrenadeInventory();

	UFUNCTION(BlueprintCallable, BlueprintPure)
	TSubclassOf<AGrenadeBase> GetSelectedGrenadeType() const;

	UFUNCTION(BlueprintCallable)
	bool SelectGrenadeType(TSubclassOf<AGrenadeBase> GrenadeType);

	UFUNCTION(BlueprintCallable)
	void SetGrenadeTypeIndex(int Index);

	UFUNCTION(BlueprintCallable, BlueprintPure)
	int GetGrenadeTypeIndex() const;

	UFUNCTION(BlueprintCallable)
	void CycleGrenadeType();

	UFUNCTION(Client, Unreliable, BlueprintCosmetic)
	void ClientCycleGrenadeType() const;
	
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	void UseEquipment();

	UFUNCTION(BlueprintCallable)
	virtual void PrimaryAttack_Pull();

	UFUNCTION(Server, Reliable)
	virtual void Server_PrimaryAttack_Pull();

	UFUNCTION(NetMulticast, Reliable)
	virtual void Multi_PrimaryAttack_Pull();

	UFUNCTION(BlueprintCallable)
	virtual void PrimaryAttack_Release();

	UFUNCTION(Server, Reliable)
	virtual void Server_PrimaryAttack_Release();

	UFUNCTION(NetMulticast, Reliable)
	virtual void Multi_PrimaryAttack_Release();
	
	UFUNCTION(BlueprintCallable)
	virtual void ReloadWeapon();

	UFUNCTION(BlueprintCallable, Server, Reliable)
	virtual void Server_ReloadWeapon();

	UFUNCTION(BlueprintCallable, NetMulticast, Reliable)
	virtual void Multi_ReloadWeapon();

	UFUNCTION(BlueprintCallable)
	virtual void SwitchWeapon();

	UFUNCTION(BlueprintCallable, Server, Reliable)
	virtual void Server_SwitchWeapon();

	UFUNCTION(BlueprintCallable, NetMulticast, Reliable)
	virtual void Multi_SwitchWeapon();

	UFUNCTION(Server, Reliable)
	void ServerFinishSwitchingWeapons();

	UFUNCTION()
	virtual void ScopeWeapon();

	UFUNCTION(BlueprintCallable)
	virtual void SecondaryAttack_Start();

	UFUNCTION(BlueprintCallable)
	virtual void SecondaryAttack_End();

	UFUNCTION()
	virtual void DrawEquippedWeapon();

	UFUNCTION(BlueprintCallable)
	virtual void SetupViewmodel(bool bFirstPerson);

	UFUNCTION(NetMulticast, Reliable)
	virtual void MulticastHolsterEquippedWeapon();

	UFUNCTION(BlueprintCallable)
	virtual void PickupWeapon(AWeaponBase* Gun);

	UFUNCTION(Server, Reliable)
	virtual void Server_PickupWeapon(AWeaponBase* Gun);
	
	// UFUNCTION(NetMulticast, Reliable, BlueprintCallable)
	// virtual void Multi_PickupWeapon(AWeaponBase* Gun);

	UFUNCTION()
	void OnRep_EquippedWeapon();

	UFUNCTION()
	void OnRep_HolsteredWeapon();

	UFUNCTION(BlueprintCallable)
	virtual void DropEquippedWeapon();

	UFUNCTION(BlueprintCallable)
	void DropWeapon(AWeaponBase* Gun);

	UFUNCTION()
	void RagdollSettled(UPrimitiveComponent* Component, FName Name);

	UFUNCTION(BlueprintCallable)
	virtual void Stun(float StunTime = 1);

	UFUNCTION(NetMulticast, Unreliable)
	void MulticastPlayStunAnimation(float StunTime);

	UFUNCTION()
	void Unstun() const;

	UFUNCTION(Server, Reliable)
	void ServerSetCurrentInteractable();

	UFUNCTION(NetMulticast, Unreliable)
	void MulticastUpdateInteractInfo();

	UFUNCTION(BlueprintCallable)
	void Interact();

	UFUNCTION(Server, Reliable)
	void Server_Interact();

	void ApplyPhysicsImpulse_Implementation(FVector Force, FVector Location, FName BoneName = NAME_None) override;


protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	/** Called for movement input */
	void Move(const FInputActionValue& Value);

	/** Called for looking input */
	void Look(const FInputActionValue& Value);

	UFUNCTION(Server, Unreliable)
	void Server_Look(float Pitch);

	UFUNCTION(NetMulticast, Unreliable)
	void Multi_Look(float Pitch);

	virtual void NotifyRestarted() override;
	
	virtual void UnPossessed() override;

	UFUNCTION(Client, Reliable)
	virtual void CL_Unpossessed();

	

	virtual void GetOwnedGameplayTags(FGameplayTagContainer& TagContainer) const override { TagContainer = GameplayTags; return; }

public:
	//Delegates

	UPROPERTY(BlueprintAssignable)
	FOnTakeCustomPointDamage OnTakeCustomPointDamage;
	
	UPROPERTY(BlueprintAssignable)
	FWeaponsUpdated WeaponsUpdated;
	
	UPROPERTY(BlueprintAssignable)
	FOnPickupWeapon OnPickupWeapon;

	UPROPERTY(BlueprintAssignable)
	FOnDropWeapon OnDropWeapon;

	UPROPERTY(BlueprintAssignable)
	FOnKilled OnKilled;

	UPROPERTY(BlueprintAssignable)
	FOnGrenadeInvetoryUpdated OnGrenadeInventoryUpdated;

	UPROPERTY(BlueprintAssignable)
	FOnGrenadeTypeSwitched OnGrenadeTypeSwitched;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Replicated)
	UHealthComponent* HealthComponent;
	

	//Loadout
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="Unit Information|Loadout", meta = (DisplayPriority=0, ExposeOnSpawn=true))
	TArray<FLoadoutStruct> Loadouts;

	UPROPERTY(BlueprintReadWrite, meta = (DisplayPriority=0, ExposeOnSpawn=true), Replicated)
	TArray<FGrenadeStruct> GrenadeInventory;

	UPROPERTY(BlueprintReadWrite)
	TSubclassOf<AEquipmentBase> EquipmentClass;

	UPROPERTY(BlueprintReadOnly, Replicated, ReplicatedUsing=OnRep_EquippedWeapon)
	AWeaponBase* EquippedWeapon;

	UPROPERTY(BlueprintReadOnly, Replicated, ReplicatedUsing=OnRep_HolsteredWeapon)
	AWeaponBase* HolsteredWeapon;

	UPROPERTY()
	FTimerHandle HolsterHandle;

	UPROPERTY()
	FTimerHandle DrawHandle;

	//Blood
	UPROPERTY(EditAnywhere)
	class UNiagaraSystem* BloodPFX;

	UPROPERTY(EditAnywhere)
	UMaterialInterface* BloodDecalMaterial;

	UPROPERTY(EditAnywhere)
	UMaterialInterface* BloodSplatterMat;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	USoundBase* DeathSound;

	//Anims
	UPROPERTY(EditDefaultsOnly)
	UAnimMontage* FiringAnim;

	UPROPERTY(EditDefaultsOnly)
	UAnimMontage* HurtAnim;

	UPROPERTY(EditDefaultsOnly)
	UAnimMontage* DeathAnim;

	UPROPERTY(EditDefaultsOnly)
	UAnimMontage* MeleeAnim;

	UPROPERTY(EditDefaultsOnly)
	UBlendSpace* HitReactionBS;

	UPROPERTY(EditInstanceOnly, BlueprintReadWrite, Category="AI", meta = (ExposeOnSpawn = "true"))
	ASmartObject* SmartObject;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Replicated, Category="AI")
	TEnumAsByte<EAlertState> AlertState;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Replicated, Category="AI")
	TEnumAsByte<EEmotion> Emotion;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ExposeOnSpawn = "true"), Replicated, Category="AI")
	uint8 TeamId = 0;

	UPROPERTY(EditDefaultsOnly)
	bool CanInteract = true;

	UPROPERTY(EditDefaultsOnly)
	bool CanPickupWeapons = true;

	UPROPERTY(EditDefaultsOnly)
	bool CanEnterVehicles = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float MaxSpeed = 100.0f;
	
	UPROPERTY()
	FTimerHandle StunTimer;

	UPROPERTY()
	FTimerHandle StunCooldownTimer;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float CurrentStunBuildup = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float StunThreshold = 100;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float StunCooldown = 3.0f;

	UPROPERTY()
	int CurGrenadeTypeI = 0;
	
	UPROPERTY(BlueprintReadOnly)
	APlayerControllerBase* PlayerController;
	
	UPROPERTY(BlueprintAssignable)
	FOnInteractableChanged OnInteractableChanged;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSubclassOf<class UUserWidget> PlayerHUDClass;

	UPROPERTY(BlueprintReadWrite)
	class UUserWidget* PlayerHUD;

	UPROPERTY(EditAnywhere, Category="Loadout")
	int32 FragCount = 0;
	UPROPERTY(EditAnywhere, Category="Loadout")
	int32 PlasmaCount = 0;
	UPROPERTY(EditAnywhere, Category="Loadout")
	int32 SpikeCount = 0;
	UPROPERTY(EditAnywhere, Category="Loadout")
	int32 IncenCount = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bCanUseWeapons = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bCanUseGrenades = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bCanUseVehicles = true;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bCanMelee = true;

	UPROPERTY(EditDefaultsOnly)
	UAnimMontage* ThrowGrenadeAnimation1P;

	UPROPERTY(EditDefaultsOnly)
	UAnimMontage* ThrowGrenadeAnimation;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TMap<TEnumAsByte<EAnimType>, UAnimMontage*> VehicleAnimEnterMontages;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TMap<TEnumAsByte<EAnimType>, UAnimMontage*> VehicleAnimExitMontages;
	
	UPROPERTY()
	TArray<AActor*> InteractableActors;

	UPROPERTY(EditDefaultsOnly)
	UCurveFloat* MeleeCurve;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GameplayTags")
	FGameplayTagContainer GameplayTags;

	float ScopeSensitivityMultiplier = 1;

	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Replicated)
	bool bIsInVehicle = false;

	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Replicated)
	AVehicleBase* OccupiedVehicle;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TMap<FName, float> HitBoxDamageMultipliers = {
		{"Head", 10.0f},
	{"Arm", 0.5f},
{"Arm", 0.5f},
{"Leg", 0.5f},
{"Leg", 0.5f},	
	};

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TMap<FName, FName> HitBoxNameMap = {
		{"Neck_001", "Head"},
	{"ForeArm_L", "Arm"},
{"ForeArm_R", "Arm"},
{"LowerLeg_L", "Leg"},
{"LowerLeg_R", "Leg"},	
	};
	
private:
	UPROPERTY(Replicated)
	AActor* InteractableActor;

	UPROPERTY()
	FTimerHandle ShieldDelayTimerHandle;

	UPROPERTY()
	FTimeline MeleeTimeline;

	UPROPERTY()
	FHitResult PlayerAim;

	UPROPERTY()
	UInputDeviceSubsystem* InputDeviceSubsystem;
	
protected:
	UPROPERTY(BlueprintReadWrite)
	FTimerHandle MeleeTimer;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float MeleeDamage = 30;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float MeleeForce = 100000;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float MeleeCooldown = 1;

	UPROPERTY()
	FTimerHandle RagdollTimer;
	
	/** Pawn mesh: 1st person view (arms; seen only by self) */
	UPROPERTY(EditAnywhere, Category=Mesh)
	USkeletalMeshComponent* Mesh1P;

	/** First person camera */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	UCameraComponent* FirstPersonCameraComponent;

	// UPROPERTY(EditAnywhere)
	// USphereComponent* InteractionSphere;

	/** MappingContext */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category=Input, meta=(AllowPrivateAccess = "true"))
	class UInputMappingContext* DefaultMappingContext;

	/** Look Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	class UInputAction* LookAction;
	
	/** Jump Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category=Input, meta=(AllowPrivateAccess = "true"))
	class UInputAction* JumpAction;

	/** Move Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category=Input, meta=(AllowPrivateAccess = "true"))
	class UInputAction* MoveAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category=Input, meta = (AllowPrivateAccess = "true"))
	class UInputAction* InteractAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category=Input, meta = (AllowPrivateAccess = "true"))
	class UInputAction* SwitchWeaponAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category=Input, meta = (AllowPrivateAccess = "true"))
	class UInputAction* PrimaryAttackAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category=Input, meta = (AllowPrivateAccess = "true"))
	class UInputAction* MeleeAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category=Input, meta = (AllowPrivateAccess = "true"))
	class UInputAction* ReloadAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category=Input, meta = (AllowPrivateAccess = "true"))
	class UInputAction* SwitchGrenadeAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category=Input, meta = (AllowPrivateAccess = "true"))
	class UInputAction* ThrowGrenadeAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category=Input, meta = (AllowPrivateAccess = "true"))
	class UInputAction* UseEquipmentAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category=Input, meta = (AllowPrivateAccess = "true"))
	class UInputAction* CrouchAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category=Input, meta = (AllowPrivateAccess = "true"))
	class UInputAction* ScopeAction;

	UPROPERTY()
	UEnhancedInputComponent* EnhancedInputComponent;

	UPROPERTY()
	FTimerHandle PossessionDelay;

	UPROPERTY(BlueprintReadWrite)
	FVector StartMeleeLocation;

	UPROPERTY(BlueprintReadWrite)
	FVector EndMeleeLocation;

	UPROPERTY(BlueprintReadWrite)
	FRotator StartMeleeRotation;

	UPROPERTY(BlueprintReadOnly)
	FHitResult MeleeHit;

	UPROPERTY(EditDefaultsOnly)
	TMap<TEnumAsByte<EPhysicalSurface>, TSubclassOf<ADecalActor>> MeleeImpactFX;

};
