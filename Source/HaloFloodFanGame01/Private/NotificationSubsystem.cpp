// Fill out your copyright notice in the Description page of Project Settings.


#include "NotificationSubsystem.h"

void UNotificationSubsystem::PushNotificationText(FString Text)
{
	OnNotificationPushed.Broadcast(Text);
}
