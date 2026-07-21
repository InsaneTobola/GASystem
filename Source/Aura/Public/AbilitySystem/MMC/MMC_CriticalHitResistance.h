// Copyright Druid Mechanics

#pragma once

#include "CoreMinimal.h"
#include "GameplayModMagnitudeCalculation.h"
#include "AbilitySystem/MMC/MMC_BaseClass.h"

#include "MMC_CriticalHitResistance.generated.h"

/**
 * 
 */
UCLASS()
class AURA_API UMMC_CriticalHitResistance : public UMMC_Base
{
	GENERATED_BODY()
public:
	UMMC_CriticalHitResistance();
	
	virtual float CalculateBaseMagnitude_Implementation(const FGameplayEffectSpec& Spec)const override;
	
private:
	
	FGameplayEffectAttributeCaptureDefinition EarthDef;
	
};
