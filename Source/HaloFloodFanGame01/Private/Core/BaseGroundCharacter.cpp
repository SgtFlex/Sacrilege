// Fill out your copyright notice in the Description page of Project Settings.


#include "Core/BaseGroundCharacter.h"

#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/MovementComponent.h"
#include "Components/CapsuleComponent.h"

ABaseGroundCharacter::ABaseGroundCharacter()
{
	CharMoveComp = CreateDefaultSubobject<UCharacterMovementComponent>(TEXT("Character Movement"));
	CharMoveComp->UpdatedComponent = CapsuleCollider;
}
