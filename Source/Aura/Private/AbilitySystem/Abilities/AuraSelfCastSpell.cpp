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

void UAuraSelfCastSpell::ActivateAbility(const FGameplayAbilitySpecHandle Handle,const FGameplayAbilityActorInfo* ActorInfo,const FGameplayAbilityActivationInfo ActivationInfo,
const FGameplayEventData* TriggerEventData)
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

    if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
    {
        UE_LOG(
            LogTemp,
            Warning,
            TEXT("[MagicSelfCast] CommitAbility failed.")
        );

        NotifyClientResult(false);

        EndAbility(Handle,ActorInfo,ActivationInfo,true,true);

        return;
    }

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

    FGameplayEffectContextHandle ContextHandle = ASC->MakeEffectContext();

    ContextHandle.SetAbility(this);
    ContextHandle.AddSourceObject(GetAvatarActorFromActorInfo());

    const FGameplayEffectSpecHandle SpecHandle = ASC->MakeOutgoingSpec(SelfGameplayEffectClass,GetAbilityLevel(),ContextHandle);

    if (!SpecHandle.IsValid() || !SpecHandle.Data.IsValid())
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

    ASC->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get()
    );

    UE_LOG(
        LogTemp,
        Log,
        TEXT("[MagicSelfCast] Applied %s to self."),
        *GetNameSafe(SelfGameplayEffectClass.Get())
    );

    NotifyClientResult(true);

    EndAbility(Handle,ActorInfo,ActivationInfo,true,false);
}