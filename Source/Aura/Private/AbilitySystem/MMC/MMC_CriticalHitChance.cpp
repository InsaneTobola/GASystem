// Copyright Druid Mechanics


#include "AbilitySystem/MMC/MMC_CriticalHitChance.h"
#include "AbilitySystem/AuraAttributeSet.h"

UMMC_CriticalHitChance::UMMC_CriticalHitChance()
{
	AddAttributeCapture(FireDef, UAuraAttributeSet::GetFireAttribute());
	AddAttributeCapture(AirDef, UAuraAttributeSet::GetAirAttribute());
}

float UMMC_CriticalHitChance::CalculateBaseMagnitude_Implementation(const FGameplayEffectSpec& Spec) const
{
	const float Fire = GetCapturedValue(FireDef, Spec);
	const float Air = GetCapturedValue(AirDef, Spec);

	return 0.1f*((Fire+Air)/20);
}
