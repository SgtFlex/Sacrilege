// Fill out your copyright notice in the Description page of Project Settings.


#include "Subsystems/NotificationSubsystem.h"

void UNotificationSubsystem::PushNotificationText_Implementation(const FString& Text)
{
	OnNotificationPushed.Broadcast(Text);
}

void UNotificationSubsystem::PushGlobalNotification_Implementation(const FString& Text)
{
	PushNotificationText(Text);
}
