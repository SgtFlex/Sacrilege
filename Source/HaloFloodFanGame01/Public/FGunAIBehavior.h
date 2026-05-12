#pragma once

#include "CoreMinimal.h"
#include "FGunAIBehavior.generated.h"

USTRUCT(BlueprintType)
struct FGunAIBehavior
{
	GENERATED_BODY()
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Range = 3000;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	uint8 AmountOfProjectilesToFire = 5;

	//Delay between Trigger pulls
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float DelayBetweenBursts = 1.0f;

	//Degrees in spread for AI
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Spread = 30.0f;

	//Projectile lead compensation

	//Projectile drop compensation
};
