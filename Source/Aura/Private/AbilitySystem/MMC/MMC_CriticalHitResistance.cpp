// Copyright Druid Mechanics


#include "AbilitySystem/MMC/MMC_CriticalHitResistance.h"
#include "AbilitySystem/AuraAttributeSet.h"

UMMC_CriticalHitResistance::UMMC_CriticalHitResistance()
{
	AddAttributeCapture(EarthDef, UAuraAttributeSet::GetEarthAttribute());
}

float UMMC_CriticalHitResistance::CalculateBaseMagnitude_Implementation(const FGameplayEffectSpec& Spec) const
{
	const float Earth = GetCapturedValue(EarthDef, Spec);

	return 0.1f * (Earth/10);
}
