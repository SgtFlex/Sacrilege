// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "NotificationSubsystem.generated.h"

/**
 * 
 */

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnNotificationPushed, FString, NotificationText);

UCLASS()

class HALOFLOODFANGAME01_API UNotificationSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable)
	void PushNotificationText(FString Text);

public:
	UPROPERTY(BlueprintAssignable)
	FOnNotificationPushed OnNotificationPushed;
};
