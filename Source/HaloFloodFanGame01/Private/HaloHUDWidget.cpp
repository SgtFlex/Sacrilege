// Fill out your copyright notice in the Description page of Project Settings.


#include "HaloHUDWidget.h"

#include "Blueprint/WidgetTree.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/GridPanel.h"
#include "Components/UniformGridPanel.h"
#include "Components/Image.h"
#include "GunBase.h"
#include "HealthComponent.h"
#include "Camera/CameraComponent.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"
#include "Components/UniformGridSlot.h"
#include "Components/VerticalBox.h"
#include "HaloFloodFanGame01/HaloFloodFanGame01Character.h"
#include "HaloFloodFanGame01/HaloFloodFanGame01GameMode.h"

void UHaloHUDWidget::NativeConstruct()
{
	Super::NativeConstruct();
	PlayerCharacter = Cast<AHaloFloodFanGame01Character>(GetOwningPlayer()->GetPawn());
	SetFragCounter(PlayerCharacter->FragCount);
	SetPlasmaCounter(PlayerCharacter->PlasmaCount);
	SetSpikeCounter(PlayerCharacter->SpikeCount);
	SetIncenCounter(PlayerCharacter->IncenCount);
	if (PlayerCharacter->EquippedWep)
	{
		UpdateHUDWeaponData(PlayerCharacter->EquippedWep, PlayerCharacter->HolsteredWeapon);
	}
	PlayerCharacter->WeaponsUpdated.AddDynamic(this, &UHaloHUDWidget::UpdateHUDWeaponData);
	PlayerCharacter->GetHealthComponent()->OnHealthUpdate.AddDynamic(this, &UHaloHUDWidget::OnHealthUpdated);
	if (AHaloFloodFanGame01GameMode* FirefightGamemode = Cast<AHaloFloodFanGame01GameMode>(UGameplayStatics::GetGameMode(GetWorld()))) FirefightGamemode->OnWaveStart.AddDynamic(this, &UHaloHUDWidget::UpdateSetAndWaveCount);
}

void UHaloHUDWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	FHitResult PlayerAim = PlayerCharacter->GetPlayerAim();
	if (Cast<IDamageableInterface>(PlayerAim.GetActor()) && PlayerAim.Distance <= 2000)
	{
		SetCrosshairType(4);
	} else if (Cast<IInteractableInterface>(PlayerAim.GetActor()) && PlayerAim.Distance <= 500)
	{
		SetCrosshairType(2);
	} else
	{
		SetCrosshairType(1);
	}

	SetCompassDirection(PlayerCharacter->GetFirstPersonCameraComponent()->GetComponentRotation().Yaw);

	//if (PlayerCharacter && PlayerCharacter->EquippedWep && PlayerCharacter->EquippedWep->CrosshairTexture) Crosshair->SetBrushFromTexture(PlayerCharacter->EquippedWep->CrosshairTexture);
	SetFragCounter(PlayerCharacter->FragCount);
	IInteractableInterface* IntActor = Cast<IInteractableInterface>(PlayerAim.GetActor());
	if (PlayerAim.bBlockingHit && IsValid(PlayerAim.GetActor()) && IntActor)
	{
		FText IntText;
		UTexture2D* IntIcon;
		IntActor->Execute_GetInteractInfo(PlayerAim.GetActor(), IntText, IntIcon);
		SetInteractInfo(IntText, IntIcon);
		SetCanInteract(true);
	}
	else 
		SetCanInteract(false);
}

void UHaloHUDWidget::SetInteractInfo(FText InfoText, UTexture2D* Icon)
{
	if (Icon)
	{
		InteractIcon->SetBrushFromTexture(Icon);
		InteractIcon->SetVisibility(ESlateVisibility::Visible);
	}
	else
	{
		InteractIcon->SetVisibility(ESlateVisibility::Hidden);
	}
}


void UHaloHUDWidget::SetCompassDirection_Implementation(float Yaw)
{
	CompassDirection = (Yaw+180);
	float Offset = 45;
	float x = ((Yaw+Offset)*-10);
	UCanvasPanelSlot* CanvasSlot = Cast<UCanvasPanelSlot>(Compass->Slot);
	CanvasSlot->SetPosition(FVector2d(x, 0));
	//UE_LOG(LogTemp, Warning, TEXT("%f"), Yaw);
	CompassNum->SetText(FText::AsNumber(x));
}


void UHaloHUDWidget::ConstructAmmoGrid_Implementation(AGunBase* Gun)
{
	if (!Gun->BulletWidget) return;
    AmmoGrid->ClearChildren();
	BulletIcons.Empty();
	int32 Columns = 15;
	
	for (int i = Columns; i > 0; i--)
	{
		if ((Gun->MaxMagazine)%i==0)
		{
			Columns = i;
			break;
		}
	}
	int32 Rows = Gun->MaxMagazine/Columns;
	int CurBullet = 0;
	for (int i = 0; i < Rows; ++i)
	{
		for (int j = 0; j < Columns; ++j)
		{
			UUserWidget* BulletIcon = WidgetTree->ConstructWidget<UUserWidget>(Gun->BulletWidget);
			UUniformGridSlot* UniSlot = AmmoGrid->AddChildToUniformGrid(BulletIcon, i, j);
			UniSlot->SetHorizontalAlignment(EHorizontalAlignment::HAlign_Fill);
			UniSlot->SetVerticalAlignment(EVerticalAlignment::VAlign_Fill);
			BulletIcons.Add(BulletIcon);
			CurBullet++;
		}
	}
}

void UHaloHUDWidget::SetAmmoReserveCounter_Implementation(int32 AmmoReserve)
{
	AmmoReserveCounter->SetText(FText::AsNumber(AmmoReserve));
}

void UHaloHUDWidget::SetAmmoGridBullets_Implementation(int32 CurMagazine, int32 MaxMagazine)
{
	int i = 0;
	for (auto BulletIcon : BulletIcons)
	{
		if (i < CurMagazine)
			BulletIcon->SetColorAndOpacity(HUDColor-FLinearColor(0,0,0,.5));
		else
			BulletIcon->SetColorAndOpacity(FLinearColor(0,0,0, .25));
		i++;
	}
}

void UHaloHUDWidget::SetMagazineReserveCounter_Implementation(int32 MagazineCount)
{
	MagazineCounter->SetText(FText::AsNumber(MagazineCount));
}


void UHaloHUDWidget::UpdateHUDMagazineElements()
{
	SetMagazineReserveCounter(PlayerCharacter->EquippedWep->CurMagazine);
	SetAmmoReserveCounter(PlayerCharacter->EquippedWep->CurReserve);
	SetAmmoGridBullets(PlayerCharacter->EquippedWep->CurMagazine, PlayerCharacter->EquippedWep->MaxMagazine);
}

void UHaloHUDWidget::SetCrosshairType(int type)
{
	switch (type)
	{
	default:
	case 1:
		Crosshair->SetColorAndOpacity(HUDColor);
		break;
	case 2:
		Crosshair->SetColorAndOpacity(InteractableColor);
		break;
	case 3:
		Crosshair->SetColorAndOpacity(AllyColor);
		break;
	case 4:
		Crosshair->SetColorAndOpacity(EnemyColor);
		break;
	}
}

void UHaloHUDWidget::SetCrosshairTexture(UTexture2D* NewTexture)
{
	Crosshair->SetBrushFromTexture(NewTexture);
}

void UHaloHUDWidget::SetFragHUDEnabled(bool bDisplay)
{
	if (bDisplay)
	{
		FragHUD->SetVisibility(ESlateVisibility::Visible);
	} else
	{
		FragHUD->SetVisibility(ESlateVisibility::Hidden);
	}
}

void UHaloHUDWidget::SetWeaponHUDEnabled(bool bDisplay)
{
	if (bDisplay)
	{
		WeaponHUD->SetVisibility(ESlateVisibility::Visible);
	} else
	{
		WeaponHUD->SetVisibility(ESlateVisibility::Hidden);
	}
}

void UHaloHUDWidget::OnHealthUpdated(UHealthComponent* HealthComp)
{
	SetHealth(HealthComp->GetHealth(), HealthComp->GetMaxHealth());
	SetShields(HealthComp->GetShields(), HealthComp->GetMaxShields());
}

void UHaloHUDWidget::UpdateHUDWeaponData(AGunBase* EquippedGun, AGunBase* HolsteredGun)
{
	if (EquippedGun)
	{
		SetCrosshairTexture(EquippedGun->CrosshairTexture);
		MagazineCounter->SetVisibility(ESlateVisibility::Visible);
		AmmoReserveCounter->SetVisibility(ESlateVisibility::Visible);
		AmmoGrid->SetVisibility(ESlateVisibility::Visible);
		SetAmmoReserveCounter(EquippedGun->CurReserve);
		EquippedGunWidget->SetBrushFromTexture(EquippedGun->WeaponIcon);
		ConstructAmmoGrid(EquippedGun);
		UpdateHUDMagazineElements();
		EquippedGun->OnFire.AddUniqueDynamic(this, &UHaloHUDWidget::UpdateHUDMagazineElements);
		EquippedGun->OnReload.AddUniqueDynamic(this, &UHaloHUDWidget::UpdateHUDMagazineElements);
	} else
	{
		MagazineCounter->SetVisibility(ESlateVisibility::Hidden);
		AmmoGrid->SetVisibility(ESlateVisibility::Hidden);
		AmmoReserveCounter->SetVisibility(ESlateVisibility::Hidden);
	}
	if (HolsteredGun)
	{
		HolsteredGunWidget->SetVisibility(ESlateVisibility::Visible);
		HolsteredGunWidget->SetBrushFromTexture(HolsteredGun->WeaponIcon);
	} else
	{
		HolsteredGunWidget->SetVisibility(ESlateVisibility::Hidden);
	}
}

void UHaloHUDWidget::UpdateSetAndWaveCount(int Set, int Wave)
{
	SetCount->SetText(FText::AsNumber(Set));
	WaveCount->SetText(FText::AsNumber(Wave));
}

void UHaloHUDWidget::PushTextNotification_Implementation(const FText& Text)
{
	UE_LOG(LogTemp, Warning, TEXT("Pushing notif"));
}


bool UHaloHUDWidget::Initialize()
{
	Super::Initialize();
	
	
	return true;
}

void UHaloHUDWidget::SetHealth_Implementation(float CurrentHealth, float MaxHealth)
{
	if (HealthBar) {
		FNumberFormattingOptions NumberFormattingOptions;
		NumberFormattingOptions.RoundingMode = ERoundingMode::FromZero;
		HealthBar->SetPercent(CurrentHealth / MaxHealth);
		HealthNum->SetText(FText::AsNumber(CurrentHealth, &NumberFormattingOptions));
	}
}

void UHaloHUDWidget::SetShields_Implementation(float CurrentShields, float MaxShields)
{
	if (ShieldBar) {
		FNumberFormattingOptions NumberFormattingOptions;
		NumberFormattingOptions.RoundingMode = ERoundingMode::FromZero;
		ShieldBar->SetPercent(CurrentShields / MaxShields);
		ShieldNum->SetText(FText::AsNumber(CurrentShields, &NumberFormattingOptions));
	}
}



void UHaloHUDWidget::SetCanInteract_Implementation(bool CanInteract)
{
	if (CanInteract)
		InteractBoxWidget->SetVisibility(ESlateVisibility::Visible);
	else
		InteractBoxWidget->SetVisibility(ESlateVisibility::Hidden);
}
void UHaloHUDWidget::SetFragCounter_Implementation(int32 NewFragCount)
{
	if (FragCounter)
		FragCounter->SetText(FText::AsNumber(NewFragCount));
}

void UHaloHUDWidget::SetPlasmaCounter_Implementation(int32 NewPlasmaCount)
{
	if (PlasmaCounter)
		PlasmaCounter->SetText(FText::AsNumber(NewPlasmaCount));
}

void UHaloHUDWidget::SetSpikeCounter_Implementation(int32 NewSpikeCount)
{
	if (SpikeCounter)
		SpikeCounter->SetText(FText::AsNumber(NewSpikeCount));
}

void UHaloHUDWidget::SetIncenCounter_Implementation(int32 NewIncenCount)
{
	if (IncenCounter)
		IncenCounter->SetText(FText::AsNumber(NewIncenCount));
}


