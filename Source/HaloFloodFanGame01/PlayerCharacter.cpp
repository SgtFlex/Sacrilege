// Copyright Epic Games, Inc. All Rights Reserved.

#include "PlayerCharacter.h"
#include "GunBase.h"


APlayerCharacter::APlayerCharacter()
{
}
//@TODO Simplify by removing InteractionSphere. Interaction radius and casting should be done by interactables, not the player character.
//We should simply find the closest Interactable actor within an array of interact-ables



//
// void APlayerCharacter::OnHealthDepleted_Implementation(float Damage, FVector Force, FVector HitLocation, FName HitBoneName, AController* EventInstigator, AActor* DamageCauser)
// {
// 	
// 	if (PlayerController)
// 	{
// 		PlayerController->UnPossess();
// 		if (PlayerHUD)
// 			PlayerHUD->RemoveFromParent();
// 		
// 	}
// 	
// 	Super::OnHealthDepleted_Implementation(Damage, Force, HitLocation, HitBoneName, EventInstigator, DamageCauser);
// 	Cast<AFirefightGameMode>(GetWorld()->GetAuthGameMode())->OnPlayerCharDied.Broadcast(this, Cast<APlayerControllerBase>(PlayerController));
// }

// void APlayerCharacter::ThrowEquippedGrenade_Implementation()
// {
// 	if (GrenadeInventory.Num() <= 0) return;
// 	if (ThrowGrenadeAnimation1P)
// 		GetMesh1P()->GetAnimInstance()->Montage_Play(ThrowGrenadeAnimation1P);
// 	GrenadeInventory[CurGrenadeTypeI].GrenadeAmount -= 1;
// 	const FTransform SpawnTransform = FTransform(GetFirstPersonCameraComponent()->GetForwardVector().Rotation(), GetFirstPersonCameraComponent()->GetComponentLocation() + GetFirstPersonCameraComponent()->GetForwardVector()*300);
// 	FActorSpawnParameters ActorSpawnParameters;
// 	ActorSpawnParameters.Instigator = this;
// 	ActorSpawnParameters.Owner = this;
//
// 	if (AGrenadeBase* Grenade = Cast<AGrenadeBase>(GetWorld()->SpawnActorDeferred<AGrenadeBase>(GrenadeInventory[CurGrenadeTypeI].GrenadeClass, SpawnTransform, this, this, ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn)))
// 	{
// 		Grenade->SetInstigator(this);
// 		Grenade->SetArmed(true);
// 		Grenade->FinishSpawning(SpawnTransform);
// 		UGameplayStatics::SpawnSoundAtLocation(GetWorld(), Grenade->ThrowSFX, Grenade->GetActorLocation());
// 		FVector Direction = GetFirstPersonCameraComponent()->GetForwardVector() + FVector(0,0,0.15);
// 		Direction.Normalize();
// 		Grenade->Mesh->AddImpulse(Direction*2000.0f, NAME_None, true);
// 		Grenade->Mesh->AddAngularImpulseInDegrees(Grenade->GetActorRightVector().GetSafeNormal()*1000 , NAME_None, true);
// 		
// 		if (GrenadeInventory[CurGrenadeTypeI].GrenadeAmount <= 0)
// 		{
// 			GrenadeInventory.RemoveAt(CurGrenadeTypeI);
// 			SwitchGrenadeType(CurGrenadeTypeI);
// 		}
// 		OnGrenadeInventoryUpdated.Broadcast(GrenadeInventory);
// 	}
// }

// void APlayerCharacter::EquipWeapon(AGunBase* Gun)
// {
// 	Super::EquipWeapon(Gun);
// 	if (HolsteredWeapon)
// 		HolsteredWeapon->SetActorHiddenInGame(true);
// 	if (Gun->DrawAnimation1P)
// 		GetMesh1P()->GetAnimInstance()->Montage_Play(Gun->DrawAnimation1P, Gun->DrawAnimation1P->GetPlayLength() / Gun->DrawSpeed);
// 	Gun->AttachToComponent(Mesh1P, FAttachmentTransformRules::SnapToTargetNotIncludingScale, "GripPoint");
// 	//Gun->Mesh->PlayAnimation(Gun->DrawAnimation1P, false);
// 	WeaponsUpdated.Broadcast(EquippedWeapon, HolsteredWeapon);
// }
//
// void APlayerCharacter::HolsterWeapon(AGunBase* Gun)
// {
// 	Gun->ReleaseTrigger();
// 	GetWorldTimerManager().ClearTimer(Gun->ReloadTimer);
// 	Gun->ScopeOut();
// 	Gun->bReloading = false;
//
// 	if (Gun->HolsterAnimation1P)
// 		GetMesh1P()->GetAnimInstance()->Montage_Play(EquippedWeapon->HolsterAnimation1P, EquippedWeapon->HolsterAnimation1P->GetPlayLength() / EquippedWeapon->HolsterSpeed);
// }

// void APlayerCharacter::SwitchWeapon()
// {
// 	Server_SwitchWeapon();
// }
//
// void APlayerCharacter::Server_SwitchWeapon_Implementation()
// {
// 	Multi_SwitchWeapon();
// }
//
// void APlayerCharacter::Multi_SwitchWeapon_Implementation()
// {
// 	if (!(EquippedWeapon && HolsteredWeapon))
// 		return;
// 	HolsterWeapon(EquippedWeapon);
// 	
// 	AGunBase* TempGun = EquippedWeapon;
// 	EquippedWeapon = HolsteredWeapon;
// 	HolsteredWeapon = TempGun;
// 	
// 	//EquipWeapon(EquippedWeapon);
//
// 	GetWorld()->GetTimerManager().SetTimer(HolsterHandle, FTimerDelegate::CreateUObject(this, &APlayerCharacter::EquipWeapon, EquippedWeapon), HolsteredWeapon->HolsterSpeed, false);
// 	
// }
