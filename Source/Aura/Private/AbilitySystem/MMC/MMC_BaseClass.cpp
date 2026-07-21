// Copyright Druid Mechanics


#include "AbilitySystem/MMC/MMC_BaseClass.h"
#include "AbilitySystem/AuraAttributeSet.h"


void UMMC_Base::AddAttributeCapture(
	FGameplayEffectAttributeCaptureDefinition& Def,
	const FGameplayAttribute& Attribute,
	EGameplayEffectAttributeCaptureSource Source,
	bool bSnapshot)
{
	Def.AttributeToCapture = Attribute;
	Def.AttributeSource = Source;
	Def.bSnapshot = bSnapshot;

	RelevantAttributesToCapture.Add(Def);
}

float UMMC_Base::GetCapturedValue(
	const FGameplayEffectAttributeCaptureDefinition& CaptureDef,
	const FGameplayEffectSpec& Spec) const
{
	const FGameplayTagContainer* SourceTags =
		Spec.CapturedSourceTags.GetAggregatedTags();

	const FGameplayTagContainer* TargetTags =
		Spec.CapturedTargetTags.GetAggregatedTags();

	FAggregatorEvaluateParameters Params;
	Params.SourceTags = SourceTags;
	Params.TargetTags = TargetTags;

	float Value = 0.f;

	GetCapturedAttributeMagnitude(
		CaptureDef,
		Spec,
		Params,
		Value);

	return FMath::Max(0.f, Value);
}