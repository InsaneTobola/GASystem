// Copyright Druid Mechanics


#include "Input/AuraInputConfig.h"

const UInputAction* UAuraInputConfig::FindAbilityInputActionForTag(const FGameplayTag& InputTag,
	bool bLogNotFound) const
{
	for (const FAuraInputAction& Action : AbilityInputActions)
	{
		if (Action.InputAction && Action.InputTag == InputTag)
		{
			return Action.InputAction;
		}
	}
	
	if (bLogNotFound)
	{
		UE_LOG(LogTemp, Error, TEXT("Cant find AbilityInputAction for InputTag [%s], on InputConfig [%s]"), *InputTag.ToString(), *GetNameSafe(this))
	}
	
	return nullptr;
}

const UInputAction* UAuraInputConfig::FindMagicInputActionForTag(const FGameplayTag& InputTag, bool bLogNotFound) const
{
	for (const FAuraInputAction& Action : MagicInputActions)
	{
		if (Action.InputAction && Action.InputTag == InputTag)
		{
			return Action.InputAction;
		}
	}

	if (bLogNotFound)
	{
		UE_LOG(LogTemp,Error,TEXT("Cant find MagicInputAction for InputTag [%s], on InputConfig [%s]"),*InputTag.ToString(),*GetNameSafe(this));
	}

	return nullptr;
}
