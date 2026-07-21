// Copyright Druid Mechanics


#include "AbilitySystem/MMC/MMC_CriticalHitDamage.h"
#include "AbilitySystem/AuraAttributeSet.h"

UMMC_CriticalHitDamage::UMMC_CriticalHitDamage()
{
AddAttributeCapture(FireDef, UAuraAttributeSet::GetFireAttribute());
}

float UMMC_CriticalHitDamage::CalculateBaseMagnitude_Implementation(const FGameplayEffectSpec& Spec) const
{
	const float Fire = GetCapturedValue(FireDef, Spec);
	
	return 0.1f*(Fire/5);
}
