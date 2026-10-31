// Copyright Druid Mechanics

#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "GameplayTagContainer.h"
#include "ScalableFloat.h"
#include "AuraGameplayAbility.generated.h"


class UGameplayEffect;
/**
 * 
 */
UCLASS()
class AURA_API UAuraGameplayAbility : public UGameplayAbility
{
	GENERATED_BODY()
public:
	
	UPROPERTY(EditDefaultsOnly, Category = "Input")
	FGameplayTag StartupInputTag;
	
	virtual bool CheckCost(const FGameplayAbilitySpecHandle Handle,const FGameplayAbilityActorInfo* ActorInfo,FGameplayTagContainer* OptionalRelevantTags) const override;
	virtual void ApplyCost(const FGameplayAbilitySpecHandle Handle,const FGameplayAbilityActorInfo* ActorInfo,const FGameplayAbilityActivationInfo ActivationInfo) const override;
	virtual bool CheckCooldown(const FGameplayAbilitySpecHandle Handle,const FGameplayAbilityActorInfo* ActorInfo,FGameplayTagContainer* OptionalRelevantTags) const override;
	virtual void ApplyCooldown(const FGameplayAbilitySpecHandle Handle,const FGameplayAbilityActorInfo* ActorInfo,const FGameplayAbilityActivationInfo ActivationInfo) const override;
	virtual const FGameplayTagContainer* GetCooldownTags() const override;

	FGameplayTag GetAbilityCooldownTag() const
	{
		return AbilityCooldownTag;
	}

	UPROPERTY(EditDefaultsOnly,BlueprintReadOnly,Category = "Costs",meta = (ClampMin = "0.0"))
	FScalableFloat ManaCost;

	UPROPERTY(EditDefaultsOnly,BlueprintReadOnly,Category = "Costs")
	TSubclassOf<UGameplayEffect> ManaCostEffectClass;

	UPROPERTY(EditDefaultsOnly,BlueprintReadOnly,Category = "Cooldown",meta = (ClampMin = "0.0"))
	FScalableFloat CooldownDuration;

	UPROPERTY(EditDefaultsOnly,BlueprintReadOnly,Category = "Cooldown")
	FGameplayTag AbilityCooldownTag;

	UPROPERTY(EditDefaultsOnly,BlueprintReadOnly,Category = "Cooldown")
	TSubclassOf<UGameplayEffect> AbilityCooldownEffectClass;

private:

	mutable FGameplayTagContainer TemporaryCooldownTags;
};
