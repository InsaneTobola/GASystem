// Copyright Druid Mechanics


#include "AbilitySystem/MMC/MMC_ArmorPenetration.h"
#include "AbilitySystem/AuraAttributeSet.h"

UMMC_ArmorPenetration::UMMC_ArmorPenetration()
{
	AddAttributeCapture(EarthDef, UAuraAttributeSet::GetEarthAttribute());
	AddAttributeCapture(WaterDef, UAuraAttributeSet::GetWaterAttribute());
	AddAttributeCapture(AirDef, UAuraAttributeSet::GetAirAttribute());
}

float UMMC_ArmorPenetration::CalculateBaseMagnitude_Implementation(const FGameplayEffectSpec& Spec) const
{
	const float Earth = GetCapturedValue(EarthDef, Spec);
	const float Water = GetCapturedValue(WaterDef, Spec);
	const float Air = GetCapturedValue(AirDef, Spec);

	// Armor is derived from Earth in GE_AuraSecondaryAttributes.
	const float Armor = Earth * 2.5f;

	return FMath::Max(Armor - ((Water * Air) / 10.f), 0.f);
}
