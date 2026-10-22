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

	const float ClampedAir = FMath::Clamp(Air, 0.f, 40.f);

	constexpr float BaseSpeed = 200.f;
	constexpr float SpeedPerAirPoint = 5.f;

	return BaseSpeed + ClampedAir * SpeedPerAirPoint;
}
