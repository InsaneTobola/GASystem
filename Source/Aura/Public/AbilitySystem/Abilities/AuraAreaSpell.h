#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/Abilities/AuraDamageGameplayAbility.h"
#include "AuraAreaSpell.generated.h"

class AAuraAreaSpellField;
class UAbilitySystemComponent;
class UGameplayEffect;

UCLASS()
class AURA_API UAuraAreaSpell : public UAuraDamageGameplayAbility
{
    GENERATED_BODY()

public:
    UAuraAreaSpell();

    float GetMaxRangeCm() const;
    float CalculateAreaRadius(const UAbilitySystemComponent* SourceASC) const;
    float CalculateDurationSeconds(const UAbilitySystemComponent* SourceASC) const;

protected:
    virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle,const FGameplayAbilityActorInfo* ActorInfo,
        const FGameplayAbilityActivationInfo ActivationInfo,const FGameplayEventData* TriggerEventData) override;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Area")
    TSubclassOf<AAuraAreaSpellField> AreaFieldClass;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Targeting", meta = (ClampMin = "1"))
    float MaxRangeCm = 1000.f;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Area", meta = (ClampMin = "1"))
    float BaseAreaDiameterCm = 150.f;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Area")
    float AreaOfEffectScalePerPoint = 0.01f;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Area")
    float MinAreaDiameterCm = 50.f;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Area")
    float MaxAreaDiameterCm = 800.f;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Duration")
    float BaseDurationSeconds = 0.5f;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Duration")
    float SecondsPerDamageOnTimePoint = 0.05f;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Duration")
    float MaxDurationSeconds = 5.f;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Targeting")
    bool bAffectOnlyEnemies = true;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Effects|Knockback")
    bool bEnableKnockback = false;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Effects|Knockback")
    float KnockbackStrength = 600.f;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Effects|Slow")
    bool bEnableSlow = false;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Effects|Slow")
    TSubclassOf<UGameplayEffect> SlowEffectClass;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Effects|Stun")
    bool bEnableStun = false;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Effects|Stun")
    TSubclassOf<UGameplayEffect> StunEffectClass;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Effects|Speed")
    bool bEnableSpeed = false;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Effects|Speed")
    TSubclassOf<UGameplayEffect> SpeedEffectClass;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Effects|Health Regeneration")
    bool bEnableHealthRegeneration = false;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Effects|Health Regeneration")
    TSubclassOf<UGameplayEffect> HealthRegenerationEffectClass;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Effects|Mana Regeneration")
    bool bEnableManaRegeneration = false;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Effects|Mana Regeneration")
    TSubclassOf<UGameplayEffect> ManaRegenerationEffectClass;
};