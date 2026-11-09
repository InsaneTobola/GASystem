// Copyright Druid Mechanics

#include "AbilitySystem/MMC/MMC_BlockChance.h"
#include "AbilitySystem/AuraAttributeSet.h"

UMMC_BlockChance::UMMC_BlockChance()
{
	AddAttributeCapture(EarthDef, UAuraAttributeSet::GetEarthAttribute());
}

float UMMC_BlockChance::CalculateBaseMagnitude_Implementation(
	const FGameplayEffectSpec& Spec
) const
{
	const float Earth = GetCapturedValue(EarthDef, Spec);
	const float Armor = Earth * 2.5f;

	constexpr float BaseBlockChance = 15.f;
	constexpr float BlockChancePerArmorPoint = 0.25f;
	constexpr float MaxBlockChance = 50.f;

	return FMath::Clamp(
		BaseBlockChance + Armor * BlockChancePerArmorPoint,
		0.f,
		MaxBlockChance
	);
}
