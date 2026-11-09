// Copyright Druid Mechanics

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/MMC/MMC_BaseClass.h"
#include "MMC_BlockChance.generated.h"

UCLASS()
class AURA_API UMMC_BlockChance : public UMMC_Base
{
	GENERATED_BODY()

public:
	UMMC_BlockChance();

	virtual float CalculateBaseMagnitude_Implementation(
		const FGameplayEffectSpec& Spec
	) const override;

private:
	FGameplayEffectAttributeCaptureDefinition EarthDef;
};
