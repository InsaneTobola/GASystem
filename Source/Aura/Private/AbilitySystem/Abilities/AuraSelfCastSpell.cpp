#include "AbilitySystem/Abilities/AuraSelfCastSpell.h"

#include "AbilitySystemComponent.h"
#include "GameplayEffect.h"
#include "Player/AuraPlayerController.h"

UAuraSelfCastSpell::UAuraSelfCastSpell()
{
    InstancingPolicy =EGameplayAbilityInstancingPolicy::InstancedPerExecution;
    NetExecutionPolicy =EGameplayAbilityNetExecutionPolicy::ServerOnly;
}

void UAuraSelfCastSpell::ActivateAbility(const FGameplayAbilitySpecHandle Handle,const FGameplayAbilityActorInfo* ActorInfo,const FGameplayAbilityActivationInfo ActivationInfo,const FGameplayEventData* TriggerEventData)
{
    Super::ActivateAbility(Handle,ActorInfo,ActivationInfo,TriggerEventData);
   

    if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
    {
        UE_LOG(
            LogTemp,
            Warning,
            TEXT("[MagicSelfCast] CommitAbility failed.")
        );

      
        EndAbility(Handle,ActorInfo,ActivationInfo,true,true);

        return;
    }

    UAbilitySystemComponent* ASC = ActorInfo? ActorInfo->AbilitySystemComponent.Get(): nullptr;

    if (!IsValid(ASC) || !SelfGameplayEffectClass)
    {
        UE_LOG(
            LogTemp,
            Error,
            TEXT(
                "[MagicSelfCast] Missing ASC or SelfGameplayEffectClass."
            )
        );

        
        EndAbility(Handle,ActorInfo,ActivationInfo,true,true);

        return;
    }

    FGameplayEffectContextHandle ContextHandle =ASC->MakeEffectContext();
    ContextHandle.SetAbility(this);
    ContextHandle.AddSourceObject(GetAvatarActorFromActorInfo());

    const FGameplayEffectSpecHandle SpecHandle =ASC->MakeOutgoingSpec(SelfGameplayEffectClass,GetAbilityLevel(),ContextHandle);

    if (!SpecHandle.IsValid() || !SpecHandle.Data.IsValid())
    {
        UE_LOG(
            LogTemp,
            Error,
            TEXT(
                "[MagicSelfCast] Failed to create GameplayEffectSpec."
            )
        );

        
        EndAbility(Handle,ActorInfo,ActivationInfo,true,true);
        return;
    }

    ASC->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());

    UE_LOG(
        LogTemp,
        Warning,
        TEXT(
            "[MagicSelfCast] Applied %s to self."
        ),
        *GetNameSafe(SelfGameplayEffectClass.Get())
    );
    
    EndAbility(Handle,ActorInfo,ActivationInfo,true,false);
}