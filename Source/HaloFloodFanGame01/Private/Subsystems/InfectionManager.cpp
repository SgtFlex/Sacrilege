// Fill out your copyright notice in the Description page of Project Settings.


#include "Subsystems/InfectionManager.h"

#include "Core/CharacterBase.h"

// ACharacterBase* UInfectionManager::CreateInfected(ACharacterBase* Host)
// {
// 	FActorSpawnParameters ActorSpawnParameters;
// 	ACharacterBase* ChosenPod = CharsBeingInfected[Host].Pods[0];
// 	UnlatchInfectionForms(Host, CharsBeingInfected[Host].Pods);
// 	ACharacterBase* FloodSpawn = GetWorld()->SpawnActorDeferred<ACharacterBase>(Host->GetClass(), Host->GetActorTransform());
// 	
// 	FloodSpawn->TeamId = ChosenPod->TeamId;
// 	
// 	ChosenPod->Destroy();
// 	Host->Destroy();
// 	
// 	FloodSpawn->FinishSpawning(Host->GetActorTransform());
// 	return FloodSpawn;
// }
//
// void UInfectionManager::OnInfectedCharacterKilled(ACharacterBase* Host, AController* Controller, AActor* Causer)
// {
// 	CreateInfected(Host);
// }
//
// void UInfectionManager::AddLatchedInfectionForm(ACharacterBase* Host, ACharacterBase* InfForm)
// {
// 	Host->OnKilled.AddDynamic(this, &UInfectionManager::OnInfectedCharacterKilled);
// 	CharsBeingInfected[Host].Pods.Add(InfForm);
// }
//
// void UInfectionManager::RemoveLatchedInfectionForm(ACharacterBase* Host, ACharacterBase* InfForm)
// {
// 	CharsBeingInfected[Host].Pods.Add(InfForm);
// 	if (CharsBeingInfected[Host].Pods.IsEmpty())
// 		CharsBeingInfected.Remove(Host);
// 	//CharsBeingInfected[Host].Remove(InfForm);
// 	//if (CharsBeingInfected[Host].IsEmpty()) CharsBeingInfected.Remove(Host);
// }
