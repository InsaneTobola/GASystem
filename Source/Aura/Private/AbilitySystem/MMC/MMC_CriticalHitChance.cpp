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

	const float ClampedFire = FMath::Clamp(Fire, 0.f, 40.f);
	const float ClampedAir = FMath::Clamp(Air, 0.f, 40.f);

	const float CombinedElementalPoints = ClampedFire + ClampedAir;

	return FMath::Clamp(0.0125f * FMath::Square(CombinedElementalPoints),0.f,80.f);
}
