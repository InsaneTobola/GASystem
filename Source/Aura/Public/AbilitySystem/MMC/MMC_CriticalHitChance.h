// Copyright Druid Mechanics

#pragma once

#include "CoreMinimal.h"
#include "GameplayModMagnitudeCalculation.h"
#include "AbilitySystem/MMC/MMC_BaseClass.h"

#include "MMC_CriticalHitChance.generated.h"


/**
 * 
 */
UCLASS()
class AURA_API UMMC_CriticalHitChance : public UMMC_Base
{
	GENERATED_BODY()
public:
	UMMC_CriticalHitChance();
	
	virtual float CalculateBaseMagnitude_Implementation(const FGameplayEffectSpec& Spec)const override;
	
private:
	
	FGameplayEffectAttributeCaptureDefinition FireDef;
	FGameplayEffectAttributeCaptureDefinition AirDef;
};
