// Copyright Druid Mechanics


#include "AbilitySystem/MMC/MMC_Slow.h"
#include "AbilitySystem/AuraAttributeSet.h"

UMMC_Slow::UMMC_Slow()
{
	AddAttributeCapture(WaterDef, UAuraAttributeSet::GetWaterAttribute());
	AddAttributeCapture(EarthDef, UAuraAttributeSet::GetEarthAttribute());
}

float UMMC_Slow::CalculateBaseMagnitude_Implementation(const FGameplayEffectSpec& Spec) const
{
	const float Water = GetCapturedValue(WaterDef, Spec);
	const float Earth = GetCapturedValue(EarthDef, Spec);

	return (Water + Earth) / 10.f;
}
