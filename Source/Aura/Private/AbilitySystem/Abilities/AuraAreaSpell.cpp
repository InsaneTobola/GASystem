#include "AbilitySystem/Abilities/AuraAreaSpell.h"

#include "Actor/AuraAreaSpellField.h"
#include "AbilitySystem/AuraAttributeSet.h"
#include "AuraGameplayTags.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"

#include "Character/AuraCharacter.h"
#include "Magic/UAuraMagicComponent.h"
#include "Magic/MagicPatternDefinition.h"
#include "Magic/MagicDebug.h"

#include "Player/AuraPlayerController.h"
#include "Engine/World.h"

UAuraAreaSpell::UAuraAreaSpell()
{
    InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerExecution;

    NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::ServerOnly;
}

void UAuraAreaSpell::RejectMagicCast(const FGameplayAbilityActorInfo* ActorInfo) const
{
    AAuraCharacter* Character = ActorInfo ? Cast<AAuraCharacter>(ActorInfo->AvatarActor.Get()): nullptr;

    // Clear any pending cast data on the server.
    if (IsValid(Character) &&IsValid(Character->MagicComponent))
    {
        Character->MagicComponent->ClearPendingCast();
    }

    // Find the owning player controller.
    AAuraPlayerController* PC = nullptr;

    if (ActorInfo)
    {
        PC = Cast<AAuraPlayerController>(ActorInfo->PlayerController.Get());
    }

    // Fallback: get the controller directly from the character.
    if (!IsValid(PC) && IsValid(Character))
    {
        PC = Cast<AAuraPlayerController>(Character->GetController());
    }

    // Notify the client so it stops waiting for a response.
    if (IsValid(PC))
    {
        PC->ClientMagicTargetRejected();
    }
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

    return FMath::Clamp(BaseDurationSeconds + FMath::Max(DamageOnTime, 0.f) *FMath::Max(SecondsPerDamageOnTimePoint, 0.f),BaseDurationSeconds,MaxDurationSeconds);
}

void UAuraAreaSpell::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
    AuraMagicDebugPrint(
    FString::Printf(
        TEXT("AREA SPELL: ActivateAbility started. Class=%s"),
        *GetNameSafe(GetClass())),FColor::Cyan,5.f);
    
    Super::ActivateAbility(Handle,ActorInfo,ActivationInfo,TriggerEventData);

    if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
    {
        AuraMagicDebugPrint(
        TEXT("AREA SPELL FAILED: CommitAbility failed (cost, cooldown or activation rules)."),FColor::Red);
        RejectMagicCast(ActorInfo);
        EndAbility(Handle,ActorInfo,ActivationInfo,true,true);
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
        AuraMagicDebugPrint(
      FString::Printf(
          TEXT("AREA SPELL FAILED: Character=%d MagicComponent=%d ASC=%d DamageEffect=%s FieldClass=%s"),
          IsValid(Character) ? 1 : 0,
          IsValid(Character) && IsValid(Character->MagicComponent) ? 1 : 0,
          IsValid(SourceASC) ? 1 : 0,
          *GetNameSafe(DamageEffectClass.Get()),
          *GetNameSafe(AreaFieldClass.Get())
      ),
      FColor::Red,
      6.f
  );
        EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
        return;
    }

    FName PatternId = NAME_None;
    FVector TargetLocation = FVector::ZeroVector;

    if (!Character->MagicComponent->ConsumePendingCast(PatternId,TargetLocation))
    {
        AuraMagicDebugPrint(
         TEXT("AREA SPELL FAILED: No pending pattern/target data."),
         FColor::Red
     );
        RejectMagicCast(ActorInfo);
        EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
        return;
    }
    
    AuraMagicDebugPrint(
        FString::Printf(
            TEXT("AREA SPELL: Pending cast received. Pattern=%s Target=(%.0f, %.0f, %.0f)"),
            *PatternId.ToString(),
            TargetLocation.X,
            TargetLocation.Y,
            TargetLocation.Z
        ),
        FColor::Cyan);
    
    UMagicPatternDefinition* Pattern = Character->MagicComponent->FindPatternDefinition(PatternId);

    if (!IsValid(Pattern) || Pattern->SkillAbilityClass != GetClass())
    {
        RejectMagicCast(ActorInfo);
        EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
        return;
    }

    const FAuraGameplayTags& Tags = FAuraGameplayTags::Get();

    FGameplayTag ExpectedDamageTag;

    switch (Pattern->Element)
    {
    case EMagicElement::Fire:ExpectedDamageTag = Tags.Damage_Fire;
        break;
    case EMagicElement::Water:ExpectedDamageTag = Tags.Damage_Water;
        break;
    case EMagicElement::Earth:ExpectedDamageTag = Tags.Damage_Earth;
        break;
    case EMagicElement::Air:ExpectedDamageTag = Tags.Damage_Air;
        break;

    default:break;
    }

    if (!ExpectedDamageTag.IsValid() || !DamageTypes.Contains(ExpectedDamageTag))
    {
        AuraMagicDebugPrint(FString::Printf(
                TEXT("AREA SPELL FAILED: Pattern %s requires damage ""type %s, but it is not configured in DamageTypes."),*Pattern->PatternId.ToString(),*ExpectedDamageTag.ToString()),
            FColor::Red,6.f
        );

        EndAbility(Handle,ActorInfo,ActivationInfo,true,true);
        return;
    }
    
    if (FVector::Dist2D(Avatar->GetActorLocation(),TargetLocation) > MaxRangeCm)
    {
        RejectMagicCast(ActorInfo);
        EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
        return;
    }

    const int32 AbilityLevel = GetAbilityLevel();
    TMap<FGameplayTag, float> ScaledDamageValues;

    for (const TPair<FGameplayTag, FScalableFloat>& Pair : DamageTypes)
    {
        ScaledDamageValues.Add(Pair.Key,Pair.Value.GetValueAtLevel(AbilityLevel));
    }
    AuraMagicDebugPrint(
        FString::Printf(
            TEXT("AREA SPELL: DamageEffect=%s | Damage entries=%d | AbilityLevel=%d"),
            *GetNameSafe(DamageEffectClass.Get()),
            ScaledDamageValues.Num(),
            AbilityLevel
        ),
        ScaledDamageValues.Num() > 0 ? FColor::Green : FColor::Red,
        5.f
    );

    for (const TPair<FGameplayTag, float>& Pair : ScaledDamageValues)
    {
        AuraMagicDebugPrint(
            FString::Printf(
                TEXT("DAMAGE CONFIG: %s = %.2f"),
                *Pair.Key.ToString(),
                Pair.Value
            ),
            FColor::Yellow,
            3.f
        );
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
        RejectMagicCast(ActorInfo);
        EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
        return;
    }

    const float Radius = CalculateAreaRadius(SourceASC);
    const float Duration = CalculateDurationSeconds(SourceASC);

    Field->InitializeField(
        Radius,
        Duration,
        DamageEffectClass,
        KnockbackEffectClass,
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

    AuraMagicDebugPrint(
    FString::Printf(
        TEXT("AREA SPELL SUCCESS: Field=%s | Radius=%.1f cm | Duration=%.2f s"),
        *GetNameSafe(Field),
        Radius,
        Duration
    ),
    FColor::Green,
    6.f
);

    if (AAuraPlayerController* PC =
        Cast<AAuraPlayerController>(Character->GetController()))
    {
        PC->ClientMagicTargetAccepted();
    }
    
    EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
}


