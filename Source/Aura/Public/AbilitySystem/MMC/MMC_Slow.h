// Copyright Druid Mechanics

#pragma once

#include "CoreMinimal.h"
#include "GameplayModMagnitudeCalculation.h"
#include "AbilitySystem/MMC/MMC_BaseClass.h"

#include "MMC_Slow.generated.h"

/**
 * 
 */
UCLASS()
class AURA_API UMMC_Slow : public UMMC_Base
{
	GENERATED_BODY()
public:
	UMMC_Slow();
	
	virtual float CalculateBaseMagnitude_Implementation(const FGameplayEffectSpec& Spec)const override;
	
private:
	
	FGameplayEffectAttributeCaptureDefinition WaterDef;
	FGameplayEffectAttributeCaptureDefinition EarthDef;
};
