// Fill out your copyright notice in the Description page of Project Settings.


#include "Core/EquipmentBase.h"

#include "Components/PickupComponent.h"
#include "Core/CharacterBase.h"

// Sets default values
AEquipmentBase::AEquipmentBase()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	Mesh = CreateDefaultSubobject<UStaticMeshComponent>("Mesh");
	SetRootComponent(Mesh);

	PickupComponent = CreateDefaultSubobject<UPickupComponent>("PickupComp");
	PickupComponent->SetupAttachment(GetRootComponent());
	PickupComponent->SetEnabled(true);

}

// Called when the game starts or when spawned
void AEquipmentBase::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AEquipmentBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void AEquipmentBase::Pickup(ACharacterBase* Character)
{
	if (!Character->GameplayTags.HasTag(FGameplayTag::RequestGameplayTag(FName("Character.CanUseEquipment")))) return;
	if (Character->EquipmentClass==nullptr)
	{
		IPickupInterface::Pickup(Character);
		Character->PickupEquipment(GetClass());
		Destroy();
	}
}

UStaticMeshComponent* AEquipmentBase::GetMesh()
{
	return Mesh;
}

