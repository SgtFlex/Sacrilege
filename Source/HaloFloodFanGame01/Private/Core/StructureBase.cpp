// Fill out your copyright notice in the Description page of Project Settings.


#include "Core/StructureBase.h"

// Sets default values
AStructureBase::AStructureBase()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

FGenericTeamId AStructureBase::GetGenericTeamId() const
{
	return IGenericTeamAgentInterface::GetGenericTeamId();
}

// Called when the game starts or when spawned
void AStructureBase::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AStructureBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

