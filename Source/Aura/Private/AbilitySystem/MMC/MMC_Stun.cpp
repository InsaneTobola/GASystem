// Copyright Druid Mechanics


#include "AbilitySystem/MMC/MMC_Stun.h"
#include "AbilitySystem/AuraAttributeSet.h"

UMMC_Stun::UMMC_Stun()
{
	AddAttributeCapture(EarthDef, UAuraAttributeSet::GetEarthAttribute());
}

float UMMC_Stun::CalculateBaseMagnitude_Implementation(const FGameplayEffectSpec& Spec) const
{
	const float Earth = GetCapturedValue(EarthDef, Spec);

	return Earth * 0.2f;
}
