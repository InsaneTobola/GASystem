// Copyright Druid Mechanics

#pragma once

#include "CoreMinimal.h"
#include "GameplayModMagnitudeCalculation.h"
#include "MMC_BaseClass.generated.h"

/**
 * 
 */
UCLASS()
class AURA_API UMMC_Base : public UGameplayModMagnitudeCalculation
{
	GENERATED_BODY()

protected:

	void AddAttributeCapture(
		FGameplayEffectAttributeCaptureDefinition& Def,
		const FGameplayAttribute& Attribute,
		EGameplayEffectAttributeCaptureSource Source =
			EGameplayEffectAttributeCaptureSource::Target,
		bool bSnapshot = false);

	float GetCapturedValue(
		const FGameplayEffectAttributeCaptureDefinition& CaptureDef,
		const FGameplayEffectSpec& Spec) const;
};