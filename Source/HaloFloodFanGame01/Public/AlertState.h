// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AlertState.generated.h"

UENUM(BlueprintType)
enum EAlertState
{
	Relaxed,
	Suspicious,
	Alerted
};