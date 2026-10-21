// Copyright Druid Mechanics


#include "Actor/AuraEffectActor.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"



AAuraEffectActor::AAuraEffectActor()
{
	PrimaryActorTick.bCanEverTick = false;
	
	SetRootComponent(CreateDefaultSubobject<USceneComponent>("SceneRoot"));
}


void AAuraEffectActor::BeginPlay()
{
	Super::BeginPlay();
}

void AAuraEffectActor::ApplyEffectToTarget(AActor* TargetActor, TSubclassOf<UGameplayEffect> GameplayEffectClass)
{
	if (!IsValid(TargetActor) || !GameplayEffectClass)
	{
		return;
	}

	UAbilitySystemComponent* TargetASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(TargetActor);

	if (!IsValid(TargetASC))
	{
		return;
	}

	FGameplayEffectContextHandle EffectContextHandle = TargetASC->MakeEffectContext();
	EffectContextHandle.AddSourceObject(this);
	const FGameplayEffectSpecHandle EffectSpecHandle = TargetASC->MakeOutgoingSpec(GameplayEffectClass,ActorLevel,EffectContextHandle);
	
	if (!EffectSpecHandle.IsValid() || !EffectSpecHandle.Data.IsValid())
	{
		UE_LOG(
			LogTemp,
			Error,
			TEXT("AuraEffectActor: Failed to create GameplayEffectSpec.")
		);

		return;
	}
	
	const UGameplayEffect* EffectDefinition = EffectSpecHandle.Data->Def.Get();

	if (!IsValid(EffectDefinition))
	{
		UE_LOG(LogTemp,Error,TEXT("AuraEffectActor: GameplayEffect definition is invalid."));
		return;
	}
	const FActiveGameplayEffectHandle ActiveEffectHandle = TargetASC->ApplyGameplayEffectSpecToSelf(*EffectSpecHandle.Data.Get());

	if (EffectDefinition->DurationPolicy == EGameplayEffectDurationType::Infinite && InfiniteEffectRemovalPolicy ==
			EEffectRemovalPolicy::RemoveOnEndOverlap && ActiveEffectHandle.IsValid())
	{
		ActiveEffectHandles.FindOrAdd(TargetASC).Add(ActiveEffectHandle);
	}
}

void AAuraEffectActor::OnOverlap(AActor* TargetActor)
{
	if (!HasAuthority()) return;
	
	if (InstantEffectApplicationPolicy == EEffectApplicationPolicy::ApplyOnOverlap)
	{
		for (const TSubclassOf<UGameplayEffect>& EffectClass : InstantGameplayEffectClass)
		{
			ApplyEffectToTarget(TargetActor, EffectClass);
		}
	}
	if (DurationEffectApplicationPolicy == EEffectApplicationPolicy::ApplyOnOverlap)
	{
		for (const TSubclassOf<UGameplayEffect>& EffectClass : DurationGameplayEffectClass)
		{
			ApplyEffectToTarget(TargetActor, EffectClass);
		}
	}
	if (InfiniteEffectApplicationPolicy == EEffectApplicationPolicy::ApplyOnOverlap)
	{
		for (const TSubclassOf<UGameplayEffect>& EffectClass : InfiniteGameplayEffectClass)
		{
			ApplyEffectToTarget(TargetActor, EffectClass);
		}
	}
}

void AAuraEffectActor::OnEndOverlap(AActor* TargetActor)
{
	if (!HasAuthority()) return;
	
	if (InstantEffectApplicationPolicy == EEffectApplicationPolicy::ApplyOnEndOverlap)
	{
		for (const TSubclassOf<UGameplayEffect>& EffectClass : InstantGameplayEffectClass)
		{
			ApplyEffectToTarget(TargetActor, EffectClass);
		}
	}
	if (DurationEffectApplicationPolicy == EEffectApplicationPolicy::ApplyOnEndOverlap)
	{
		for (const TSubclassOf<UGameplayEffect>& EffectClass : DurationGameplayEffectClass)
		{
			ApplyEffectToTarget(TargetActor, EffectClass);
		}
	}
	if (InfiniteEffectApplicationPolicy == EEffectApplicationPolicy::ApplyOnEndOverlap)
	{
		for (const TSubclassOf<UGameplayEffect>& EffectClass : InfiniteGameplayEffectClass)
		{
			ApplyEffectToTarget(TargetActor, EffectClass);
		}
	}
	
	
	if (InfiniteEffectRemovalPolicy == EEffectRemovalPolicy::RemoveOnEndOverlap)
	{
		UAbilitySystemComponent* TargetASC =
			UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(TargetActor);

		if (!IsValid(TargetASC)) return;

		if (TArray<FActiveGameplayEffectHandle>* Handles =
			ActiveEffectHandles.Find(TargetASC))
		{
			for (const FActiveGameplayEffectHandle& Handle : *Handles)
			{
				TargetASC->RemoveActiveGameplayEffect(Handle);
			}

			ActiveEffectHandles.Remove(TargetASC);
		}
	}
	
}



