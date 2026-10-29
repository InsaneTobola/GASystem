#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/Abilities/AuraGameplayAbility.h"
#include "AuraSelfCastSpell.generated.h"

class UGameplayEffect;

UCLASS()
class AURA_API UAuraSelfCastSpell : public UAuraGameplayAbility
{
	GENERATED_BODY()

public:
	UAuraSelfCastSpell();

protected:
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle,const FGameplayAbilityActorInfo* ActorInfo,const FGameplayAbilityActivationInfo ActivationInfo,const FGameplayEventData* TriggerEventData) override;

	UPROPERTY(EditDefaultsOnly,BlueprintReadOnly,Category = "Effects|Self Cast")
	TSubclassOf<UGameplayEffect> SelfGameplayEffectClass;
};