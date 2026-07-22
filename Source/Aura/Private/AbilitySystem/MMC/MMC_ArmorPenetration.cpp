// Copyright Druid Mechanics


#include "AbilitySystem/MMC/MMC_ArmorPenetration.h"
#include "AbilitySystem/AuraAttributeSet.h"

UMMC_ArmorPenetration::UMMC_ArmorPenetration()
{
	AddAttributeCapture(WaterDef, UAuraAttributeSet::GetWaterAttribute());
	AddAttributeCapture(AirDef, UAuraAttributeSet::GetAirAttribute());
	AddAttributeCapture(ArmorDef, UAuraAttributeSet::GetArmorAttribute());
}

float UMMC_ArmorPenetration::CalculateBaseMagnitude_Implementation(const FGameplayEffectSpec& Spec) const
{
	const float Water = GetCapturedValue(WaterDef, Spec);
	const float Air = GetCapturedValue(AirDef, Spec);
	const float Armor = GetCapturedValue(ArmorDef, Spec);
	
	return FMath::Max(Armor - ((Water * Air) / 10.f), 0.f);
}
