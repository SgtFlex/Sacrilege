// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "DamageableInterface.h"
#include "Components/TimelineComponent.h"
#include "GameFramework/Character.h"
#include "CharacterBase.generated.h"

enum EAlertState : uint8;
enum EEmotion : uint8;
class ASmartObject;
class AGrenadeBase;
class ADecalActor;
class AGunBase;
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

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnInteractableChanged, AActor*, Interactable);
//
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnPickupWeapon);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnDropWeapon);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FWeaponsUpdated, AGunBase*, NewGun, AGunBase*, OldGun);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnKilled, ACharacterBase*, Character, AController*, Instigator, AActor*, Causer);



USTRUCT(BlueprintType)
struct FGrenadeStruct
{
	GENERATED_BODY()
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	TSubclassOf<AGrenadeBase> GrenadeClass;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int GrenadeAmount;
	
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnGrenadeInvetoryUpdated, TArray<FGrenadeStruct>, GrenadeInventory);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnGrenadeTypeSwitched, TSubclassOf<AGrenadeBase>, GrenadeClass);

UCLASS()
class HALOFLOODFANGAME01_API ACharacterBase : public ACharacter, public IDamageableInterface
{
	GENERATED_BODY()

public:
	// Sets default values for this pawn's properties
	ACharacterBase();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	/** Pawn mesh: 1st person view (arms; seen only by self) */
	UPROPERTY(EditAnywhere, Category=Mesh)
	USkeletalMeshComponent* Mesh1P;

	/** First person camera */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	UCameraComponent* FirstPersonCameraComponent;

	UPROPERTY(EditAnywhere)
	USphereComponent* InteractionSphere;

	/** MappingContext */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category=Input, meta=(AllowPrivateAccess = "true"))
	class UInputMappingContext* DefaultMappingContext;

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
	
public:
	/** Returns Mesh1P subobject **/
	UFUNCTION(BlueprintCallable)
	virtual USkeletalMeshComponent* GetMesh1P() const { return Mesh1P; }
	
	/** Returns FirstPersonCameraComponent subobject **/
	UFUNCTION(BlueprintCallable)
	virtual UCameraComponent* GetFirstPersonCameraComponent() const { return FirstPersonCameraComponent; }

	virtual UUserWidget* GetPlayerHUD() const { return PlayerHUD; }
public:

	virtual void Restart() override;

	UFUNCTION()
	virtual void SpawnWeapons();

	UFUNCTION(Server, Reliable)
	virtual void Server_SpawnWeapons();

	UFUNCTION(NetMulticast, Reliable)
	virtual void Multi_SpawnWeapons();

	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	float CustomTakeDamage(float DamageAmount, FVector Force, FDamageEvent const& DamageEvent, AController* EventInstigator = nullptr, AActor* DamageCauser = nullptr);

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	float CustomTakePointDamage(FPointDamageEvent const& PointDamageEvent, float Force, AController* EventInstigator = nullptr, AActor* DamageCauser = nullptr);

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	float CustomTakeRadialDamage(float Force, FRadialDamageEvent const& RadialDamageEvent, AController* EventInstigator = nullptr, AActor* DamageCauser = nullptr);

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	UHealthComponent* GetHealthComponent();
	
	UFUNCTION(BlueprintNativeEvent)
	void OnHealthDepleted(float Damage, FVector Force, FVector HitLocation = FVector(0,0,0), FName HitBoneName = "", AController* EventInstigator = nullptr, AActor* DamageCauser = nullptr);

	UFUNCTION(BlueprintNativeEvent)
	void DropGrenades();
	
	UFUNCTION()
	virtual void OnHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit );

	UFUNCTION(BlueprintCallable)
	void SetSmartObject(ASmartObject* NewSmartObject);
	
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	void Melee();

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	void PlayerMelee();

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	void NPCMelee();
	
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	bool CanMelee();

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	void EquipGrenadeType(TSubclassOf<AGrenadeBase> Grenade);

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	void ThrowEquippedGrenade();

	UFUNCTION(BlueprintCallable)
	virtual void UseEquipment();

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

	UFUNCTION()
	virtual void EquipWeapon(AGunBase* Gun);

	UFUNCTION()
	virtual void HolsterWeapon(AGunBase* Gun);

	UFUNCTION(BlueprintCallable)
	virtual void PickupWeapon(AGunBase* Gun);

	UFUNCTION(Server, Reliable, BlueprintCallable)
	virtual void Server_PickupWeapon(AGunBase* Gun);
	
	UFUNCTION(NetMulticast, Reliable, BlueprintCallable)
    virtual void Multi_PickupWeapon(AGunBase* Gun);

	UFUNCTION(BlueprintCallable)
	virtual void DropWeapon();

	UFUNCTION()
	void RagdollSettled(UPrimitiveComponent* Component, FName Name);

	UFUNCTION(BlueprintCallable)
	virtual void Stun(float StunTime = 1);

	void Unstun();

public:
	//Delegates
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
	
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	UHealthComponent* HealthComponent;

	//Loadout
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="Unit Information|Loadout", meta = (DisplayPriority=0, ExposeOnSpawn=true), Replicated)
	TSubclassOf<AGunBase> EquippedWeaponClass;
	
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="Unit Information|Loadout", meta = (DisplayPriority=0, ExposeOnSpawn=true), Replicated)
	TSubclassOf<AGunBase> HolsteredWeaponClass;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="Unit Information|Loadout", meta = (DisplayPriority=0, ExposeOnSpawn=true))
	TArray<FGrenadeStruct> GrenadeInventory;

	UPROPERTY(BlueprintReadOnly, Replicated)
	AGunBase* EquippedWeapon;

	UPROPERTY(BlueprintReadOnly, Replicated)
	AGunBase* HolsteredWeapon;

	FTimerHandle HolsterHandle;

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

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	ASmartObject* SmartObject;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Replicated)
	TEnumAsByte<EAlertState> AlertState;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Replicated)
	TEnumAsByte<EEmotion> Emotion;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float MaxSpeed = 100.0f;
	
	UPROPERTY()
	FTimerHandle StunTimer;

	UPROPERTY()
	float StunAmount = 100;

	int CurGrenadeTypeI = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ExposeOnSpawn = "true"))
	uint8 TeamId = 0;
private:
	
protected:
	UPROPERTY(BlueprintReadWrite)
	FTimerHandle MeleeTimer;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float MeleeDamage = 30;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float MeleeForce = 100000;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float MeleeCooldown = 1;

	FTimerHandle RagdollTimer;

public:
	UPROPERTY(BlueprintReadOnly)
	APlayerController* PlayerController;
	
	UPROPERTY()
	FOnInteractableChanged OnInteractableChanged;

	UPROPERTY(EditAnywhere)
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

	// UPROPERTY(EditDefaultsOnly)
	// UAnimMontage* DrawAnimation1P;
	//
	// UPROPERTY(EditDefaultsOnly)
	// UAnimMontage* HolsterAnimation1P;
	//
	// UPROPERTY(EditDefaultsOnly)
	// UAnimMontage* FireAnimation1P;

	UPROPERTY(EditDefaultsOnly)
	UAnimMontage* ThrowGrenadeAnimation1P;

	UPROPERTY()
	TArray<AActor*> InteractableActors;

private:
	UPROPERTY()
	AActor* InteractableActor;
	
	FTimerHandle ShieldDelayTimerHandle;

	FTimeline MeleeTimeline;

	UPROPERTY(EditDefaultsOnly)
	UCurveFloat* MeleeCurve;

	FHitResult PlayerAim;

	UPROPERTY()
	UInputDeviceSubsystem* InputDeviceSubsystem;
};
