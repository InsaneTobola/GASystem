// Copyright Druid Mechanics


#include "AbilitySystem/MMC/MMC_AreaOfEffect.h"
#include "AbilitySystem/AuraAttributeSet.h"

UMMC_AreaOfEffect::UMMC_AreaOfEffect()
{
	AddAttributeCapture(WaterDef, UAuraAttributeSet::GetWaterAttribute());
	AddAttributeCapture(AirDef, UAuraAttributeSet::GetAirAttribute());
}

float UMMC_AreaOfEffect::CalculateBaseMagnitude_Implementation(const FGameplayEffectSpec& Spec) const
{
	const float Water = GetCapturedValue(WaterDef, Spec);
	const float Air = GetCapturedValue(AirDef, Spec);

	return (Water + Air) * 0.2f;
}


