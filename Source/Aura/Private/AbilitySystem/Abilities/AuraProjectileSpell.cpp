// Copyright Druid Mechanics

#include "AbilitySystem/Abilities/AuraProjectileSpell.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "Actor/AuraProjectile.h"
#include "Engine/World.h"
#include "GameFramework/Pawn.h"
#include "Interaction/CombatInterface.h"

/*
 * Core Mechanisms
 */

// Projectile
void UAuraProjectileSpell::SpawnProjectile(
    const FVector& ProjectileTargetLocation
)
{
    AActor* Avatar = GetAvatarActorFromActorInfo();

    if (!IsValid(Avatar) || !Avatar->HasAuthority())
    {
        return;
    }

    if (!ProjectileClass || !DamageEffectClass)
    {
        UE_LOG(
            LogTemp,
            Error,
            TEXT("[%s] ProjectileClass or DamageEffectClass is not assigned."),
            *GetName()
        );
        return;
    }

    ICombatInterface* CombatInterface = Cast<ICombatInterface>(Avatar);

    if (!CombatInterface)
    {
        UE_LOG(
            LogTemp,
            Error,
            TEXT("[%s] Avatar does not implement CombatInterface."),
            *GetName()
        );
        return;
    }

    UAbilitySystemComponent* SourceASC =
        UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(Avatar);

    if (!IsValid(SourceASC))
    {
        UE_LOG(
            LogTemp,
            Error,
            TEXT("[%s] Source AbilitySystemComponent is invalid."),
            *GetName()
        );
        return;
    }

    UWorld* World = GetWorld();

    if (!IsValid(World))
    {
        return;
    }

    const FVector SocketLocation = CombatInterface->GetCombatSocketLocation();

    FRotator Rotation = (ProjectileTargetLocation - SocketLocation).Rotation();
    Rotation.Pitch = 0.f;

    FTransform SpawnTransform;
    SpawnTransform.SetLocation(SocketLocation);
    SpawnTransform.SetRotation(Rotation.Quaternion());

    AAuraProjectile* Projectile = World->SpawnActorDeferred<AAuraProjectile>(
        ProjectileClass,
        SpawnTransform,
        Avatar,
        Cast<APawn>(Avatar),
        ESpawnActorCollisionHandlingMethod::AlwaysSpawn
    );

    if (!IsValid(Projectile))
    {
        UE_LOG(
            LogTemp,
            Error,
            TEXT("[%s] Failed to spawn projectile class %s."),
            *GetName(),
            *GetNameSafe(ProjectileClass.Get())
        );
        return;
    }

    FGameplayEffectContextHandle EffectContextHandle = SourceASC->MakeEffectContext();
    EffectContextHandle.SetAbility(this);
    EffectContextHandle.AddSourceObject(Projectile);

    TArray<TWeakObjectPtr<AActor>> Actors;
    Actors.Add(Projectile);
    EffectContextHandle.AddActors(Actors);

    FHitResult HitResult;
    HitResult.Location = ProjectileTargetLocation;
    EffectContextHandle.AddHitResult(HitResult);

    const int32 AbilityLevel = GetAbilityLevel();
    const FGameplayEffectSpecHandle SpecHandle = SourceASC->MakeOutgoingSpec(
        DamageEffectClass,
        AbilityLevel,
        EffectContextHandle
    );

    if (!SpecHandle.IsValid() || !SpecHandle.Data.IsValid())
    {
        UE_LOG(
            LogTemp,
            Error,
            TEXT("[%s] Failed to create projectile damage GameplayEffectSpec."),
            *GetName()
        );

        Projectile->Destroy();
        return;
    }

    TMap<FGameplayTag, float> ScaledDamageValues;
    BuildScaledDamageValues(AbilityLevel, ScaledDamageValues);

    for (const TPair<FGameplayTag, float>& DamageEntry : ScaledDamageValues)
    {
        UAbilitySystemBlueprintLibrary::AssignTagSetByCallerMagnitude(
            SpecHandle,
            DamageEntry.Key,
            DamageEntry.Value
        );
    }

    Projectile->DamageEffectSpecHandle = SpecHandle;
    Projectile->FinishSpawning(SpawnTransform);
}