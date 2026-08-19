// Copyright Druid Mechanics

#pragma once

#include "CoreMinimal.h"
#include "AttackDefinition.h"
#include "Engine/DataAsset.h"
#include "CombatProfileInfo.generated.h"


/**
 * 
 */
UCLASS()
class AURA_API UCombatProfileInfo : public UPrimaryDataAsset
{
	GENERATED_BODY()
public:

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combat")
	TArray<TObjectPtr<UAttackDefinition>> Attacks;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combat")
	float AttackRange = 180.f;
};