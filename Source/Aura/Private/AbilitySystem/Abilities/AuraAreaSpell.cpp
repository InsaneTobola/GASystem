#include "AbilitySystem/Abilities/AuraAreaSpell.h"

#include "Actor/AuraAreaSpellField.h"
#include "AbilitySystem/AuraAttributeSet.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"

#include "Character/AuraCharacter.h"
#include "Magic/UAuraMagicComponent.h"
#include "Magic/MagicPatternDefinition.h"

#include "Engine/World.h"

UAuraAreaSpell::UAuraAreaSpell()
{
    InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerExecution;

    NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::ServerOnly;
}

float UAuraAreaSpell::GetMaxRangeCm() const
{
    return MaxRangeCm;
}

float UAuraAreaSpell::CalculateAreaRadius(const UAbilitySystemComponent* SourceASC) const
{
    float AreaOfEffect = 0.f;

    if (IsValid(SourceASC))
    {
        AreaOfEffect = SourceASC->GetNumericAttribute(UAuraAttributeSet::GetAreaOfEffectAttribute());
    }

    const float Scale =1.f + FMath::Max(AreaOfEffect, 0.f) *FMath::Max(AreaOfEffectScalePerPoint, 0.f);
    const float Diameter = FMath::Clamp(BaseAreaDiameterCm * Scale,MinAreaDiameterCm,MaxAreaDiameterCm);
    return Diameter * 0.5f;
}

float UAuraAreaSpell::CalculateDurationSeconds(const UAbilitySystemComponent* SourceASC) const
{
    float DamageOnTime = 0.f;

    if (IsValid(SourceASC))
    {
        DamageOnTime = SourceASC->GetNumericAttribute(UAuraAttributeSet::GetDamageOnTimeAttribute());
    }

    return FMath::Clamp(
        BaseDurationSeconds +
        FMath::Max(DamageOnTime, 0.f) *
        FMath::Max(SecondsPerDamageOnTimePoint, 0.f),
        BaseDurationSeconds,
        MaxDurationSeconds
    );
}

void UAuraAreaSpell::ActivateAbility(
    const FGameplayAbilitySpecHandle Handle,
    const FGameplayAbilityActorInfo* ActorInfo,
    const FGameplayAbilityActivationInfo ActivationInfo,
    const FGameplayEventData* TriggerEventData)
{
    Super::ActivateAbility(Handle,ActorInfo,ActivationInfo,TriggerEventData);

    if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
    {
        EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
        return;
    }

    AActor* Avatar = GetAvatarActorFromActorInfo();
    AAuraCharacter* Character = Cast<AAuraCharacter>(Avatar);
    UAbilitySystemComponent* SourceASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(Avatar);

    if (!IsValid(Character) ||
        !IsValid(Character->MagicComponent) ||
        !IsValid(SourceASC) ||
        !DamageEffectClass ||
        !AreaFieldClass)
    {
        EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
        return;
    }

    FName PatternId = NAME_None;
    FVector TargetLocation = FVector::ZeroVector;

    if (!Character->MagicComponent->ConsumePendingCast(PatternId,TargetLocation))
    {
        EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
        return;
    }

    UMagicPatternDefinition* Pattern = Character->MagicComponent->FindPatternDefinition(PatternId);

    if (!IsValid(Pattern) || Pattern->SkillAbilityClass != GetClass())
    {
        EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
        return;
    }

    if (FVector::Dist2D(Avatar->GetActorLocation(),TargetLocation) > MaxRangeCm)
    {
        EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
        return;
    }

    const int32 AbilityLevel = GetAbilityLevel();

    TMap<FGameplayTag, float> ScaledDamageValues;

    for (const TPair<FGameplayTag, FScalableFloat>& Pair : DamageTypes)
    {
        ScaledDamageValues.Add(Pair.Key,Pair.Value.GetValueAtLevel(AbilityLevel));
    }

    TArray<TSubclassOf<UGameplayEffect>> AdditionalEffects;

    if (bEnableSlow && SlowEffectClass)
    {
        AdditionalEffects.Add(SlowEffectClass);
    }

    if (bEnableStun && StunEffectClass)
    {
        AdditionalEffects.Add(StunEffectClass);
    }

    if (bEnableSpeed && SpeedEffectClass)
    {
        AdditionalEffects.Add(SpeedEffectClass);
    }

    if (bEnableHealthRegeneration && HealthRegenerationEffectClass)
    {
        AdditionalEffects.Add(HealthRegenerationEffectClass);
    }

    if (bEnableManaRegeneration && ManaRegenerationEffectClass)
    {
        AdditionalEffects.Add(ManaRegenerationEffectClass);
    }

    FTransform SpawnTransform;
    SpawnTransform.SetLocation(TargetLocation);
    SpawnTransform.SetRotation(FQuat::Identity);

    AAuraAreaSpellField* Field = GetWorld()->SpawnActorDeferred<AAuraAreaSpellField>(
            AreaFieldClass,
            SpawnTransform,
            Avatar,
            Cast<APawn>(Avatar),
            ESpawnActorCollisionHandlingMethod::AlwaysSpawn
        );

    if (!IsValid(Field))
    {
        EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
        return;
    }

    const float Radius = CalculateAreaRadius(SourceASC);
    const float Duration = CalculateDurationSeconds(SourceASC);

    Field->InitializeField(
        Radius,
        Duration,
        DamageEffectClass,
        ScaledDamageValues,
        AbilityLevel,
        this,
        Avatar,
        AdditionalEffects,
        bAffectOnlyEnemies,
        bEnableKnockback,
        KnockbackStrength
    );

    Field->FinishSpawning(SpawnTransform);

    EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
}