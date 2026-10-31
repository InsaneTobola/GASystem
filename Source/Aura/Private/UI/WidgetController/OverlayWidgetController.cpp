// Copyright Druid Mechanics


#include "UI/WidgetController/OverlayWidgetController.h"

#include "AuraGameplayTags.h"
#include "AbilitySystem/AuraAbilitySystemComponent.h"
#include "AbilitySystem/AuraAttributeSet.h"
#include "AbilitySystem/Abilities/AuraGameplayAbility.h"
#include "GameplayEffect.h"
#include "Data/LevelUpInfo.h"
#include "Player/AuraPlayerState.h"

void UOverlayWidgetController::BroadcastInitialValues()
{
	const UAuraAttributeSet* AuraAttributeSet = CastChecked<UAuraAttributeSet>(AttributeSet);	
	
	OnHealthChanged.Broadcast(AuraAttributeSet->GetHealth());
	OnMaxHealthChanged.Broadcast(AuraAttributeSet->GetMaxHealth());
	OnManaChanged.Broadcast(AuraAttributeSet->GetMana());
	OnMaxManaChanged.Broadcast(AuraAttributeSet->GetMaxMana());
	
	
}

void UOverlayWidgetController::BindCallbacksToDependencies()
{
	AAuraPlayerState* AuraPlayerState = CastChecked<AAuraPlayerState>(PlayerState);
	AuraPlayerState->OnXPChangedDelegate.AddUObject(this, &UOverlayWidgetController::OnXPChanged);
	AuraPlayerState->OnLevelChangedDelegate.AddLambda(
	[this](int32 NewLevel)
		{
			OnPlayerLevelChangedDelegate.Broadcast(NewLevel);
		}
	);
	
	const UAuraAttributeSet* AuraAttributeSet = CastChecked<UAuraAttributeSet>(AttributeSet);
	
	AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(AuraAttributeSet->GetHealthAttribute()).AddLambda(
	[this](const FOnAttributeChangeData& Data)
		{
			OnHealthChanged.Broadcast(Data.NewValue);
		}	
	);
	
	AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(AuraAttributeSet->GetMaxHealthAttribute()).AddLambda(
	[this](const FOnAttributeChangeData& Data)
		{
			OnMaxHealthChanged.Broadcast(Data.NewValue);
		}	
	);
	
	
	AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(AuraAttributeSet->GetManaAttribute()).AddLambda(
	[this](const FOnAttributeChangeData& Data)
		{
			OnManaChanged.Broadcast(Data.NewValue);
		}	
	);
	
	AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(AuraAttributeSet->GetMaxManaAttribute()).AddLambda(
	[this](const FOnAttributeChangeData& Data)
		{
			OnMaxManaChanged.Broadcast(Data.NewValue);
		}	
	);
	
	AbilitySystemComponent->AbilityFailedCallbacks.AddUObject(
	this,
	&UOverlayWidgetController::HandleAbilityFailed
	);

	AbilitySystemComponent->OnActiveGameplayEffectAddedDelegateToSelf.AddUObject(
		this,
		&UOverlayWidgetController::HandleActiveEffectAdded
	);
	
	Cast<UAuraAbilitySystemComponent>(AbilitySystemComponent)->EffectAssetTags.AddLambda(
	[this](const FGameplayTagContainer& AssetTags)
		{
		for (const  FGameplayTag& Tag: AssetTags)
			{
				// A.1 .MatchesTag("A") wil return True, "A".MatchesTag("A.1") will return False
				FGameplayTag MessageTag = FGameplayTag::RequestGameplayTag(FName("Message"));
				if (Tag.MatchesTag(MessageTag))
				{
					const FUIWidgetRow* Row = GetDataTableRowByTag<FUIWidgetRow>(MessageWidgetDataTable, Tag);
					if (Row)
					{
						MessageWidgetRowDelegate.Broadcast(*Row);
					}
					else
					{
						UE_LOG(LogTemp,Warning,TEXT("OverlayWidgetController: Missing message row for tag %s."),*Tag.ToString());
					}
				}
			}
		}	
	);
}

void UOverlayWidgetController::OnXPChanged(int32 NewXP) const 
{
	const AAuraPlayerState* AuraPlayerState = CastChecked<AAuraPlayerState>(PlayerState);
	const ULevelUpInfo* LevelUpInfo = AuraPlayerState->LevelUpInfo;
	checkf(LevelUpInfo, TEXT("Unabled to find LevelUpInfo, please fill out AuraPlayerState Blueprint"));
	
	int32 Level = LevelUpInfo->FindLevelForXP(NewXP);
	int32 MaxLevel = LevelUpInfo->LevelUpInformation.Num();
	
	if (Level <= MaxLevel && Level > 0)
	{
		const int32 LevelUpRequirement = LevelUpInfo->LevelUpInformation[Level].LevelUpRequirement;
		const int32 PreviousLevelUpRequirement = LevelUpInfo->LevelUpInformation[Level - 1].LevelUpRequirement;
		
		const int32 DeltaLevelRequirement = LevelUpRequirement - PreviousLevelUpRequirement; 
		const int32 XPForThisLevel = NewXP - PreviousLevelUpRequirement;
		
		const float XPBarPercent = static_cast<float>(XPForThisLevel) / static_cast<float>(DeltaLevelRequirement);
		
		OnXPPercentChangedDelegate.Broadcast(XPBarPercent);
	}
}

void UOverlayWidgetController::HandleActiveEffectAdded(
    UAbilitySystemComponent* ASC,
    const FGameplayEffectSpec& Spec,
    FActiveGameplayEffectHandle Handle
)
{
    const FGameplayTag CooldownRoot =
        FGameplayTag::RequestGameplayTag(
            FName("Cooldown.Magic")
        );

    for (const FGameplayTag& GrantedTag : Spec.DynamicGrantedTags)
    {
        if (GrantedTag.MatchesTag(CooldownRoot))
        {
            RegisterCooldownTag(GrantedTag);
        }
    }
}

void UOverlayWidgetController::RegisterCooldownTag(const FGameplayTag& CooldownTag)
{
    if (!CooldownTag.IsValid() || RegisteredCooldownTags.HasTagExact(CooldownTag))
    {
        return;
    }

    RegisteredCooldownTags.AddTag(CooldownTag);

    AbilitySystemComponent->RegisterAndCallGameplayTagEvent(CooldownTag,FOnGameplayEffectTagCountChanged::FDelegate::CreateUObject(
    	this,
            &UOverlayWidgetController::HandleCooldownTagChanged
        ),
        EGameplayTagEventType::NewOrRemoved
    );
}

void UOverlayWidgetController::GetCooldownTimes(FGameplayTag CooldownTag,float& OutRemaining,float& OutDuration) const
{
    OutRemaining = 0.f;
    OutDuration = 0.f;

    if (!IsValid(AbilitySystemComponent) || !CooldownTag.IsValid())
    {
        return;
    }

    FGameplayTagContainer QueryTags;
    QueryTags.AddTag(CooldownTag);

    const FGameplayEffectQuery Query = FGameplayEffectQuery::MakeQuery_MatchAnyOwningTags(QueryTags);
    const TArray<float> RemainingValues = AbilitySystemComponent->GetActiveEffectsTimeRemaining(Query);
    const TArray<float> DurationValues = AbilitySystemComponent->GetActiveEffectsDuration(Query);

    if (!RemainingValues.IsEmpty())
    {
        OutRemaining = RemainingValues[0];
    }

    if (!DurationValues.IsEmpty())
    {
        OutDuration = DurationValues[0];
    }
}

void UOverlayWidgetController::ClientMagicAbilityFailed_Implementation(FGameplayTag MessageTag)
{
	if (!MessageTag.IsValid())
	{
		MessageTag =
			FAuraGameplayTags::Get().Message_AbilityFail_Generic;
	}

	if (UAuraAbilitySystemComponent* ASC = Cast<UAuraAbilitySystemComponent>(AbilitySystemComponent))
	{
		FGameplayTagContainer Tags;
		Tags.AddTag(MessageTag);
		 
		ASC->EffectAssetTags.Broadcast(Tags);
	}
}

void UOverlayWidgetController::HandleCooldownTagChanged(const FGameplayTag CallbackTag,int32 NewCount)
{
    float Remaining = 0.f;
    float Duration = 0.f;

    if (NewCount > 0)
    {
        GetCooldownTimes(CallbackTag, Remaining, Duration);
    }

    OnAbilityCooldownChanged.Broadcast(CallbackTag,NewCount > 0,Duration,Remaining);
}

void UOverlayWidgetController::HandleAbilityFailed(const UGameplayAbility* FailedAbility,const FGameplayTagContainer& FailureTags)
{
	const FAuraGameplayTags& Tags = FAuraGameplayTags::Get();

	FGameplayTag MessageTag;

	if (FailureTags.HasTagExact(Tags.Ability_Fail_Cost_Mana))
	{
		MessageTag = Tags.Message_AbilityFail_Mana;
	}
	else if (FailureTags.HasTagExact(Tags.Ability_Fail_Cooldown))
	{
		MessageTag = Tags.Message_AbilityFail_Cooldown;
	}
	else if (FailureTags.HasTagExact(Tags.Ability_Fail_Configuration))
	{
		MessageTag = Tags.Message_AbilityFail_Generic;
	}

	if (!MessageTag.IsValid())
	{
		return;
	}

	const FUIWidgetRow* Row = GetDataTableRowByTag<FUIWidgetRow>(MessageWidgetDataTable,MessageTag);

	if (Row)
	{
		MessageWidgetRowDelegate.Broadcast(*Row);
	}
	else
	{
		UE_LOG(
			LogTemp,
			Warning,
			TEXT("Missing UI message row for tag %s"),
			*MessageTag.ToString()
		);
	}
}

