// Copyright Druid Mechanics


#include "AbilitySystem/MMC/MMC_Knockback.h"
#include "AbilitySystem/AuraAttributeSet.h"

UMMC_Knockback::UMMC_Knockback()
{
	AddAttributeCapture(AirDef, UAuraAttributeSet::GetAirAttribute());
}

float UMMC_Knockback::CalculateBaseMagnitude_Implementation(const FGameplayEffectSpec& Spec) const
{
	const float Air = GetCapturedValue(AirDef, Spec);

	return Air * 1.1f;
}
