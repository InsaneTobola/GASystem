#include "AbilitySystem/Abilities/AuraSelfCastSpell.h"

#include "AbilitySystemComponent.h"
#include "GameplayEffect.h"
#include "Player/AuraPlayerController.h"
#include "GameFramework/Pawn.h"

UAuraSelfCastSpell::UAuraSelfCastSpell()
{
    InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerExecution;

    NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::ServerOnly;
}

void UAuraSelfCastSpell::ActivateAbility(const FGameplayAbilitySpecHandle Handle,const FGameplayAbilityActorInfo* ActorInfo,const FGameplayAbilityActivationInfo ActivationInfo,const FGameplayEventData* TriggerEventData)
{
    Super::ActivateAbility(Handle,ActorInfo,ActivationInfo,TriggerEventData);

    const auto NotifyClientResult = [ActorInfo](bool bSucceeded)
    {
        AAuraPlayerController* PC = ActorInfo? Cast<AAuraPlayerController>(ActorInfo->PlayerController.Get()): nullptr;

        if (!IsValid(PC) && ActorInfo)
        {
            APawn* AvatarPawn = Cast<APawn>(ActorInfo->AvatarActor.Get());

            if (IsValid(AvatarPawn))
            {
                PC = Cast<AAuraPlayerController>(AvatarPawn->GetController());
            }
        }

        if (!IsValid(PC))
        {
            return;
        }

        if (bSucceeded)
        {
            PC->ClientMagicSelfCastAccepted();
        }
        else
        {
            PC->ClientMagicSelfCastRejected();
        }
    };
    
    UAbilitySystemComponent* ASC = ActorInfo? ActorInfo->AbilitySystemComponent.Get(): nullptr;

    if (!IsValid(ASC) || !SelfGameplayEffectClass)
    {
        UE_LOG(
            LogTemp,
            Error,
            TEXT("[MagicSelfCast] Missing ASC or SelfGameplayEffectClass.")
        );

        NotifyClientResult(false);
        EndAbility(Handle,ActorInfo,ActivationInfo,true,true);
        return;
    }

    UGameplayEffect* EffectCDO =
        SelfGameplayEffectClass->GetDefaultObject<UGameplayEffect>();

    if (!IsValid(EffectCDO))
    {
        UE_LOG(
            LogTemp,
            Error,
            TEXT("[MagicSelfCast] Invalid GameplayEffect CDO. Class: %s"),
            *GetNameSafe(SelfGameplayEffectClass.Get())
        );

        NotifyClientResult(false);
        EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
        return;
    }
    
    FGameplayEffectContextHandle ContextHandle = ASC->MakeEffectContext();

    ContextHandle.SetAbility(this);
    ContextHandle.AddSourceObject(GetAvatarActorFromActorInfo());

    const int32 AbilityLevel = GetAbilityLevel(Handle, ActorInfo);
    const FGameplayEffectSpecHandle SpecHandle = ASC->MakeOutgoingSpec(
        SelfGameplayEffectClass,
        AbilityLevel,
        ContextHandle
    );
    
    if (!SpecHandle.IsValid() || !SpecHandle.Data.IsValid())
    {
        UE_LOG(
            LogTemp,
            Error,
            TEXT("[MagicSelfCast] MakeOutgoingSpec failed. "
                 "Effect=%s | CDO=%s | ASC=%s"),
            *GetNameSafe(SelfGameplayEffectClass.Get()),
            *GetNameSafe(EffectCDO),
            *GetNameSafe(ASC)
        );

        NotifyClientResult(false);
        EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
        return;
    }
    
    if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
    {
        UE_LOG(
            LogTemp,
            Error,
            TEXT("[MagicSelfCast] Failed to create GameplayEffectSpec.")
        );

        NotifyClientResult(false);
        EndAbility(Handle,ActorInfo,ActivationInfo,true,true);
        return;
    }

    bool bEffectApplied = false;
    const FDelegateHandle ApplyDelegateHandle =
        ASC->OnGameplayEffectAppliedDelegateToSelf.AddLambda(
            [EffectCDO, &bEffectApplied](
                UAbilitySystemComponent*,
                const FGameplayEffectSpec& AppliedSpec,
                FActiveGameplayEffectHandle
            )
            {
                if (AppliedSpec.Def == EffectCDO)
                {
                    bEffectApplied = true;
                }
            }
        );

    ASC->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());
    ASC->OnGameplayEffectAppliedDelegateToSelf.Remove(ApplyDelegateHandle);

    if (!bEffectApplied)
    {
        UE_LOG(
            LogTemp,
            Error,
            TEXT("[MagicSelfCast] GameplayEffect %s was not applied."),
            *GetNameSafe(SelfGameplayEffectClass.Get())
        );

        NotifyClientResult(false);
        EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
        return;
    }

    UE_LOG(
        LogTemp,
        Log,
        TEXT("[MagicSelfCast] Applied %s to self."),
        *GetNameSafe(SelfGameplayEffectClass.Get())
    );

    NotifyClientResult(true);

    EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
}