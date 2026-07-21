// Copyright Druid Mechanics


#include "AbilitySystem/MMC/MMC_MaxMana.h"
#include "AbilitySystem/AuraAttributeSet.h"
#include "Interaction/CombatInterface.h"

UMMC_MaxMana::UMMC_MaxMana()
{
	WaterDef.AttributeToCapture = UAuraAttributeSet::GetWaterAttribute();
	WaterDef.AttributeSource = EGameplayEffectAttributeCaptureSource::Target;
	WaterDef.bSnapshot = false;
	
	RelevantAttributesToCapture.Add(WaterDef);
}

float UMMC_MaxMana::CalculateBaseMagnitude_Implementation(const FGameplayEffectSpec& Spec) const
{
	// Gather tags from source and target
	const FGameplayTagContainer* SourceTags = Spec.CapturedSourceTags.GetAggregatedTags();
	const FGameplayTagContainer* TargetTags = Spec.CapturedTargetTags.GetAggregatedTags();
	
	FAggregatorEvaluateParameters EvaluationParameters;
	EvaluationParameters.SourceTags = SourceTags;
	EvaluationParameters.TargetTags = TargetTags;
	
	float Water = 0.f;
	GetCapturedAttributeMagnitude(WaterDef, Spec, EvaluationParameters, Water );
	Water = FMath::Max<float>(Water, 0.f);
	
	ICombatInterface* CombatInterface = Cast<ICombatInterface>(Spec.GetContext(). GetSourceObject());
	const int32 PlayerLevel = CombatInterface->GetPlayerLevel();
	
	return 120.f + 4.f * Water + 40.0f * PlayerLevel;
	
}
