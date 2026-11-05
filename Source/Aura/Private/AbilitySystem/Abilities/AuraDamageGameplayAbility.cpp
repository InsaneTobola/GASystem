// Copyright Druid Mechanics

#include "AbilitySystem/Abilities/AuraDamageGameplayAbility.h"

/*
 * Core Mechanisms
 */

// Damage
void UAuraDamageGameplayAbility::BuildScaledDamageValues(
	int32 AbilityLevel,
	TMap<FGameplayTag, float>& OutDamageValues
) const
{
	OutDamageValues.Reset();

	for (const TPair<FGameplayTag, FScalableFloat>& DamageEntry : DamageTypes)
	{
		OutDamageValues.Add(
			DamageEntry.Key,
			DamageEntry.Value.GetValueAtLevel(AbilityLevel)
		);
	}
}

