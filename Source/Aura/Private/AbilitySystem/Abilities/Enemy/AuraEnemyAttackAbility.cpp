// Copyright Druid Mechanics

#include "AbilitySystem/Abilities/Enemy/AuraEnemyAttackAbility.h"

#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Animation/AnimMontage.h"

void UAuraEnemyAttackAbility::ActivateAbility(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle,ActorInfo,ActivationInfo,TriggerEventData);

	if (!AttackMontage)
	{
		EndAbility(Handle,ActorInfo,ActivationInfo,true,true);
		return;
	}

	UAbilityTask_PlayMontageAndWait* MontageTask =UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(this,NAME_None,AttackMontage,1.0f);

	if (!MontageTask)
	{
		EndAbility(Handle,ActorInfo,ActivationInfo,true,true);
		return;
	}

	MontageTask->OnCompleted.AddDynamic(this,&UAuraEnemyAttackAbility::OnMontageCompleted);
	MontageTask->OnInterrupted.AddDynamic(this,&UAuraEnemyAttackAbility::OnMontageInterrupted);
	MontageTask->OnCancelled.AddDynamic(this,&UAuraEnemyAttackAbility::OnMontageInterrupted);
	MontageTask->ReadyForActivation();
}

void UAuraEnemyAttackAbility::OnMontageCompleted()
{
	EndAbility(CurrentSpecHandle,CurrentActorInfo,CurrentActivationInfo,true,false);
}

void UAuraEnemyAttackAbility::OnMontageInterrupted()
{
	EndAbility(CurrentSpecHandle,CurrentActorInfo,CurrentActivationInfo,true,true);
}