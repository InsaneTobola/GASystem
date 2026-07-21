// Copyright Druid Mechanics


#include "AbilitySystem/MMC/MMC_Speed.h"
#include "AbilitySystem/AuraAttributeSet.h"

UMMC_Speed::UMMC_Speed()
{
	AddAttributeCapture(AirDef, UAuraAttributeSet::GetAirAttribute());
}

float UMMC_Speed::CalculateBaseMagnitude_Implementation(const FGameplayEffectSpec& Spec) const
{
	const float Air = GetCapturedValue(AirDef, Spec);

	return  Air/5.f;
}
