// Copyright Druid Mechanics

#pragma once

#include "CoreMinimal.h"
#include "GameplayModMagnitudeCalculation.h"
#include "AbilitySystem/MMC/MMC_BaseClass.h"
#include "MMC_ArmorPenetration.generated.h"


/**
 * 
 */
UCLASS()
class AURA_API UMMC_ArmorPenetration : public UMMC_Base
{
	GENERATED_BODY()
public:
	UMMC_ArmorPenetration();
	
	virtual float CalculateBaseMagnitude_Implementation(const FGameplayEffectSpec& Spec)const override;
	
private:
	
	FGameplayEffectAttributeCaptureDefinition WaterDef;
	FGameplayEffectAttributeCaptureDefinition AirDef;
	FGameplayEffectAttributeCaptureDefinition ArmorDef;
};
