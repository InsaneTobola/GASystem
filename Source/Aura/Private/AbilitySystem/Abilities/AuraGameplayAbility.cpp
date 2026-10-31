#include "AbilitySystem/Abilities/AuraGameplayAbility.h"

#include "AbilitySystem/AuraAttributeSet.h"
#include "AbilitySystemComponent.h"
#include "AuraGameplayTags.h"
#include "GameplayEffect.h"

bool UAuraGameplayAbility::CheckCost(
    const FGameplayAbilitySpecHandle Handle,
    const FGameplayAbilityActorInfo* ActorInfo,
    FGameplayTagContainer* OptionalRelevantTags
) const
{
    if (!Super::CheckCost(Handle, ActorInfo, OptionalRelevantTags))
    {
        return false;
    }

    const float Cost = FMath::Max(0.f,ManaCost.GetValueAtLevel(GetAbilityLevel(Handle, ActorInfo)));

    // Zero means the ability is free.
    if (Cost <= KINDA_SMALL_NUMBER)
    {
        return true;
    }

    UAbilitySystemComponent* ASC = ActorInfo? ActorInfo->AbilitySystemComponent.Get(): nullptr;

    if (!IsValid(ASC) || !ManaCostEffectClass)
    {
        UE_LOG(
            LogTemp,
            Error,
            TEXT("[%s] Mana cost is configured, but ASC or ManaCostEffectClass is missing."),
            *GetName()
        );

        if (OptionalRelevantTags)
        {
            OptionalRelevantTags->AddTag(FAuraGameplayTags::Get().Ability_Fail_Configuration);
        }

        return false;
    }

    const float CurrentMana = ASC->GetNumericAttribute(UAuraAttributeSet::GetManaAttribute());

    if (CurrentMana + KINDA_SMALL_NUMBER < Cost)
    {
        if (OptionalRelevantTags)
        {
            OptionalRelevantTags->AddTag(FAuraGameplayTags::Get().Ability_Fail_Cost_Mana
            );
        }

        return false;
    }

    return true;
}

void UAuraGameplayAbility::ApplyCost(const FGameplayAbilitySpecHandle Handle,const FGameplayAbilityActorInfo* ActorInfo,const FGameplayAbilityActivationInfo ActivationInfo) const
{
    Super::ApplyCost(Handle, ActorInfo, ActivationInfo);

    const float Cost = FMath::Max(0.f,ManaCost.GetValueAtLevel(GetAbilityLevel(Handle, ActorInfo)));

    if (Cost <= KINDA_SMALL_NUMBER)
    {
        return;
    }

    UAbilitySystemComponent* ASC = ActorInfo? ActorInfo->AbilitySystemComponent.Get(): nullptr;

    if (!IsValid(ASC) || !ManaCostEffectClass)
    {
        return;
    }

    FGameplayEffectContextHandle ContextHandle = ASC->MakeEffectContext();

    ContextHandle.SetAbility(const_cast<UAuraGameplayAbility*>(this));
    ContextHandle.AddSourceObject(ActorInfo->AvatarActor.Get());

    const FGameplayEffectSpecHandle SpecHandle = ASC->MakeOutgoingSpec(ManaCostEffectClass,GetAbilityLevel(Handle, ActorInfo),ContextHandle);

    if (!SpecHandle.IsValid() || !SpecHandle.Data.IsValid())
    {
        UE_LOG(
            LogTemp,
            Error,
            TEXT("[%s] Failed to create mana cost GameplayEffectSpec."),
            *GetName()
        );

        return;
    }

    SpecHandle.Data->SetSetByCallerMagnitude(FAuraGameplayTags::Get().Data_ManaCost,-Cost);

    ASC->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());
}

bool UAuraGameplayAbility::CheckCooldown(const FGameplayAbilitySpecHandle Handle,const FGameplayAbilityActorInfo* ActorInfo,FGameplayTagContainer* OptionalRelevantTags) const
{
    if (!Super::CheckCooldown(Handle, ActorInfo, OptionalRelevantTags))
    {
        return false;
    }

    UAbilitySystemComponent* ASC = ActorInfo? ActorInfo->AbilitySystemComponent.Get(): nullptr;

    if (!IsValid(ASC))
    {
        return false;
    }

    // A still-active cooldown always blocks reuse.
    if (AbilityCooldownTag.IsValid() && ASC->HasMatchingGameplayTag(AbilityCooldownTag))
    {
        if (OptionalRelevantTags)
        {
            OptionalRelevantTags->AddTag(FAuraGameplayTags::Get().Ability_Fail_Cooldown);
        }

        return false;
    }

    const float Duration = FMath::Max(0.f,CooldownDuration.GetValueAtLevel(GetAbilityLevel(Handle, ActorInfo)));

    // Zero means no cooldown.
    if (Duration <= KINDA_SMALL_NUMBER)
    {
        return true;
    }

    if (!AbilityCooldownTag.IsValid() || !AbilityCooldownEffectClass)
    {
        UE_LOG(
            LogTemp,
            Error,
            TEXT("[%s] Cooldown duration is positive, but its tag or effect class is missing."),
            *GetName()
        );

        if (OptionalRelevantTags)
        {
            OptionalRelevantTags->AddTag(FAuraGameplayTags::Get().Ability_Fail_Configuration
            );
        }

        return false;
    }

    return true;
}

void UAuraGameplayAbility::ApplyCooldown(const FGameplayAbilitySpecHandle Handle,const FGameplayAbilityActorInfo* ActorInfo,const FGameplayAbilityActivationInfo ActivationInfo) const
{
    Super::ApplyCooldown(Handle, ActorInfo, ActivationInfo);

    const float Duration = FMath::Max(0.f,CooldownDuration.GetValueAtLevel(GetAbilityLevel(Handle, ActorInfo)));

    if (Duration <= KINDA_SMALL_NUMBER)
    {
        return;
    }

    UAbilitySystemComponent* ASC = ActorInfo? ActorInfo->AbilitySystemComponent.Get(): nullptr;

    if (!IsValid(ASC) || !AbilityCooldownEffectClass || !AbilityCooldownTag.IsValid())
    {
        return;
    }

    FGameplayEffectContextHandle ContextHandle = ASC->MakeEffectContext();
    ContextHandle.SetAbility(const_cast<UAuraGameplayAbility*>(this));
    ContextHandle.AddSourceObject(ActorInfo->AvatarActor.Get());

    const FGameplayEffectSpecHandle SpecHandle = ASC->MakeOutgoingSpec(AbilityCooldownEffectClass,GetAbilityLevel(Handle, ActorInfo),ContextHandle);

    if (!SpecHandle.IsValid() || !SpecHandle.Data.IsValid())
    {
        UE_LOG(
            LogTemp,
            Error,
            TEXT("[%s] Failed to create cooldown GameplayEffectSpec."),
            *GetName()
        );

        return;
    }

    SpecHandle.Data->SetSetByCallerMagnitude(FAuraGameplayTags::Get().Data_Cooldown_Duration,Duration);

    SpecHandle.Data->DynamicGrantedTags.AddTag(AbilityCooldownTag);
    ASC->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());
}

const FGameplayTagContainer*UAuraGameplayAbility::GetCooldownTags() const
{
    TemporaryCooldownTags.Reset();

    if (const FGameplayTagContainer* ParentTags = Super::GetCooldownTags())
    {
        TemporaryCooldownTags.AppendTags(*ParentTags);
    }

    if (AbilityCooldownTag.IsValid())
    {
        TemporaryCooldownTags.AddTag(AbilityCooldownTag);
    }

    return &TemporaryCooldownTags;
}