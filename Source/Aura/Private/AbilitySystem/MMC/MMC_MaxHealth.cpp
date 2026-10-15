// Copyright Druid Mechanics


#include "AbilitySystem/MMC/MMC_MaxHealth.h"

#include "AbilitySystem/AuraAttributeSet.h"
#include "Interaction/CombatInterface.h"

UMMC_MaxHealth::UMMC_MaxHealth()
{
	EarthDef.AttributeToCapture = UAuraAttributeSet::GetEarthAttribute();
	EarthDef.AttributeSource = EGameplayEffectAttributeCaptureSource::Target;
	EarthDef.bSnapshot = false;
	
	RelevantAttributesToCapture.Add(EarthDef);
}

float UMMC_MaxHealth::CalculateBaseMagnitude_Implementation(const FGameplayEffectSpec& Spec) const
{
	// Gather tags from source and target
	const FGameplayTagContainer* SourceTags = Spec.CapturedSourceTags.GetAggregatedTags();
	const FGameplayTagContainer* TargetTags = Spec.CapturedTargetTags.GetAggregatedTags();
	
	FAggregatorEvaluateParameters EvaluationParameters;
	EvaluationParameters.SourceTags = SourceTags;
	EvaluationParameters.TargetTags = TargetTags;
	
	float Earth = 0.f;
	GetCapturedAttributeMagnitude(EarthDef, Spec, EvaluationParameters, Earth );
	Earth = FMath::Max<float>(Earth, 0.f);
	
	int32 PlayerLevel = 1;

	UObject* SourceObject = Spec.GetContext().GetSourceObject();

	if (IsValid(SourceObject) && SourceObject->Implements<UCombatInterface>())
	{
		PlayerLevel = ICombatInterface::Execute_GetPlayerLevel(SourceObject);
	}
	
	return 430.f + 5.f * Earth + 20.0f * PlayerLevel;
}
