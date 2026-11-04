#include "AbilitySystem/Abilities/AuraVolcanicArcsSpell.h"

#include "Actor/AuraVolcanicArcsField.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystem/AuraAttributeSet.h"
#include "AuraGameplayTags.h"

#include "Character/AuraCharacter.h"
#include "Magic/UAuraMagicComponent.h"
#include "Magic/MagicPatternDefinition.h"
#include "Magic/MagicDebug.h"

#include "Player/AuraPlayerController.h"
#include "Engine/World.h"
#include "GameFramework/Pawn.h"
#include "Abilities/GameplayAbility.h"

UAuraVolcanicArcsSpell::UAuraVolcanicArcsSpell()
{
	// InstancingPolicy and NetExecutionPolicy
	// are inherited from UAuraAreaSpell.
}

void UAuraVolcanicArcsSpell::RejectVolcanicArcsCast(const FGameplayAbilityActorInfo* ActorInfo) const
{
	AAuraCharacter* Character = ActorInfo? Cast<AAuraCharacter>(ActorInfo->AvatarActor.Get()): nullptr;

	if (IsValid(Character) && IsValid(Character->MagicComponent))
	{
		Character->MagicComponent->ClearPendingCast();
	}

	AAuraPlayerController* PC = ActorInfo? Cast<AAuraPlayerController>(ActorInfo->PlayerController.Get()): nullptr;

	if (!IsValid(PC) && IsValid(Character))
	{
		PC = Cast<AAuraPlayerController>(Character->GetController());
	}

	if (IsValid(PC))
	{
		PC->ClientMagicTargetRejected();
	}
}

void UAuraVolcanicArcsSpell::ActivateAbility(
    const FGameplayAbilitySpecHandle Handle,
    const FGameplayAbilityActorInfo* ActorInfo,
    const FGameplayAbilityActivationInfo ActivationInfo,
    const FGameplayEventData* TriggerEventData)
{
    // Do not call Super::ActivateAbility here.
    // UAuraAreaSpell would create a normal circular field.
    UGameplayAbility::ActivateAbility(Handle,ActorInfo,ActivationInfo,TriggerEventData);
    AActor* Avatar = GetAvatarActorFromActorInfo();
    AAuraCharacter* Character = Cast<AAuraCharacter>(Avatar);
    UAbilitySystemComponent* SourceASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(Avatar);

    if (!IsValid(Character) ||
        !IsValid(Character->MagicComponent) ||
        !IsValid(SourceASC) ||
        !VolcanicArcsFieldClass ||
        !DamageEffectClass)
    {
        RejectVolcanicArcsCast(ActorInfo);
        EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
        return;
    }

    FName PatternId = NAME_None;
    FVector TargetLocation = FVector::ZeroVector;
    float ArcScale = 1.f;

    if (!Character->MagicComponent->ConsumePendingCast(PatternId,TargetLocation,&ArcScale))
    {
        RejectVolcanicArcsCast(ActorInfo);
        EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
        return;
    }

    UMagicPatternDefinition* Pattern = Character->MagicComponent->FindPatternDefinition(PatternId);

    if (!IsValid(Pattern) || Pattern->SkillAbilityClass != GetClass() || Pattern->Element != EMagicElement::Fire)
    {
        RejectVolcanicArcsCast(ActorInfo);
        EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
        return;
    }

    const FGameplayTag FireDamageTag = FAuraGameplayTags::Get().Damage_Fire;

    if (!FireDamageTag.IsValid() || !DamageTypes.Contains(FireDamageTag))
    {
        RejectVolcanicArcsCast(ActorInfo);
        EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
        return;
    }

    if (!FMath::IsFinite(ArcScale) || ArcScale < MinArcLengthScale || ArcScale > MaxArcLengthScale)
    {
        RejectVolcanicArcsCast(ActorInfo);
        EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
        return;
    }

    if (FVector::Dist2D(Avatar->GetActorLocation(),TargetLocation) > GetMaxRangeCm())
    {
        RejectVolcanicArcsCast(ActorInfo);
        EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
        return;
    }

    // All configuration and target checks passed.
    // Commit mana cost and cooldown now.
    if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
    {
        RejectVolcanicArcsCast(ActorInfo);
        EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
        return;
    }

    const int32 AbilityLevel = GetAbilityLevel();

    TMap<FGameplayTag, float> ScaledDamageValues;

    for (const TPair<FGameplayTag, FScalableFloat>& Pair : DamageTypes)
    {
        ScaledDamageValues.Add(Pair.Key,Pair.Value.GetValueAtLevel(AbilityLevel));
    }

    const FVector Forward = (TargetLocation - Avatar->GetActorLocation()).GetSafeNormal2D();
    const FRotator FieldRotation(0.f,Forward.Rotation().Yaw,0.f);
    const FTransform SpawnTransform(FieldRotation,TargetLocation + FVector(0.f, 0.f, 3.f));

    AAuraVolcanicArcsField* Field = GetWorld()->SpawnActorDeferred<AAuraVolcanicArcsField>(
            VolcanicArcsFieldClass,
            SpawnTransform,
            Avatar,
            Cast<APawn>(Avatar),
            ESpawnActorCollisionHandlingMethod::AlwaysSpawn
        );

    if (!IsValid(Field))
    {
        RejectVolcanicArcsCast(ActorInfo);
        EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
        return;
    }

    Field->InitializeField(
        BaseHalfLengthCm,
        ArcScale,
        ArcCollisionRadiusCm,
        IntersectionRadiusCm,
        CalculateDurationSeconds(SourceASC),
        DamageEffectClass,
        ScaledDamageValues,
        AbilityLevel,
        this,
        Avatar
    );

    Field->FinishSpawning(SpawnTransform);

    if (AAuraPlayerController* PC = Cast<AAuraPlayerController>(Character->GetController()))
    {
        PC->ClientMagicTargetAccepted();
    }

    EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
}