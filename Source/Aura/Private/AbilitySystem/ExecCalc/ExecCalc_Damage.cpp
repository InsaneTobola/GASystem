// Copyright Druid Mechanics


#include "AbilitySystem/ExecCalc/ExecCalc_Damage.h"
#include "AbilitySystemComponent.h"
#include "AuraAbilityTypes.h"
#include "AuraGameplayTags.h"
#include "AbilitySystem/AuraAbilitySystemLibrary.h"
#include "AbilitySystem/AuraAttributeSet.h"
#include "Data/CharacterClassInfo.h"
#include "Interaction/CombatInterface.h"

struct AuraDamageStatics
{
	DECLARE_ATTRIBUTE_CAPTUREDEF(Fire);
	DECLARE_ATTRIBUTE_CAPTUREDEF(Water);
	DECLARE_ATTRIBUTE_CAPTUREDEF(Earth);
	DECLARE_ATTRIBUTE_CAPTUREDEF(Air);
	
	DECLARE_ATTRIBUTE_CAPTUREDEF(Armor);
	DECLARE_ATTRIBUTE_CAPTUREDEF(ArmorPenetration);
	DECLARE_ATTRIBUTE_CAPTUREDEF(BlockChance);
	DECLARE_ATTRIBUTE_CAPTUREDEF(CriticalHitChance);
	DECLARE_ATTRIBUTE_CAPTUREDEF(CriticalHitDamage);
	DECLARE_ATTRIBUTE_CAPTUREDEF(CriticalHitResistance);
	
	DECLARE_ATTRIBUTE_CAPTUREDEF(FireResistance)
	DECLARE_ATTRIBUTE_CAPTUREDEF(WaterResistance)
	DECLARE_ATTRIBUTE_CAPTUREDEF(EarthResistance)
	DECLARE_ATTRIBUTE_CAPTUREDEF(AirResistance)
	
	TMap<FGameplayTag, FGameplayEffectAttributeCaptureDefinition> TagsToCaptureDefs;
	
	AuraDamageStatics()
	{
		DEFINE_ATTRIBUTE_CAPTUREDEF(UAuraAttributeSet, Fire, Source, false);
		DEFINE_ATTRIBUTE_CAPTUREDEF(UAuraAttributeSet, Water, Source, false);
		DEFINE_ATTRIBUTE_CAPTUREDEF(UAuraAttributeSet, Earth, Source, false);
		DEFINE_ATTRIBUTE_CAPTUREDEF(UAuraAttributeSet, Air, Source, false);
		
		DEFINE_ATTRIBUTE_CAPTUREDEF(UAuraAttributeSet, Armor, Target, false);
		DEFINE_ATTRIBUTE_CAPTUREDEF(UAuraAttributeSet, ArmorPenetration, Source, false);
		DEFINE_ATTRIBUTE_CAPTUREDEF(UAuraAttributeSet, BlockChance, Target, false);
		DEFINE_ATTRIBUTE_CAPTUREDEF(UAuraAttributeSet, CriticalHitChance, Source, false);
		DEFINE_ATTRIBUTE_CAPTUREDEF(UAuraAttributeSet, CriticalHitDamage, Source, false);
		DEFINE_ATTRIBUTE_CAPTUREDEF(UAuraAttributeSet, CriticalHitResistance, Target, false);
		
		DEFINE_ATTRIBUTE_CAPTUREDEF(UAuraAttributeSet, FireResistance, Target, false);
		DEFINE_ATTRIBUTE_CAPTUREDEF(UAuraAttributeSet, WaterResistance, Target, false);
		DEFINE_ATTRIBUTE_CAPTUREDEF(UAuraAttributeSet, EarthResistance, Target, false);
		DEFINE_ATTRIBUTE_CAPTUREDEF(UAuraAttributeSet, AirResistance, Target, false);
	
		const FAuraGameplayTags& Tags = FAuraGameplayTags::Get();
		
		TagsToCaptureDefs.Add(Tags.Attributes_Primary_Fire, FireDef);
		TagsToCaptureDefs.Add(Tags.Attributes_Primary_Water, WaterDef);
		TagsToCaptureDefs.Add(Tags.Attributes_Primary_Earth, EarthDef);
		TagsToCaptureDefs.Add(Tags.Attributes_Primary_Air, AirDef);
		
		TagsToCaptureDefs.Add(Tags.Attributes_Secondary_Armor, ArmorDef);
		TagsToCaptureDefs.Add(Tags.Attributes_Secondary_ArmorPenetration, ArmorPenetrationDef);
		TagsToCaptureDefs.Add(Tags.Attributes_Secondary_BlockChance, BlockChanceDef);
		TagsToCaptureDefs.Add(Tags.Attributes_Secondary_CriticalHitChance, CriticalHitChanceDef);
		TagsToCaptureDefs.Add(Tags.Attributes_Secondary_CriticalHitDamage, CriticalHitDamageDef);
		TagsToCaptureDefs.Add(Tags.Attributes_Secondary_CriticalHitResistance, CriticalHitResistanceDef);
		
		TagsToCaptureDefs.Add(Tags.Attributes_Resistance_Fire, FireResistanceDef);
		TagsToCaptureDefs.Add(Tags.Attributes_Resistance_Water, WaterResistanceDef);
		TagsToCaptureDefs.Add(Tags.Attributes_Resistance_Earth, EarthResistanceDef);
		TagsToCaptureDefs.Add(Tags.Attributes_Resistance_Air, AirResistanceDef);
	}
};

static const AuraDamageStatics& DamageStatics()
{
	static AuraDamageStatics DStatics;
	return DStatics;
}

UExecCalc_Damage::UExecCalc_Damage()
{
	RelevantAttributesToCapture.Add(DamageStatics().FireDef);
	RelevantAttributesToCapture.Add(DamageStatics().WaterDef);
	RelevantAttributesToCapture.Add(DamageStatics().EarthDef);
	RelevantAttributesToCapture.Add(DamageStatics().AirDef);
	
	RelevantAttributesToCapture.Add(DamageStatics().ArmorDef);
	RelevantAttributesToCapture.Add(DamageStatics().ArmorPenetrationDef);
	RelevantAttributesToCapture.Add(DamageStatics().BlockChanceDef);
	RelevantAttributesToCapture.Add(DamageStatics().CriticalHitChanceDef);
	RelevantAttributesToCapture.Add(DamageStatics().CriticalHitDamageDef);
	RelevantAttributesToCapture.Add(DamageStatics().CriticalHitResistanceDef);
	
	RelevantAttributesToCapture.Add(DamageStatics().FireResistanceDef);
	RelevantAttributesToCapture.Add(DamageStatics().WaterResistanceDef);
	RelevantAttributesToCapture.Add(DamageStatics().EarthResistanceDef);
	RelevantAttributesToCapture.Add(DamageStatics().AirResistanceDef);
}

void UExecCalc_Damage::Execute_Implementation(const FGameplayEffectCustomExecutionParameters& ExecutionParams,
	FGameplayEffectCustomExecutionOutput& OutExecutionOutput) const
{
	const UAbilitySystemComponent* SourceASC = ExecutionParams.GetSourceAbilitySystemComponent();
	const UAbilitySystemComponent* TargetASC = ExecutionParams.GetTargetAbilitySystemComponent();
	
	AActor* SourceAvatar = SourceASC ? SourceASC->GetAvatarActor() : nullptr;
	AActor* TargetAvatar = TargetASC ? TargetASC->GetAvatarActor() : nullptr;
	
	if (!IsValid(SourceAvatar) || !IsValid(TargetAvatar))
	{
		UE_LOG(LogTemp,Error,TEXT("ExecCalc_Damage: SourceAvatar or TargetAvatar is invalid."));
		return;
	}
		
	int32 SourcePlayerLevel = 1.f;
	if (SourceAvatar->Implements<UCombatInterface>())
	{
		SourcePlayerLevel = ICombatInterface::Execute_GetPlayerLevel(SourceAvatar);
	}
	int32 TargetPlayerLevel = 1.f;
	if (TargetAvatar->Implements<UCombatInterface>())
	{
		TargetPlayerLevel = ICombatInterface::Execute_GetPlayerLevel(TargetAvatar);
	}
	UE_LOG(
	LogTemp,
	Warning,
	TEXT("[ElementalDebug] SourceASC=%s | SourceAvatar=%s | Fire=%.2f | Water=%.2f | Earth=%.2f | Air=%.2f"),
	*GetNameSafe(SourceASC),
	*GetNameSafe(SourceAvatar),
	SourceASC->GetNumericAttribute(UAuraAttributeSet::GetFireAttribute()),
	SourceASC->GetNumericAttribute(UAuraAttributeSet::GetWaterAttribute()),
	SourceASC->GetNumericAttribute(UAuraAttributeSet::GetEarthAttribute()),
	SourceASC->GetNumericAttribute(UAuraAttributeSet::GetAirAttribute())
);
	const FGameplayEffectSpec& Spec = ExecutionParams.GetOwningSpec();
	const FGameplayTagContainer* SourceTags = Spec.CapturedSourceTags.GetAggregatedTags();
	const FGameplayTagContainer* TargetTags = Spec.CapturedTargetTags.GetAggregatedTags();
	FAggregatorEvaluateParameters EvaluationParameters;
	EvaluationParameters.SourceTags = SourceTags;
	EvaluationParameters.TargetTags = TargetTags;
	
	// Get Damage Set by Caller Magnitude
	float Damage = 0.f;
	for (const TTuple<FGameplayTag, FGameplayTag>& Pair : FAuraGameplayTags::Get().DamageTypesToResistances)
	{
		
		const FGameplayTag DamageTypeTag =  Pair.Key;
		const FGameplayTag ResistanceTag = Pair.Value;
		
		checkf(AuraDamageStatics().TagsToCaptureDefs.Contains(ResistanceTag), TEXT("TagsToCaptureDefs doesnt contain Tag: [%s] in ExecCalc_Damage"), *ResistanceTag.ToString());
		const FGameplayEffectAttributeCaptureDefinition CaptureDef = AuraDamageStatics().TagsToCaptureDefs[ResistanceTag];
		
		float DamageTypeValue = Spec.GetSetByCallerMagnitude(Pair.Key, false, 0.f);
		
		const float RawDamageTypeValue = DamageTypeValue;

		float ElementalPower = 0.f;
		float ElementalMultiplier = 1.f;

		const FAuraGameplayTags& GameplayTags = FAuraGameplayTags::Get();
		const FGameplayTag* PrimaryAttributeTag = GameplayTags.DamageTypesToPrimaryAttributes.Find(DamageTypeTag);

		if (PrimaryAttributeTag)
		{
			const FGameplayEffectAttributeCaptureDefinition* ElementalCaptureDef = AuraDamageStatics().TagsToCaptureDefs.Find(*PrimaryAttributeTag);

			if (ElementalCaptureDef)
			{
				const bool bCaptured = ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(*ElementalCaptureDef,EvaluationParameters,ElementalPower);
				if (!bCaptured)
				{
					ElementalPower = 0.f;
					UE_LOG(LogTemp,Warning,TEXT("[ExecCalc_Damage] Failed to capture ""elemental attribute for %s"),*DamageTypeTag.ToString());
				}
			}
			else
			{
				UE_LOG(LogTemp,Error,TEXT("[ExecCalc_Damage] Missing capture definition ""for elemental attribute of %s"),*DamageTypeTag.ToString());
			}
			ElementalPower = FMath::Max(ElementalPower, 0.f);

			// Starting balance value: +1% damage per attribute point.
			constexpr float ElementalDamageScalePerPoint = 0.01f;
			ElementalMultiplier = 1.f + ElementalPower * ElementalDamageScalePerPoint;
			DamageTypeValue *= ElementalMultiplier;
		}
		else
		{
			UE_LOG(LogTemp,Warning,TEXT("[ExecCalc_Damage] No elemental scaling mapping ""for damage type %s"),*DamageTypeTag.ToString());
		}
		
		float Resistance = 0.f;
		ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(CaptureDef, EvaluationParameters, Resistance);
		Resistance = FMath::Clamp(Resistance, 0.f, 100.f);
		
		DamageTypeValue *= (100.f - Resistance)/100.f;
		
		UE_LOG(
	LogTemp,
	Warning,
	TEXT("[ExecCalc_Damage] Type=%s Raw=%.2f Resistance=%.2f AfterResistance=%.2f"),
	*Pair.Key.ToString(),
	RawDamageTypeValue,
	Resistance,
	DamageTypeValue
);
		
		Damage += DamageTypeValue;
	}
	
	
	// Capture BlockChance on Target, and determine if there was a successful Block
	float TargetBlockChance = 0.f;
	ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(DamageStatics().BlockChanceDef,EvaluationParameters,TargetBlockChance);
	TargetBlockChance = FMath::Clamp(TargetBlockChance, 0.f, 100.f);
	const bool bBlocked = FMath::FRand() * 100.f < TargetBlockChance;
	
	FGameplayEffectContextHandle EffectContextHandle = Spec.GetContext();
	FGameplayEffectContext* Context = EffectContextHandle.Get();
	FAuraGameplayEffectContext* AuraContext = static_cast<FAuraGameplayEffectContext*>(Context);
	AuraContext->SetIsBlockedHit(bBlocked);
	
	// If Block, halve Damage
	if (bBlocked)
	{
		Damage *= 0.5f;
	}
	
	float TargetArmor = 0.f;
	ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(DamageStatics().ArmorDef,EvaluationParameters,TargetArmor);
	TargetArmor = FMath::Max<float>(TargetArmor, 0.f);
	
	float SourceArmorPenetration = 0.f;
	ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(DamageStatics().ArmorPenetrationDef,EvaluationParameters,SourceArmorPenetration);
	SourceArmorPenetration = FMath::Max<float>(SourceArmorPenetration, 0.f);
	
	const UCharacterClassInfo* CharacterClassInfo =
	UAuraAbilitySystemLibrary::GetCharacterClassInfo(SourceAvatar);

	if (!IsValid(CharacterClassInfo) || !IsValid(CharacterClassInfo->DamageCalculationCoefficient.Get()))
	{
		UE_LOG(LogTemp,Error,TEXT("ExecCalc_Damage: CharacterClassInfo or DamageCalculationCoefficient is missing."));

		return;
	}
	const UCurveTable* DamageCoefficientTable = CharacterClassInfo->DamageCalculationCoefficient.Get();
	const FRealCurve* ArmorPenetrationCurve = DamageCoefficientTable->FindCurve(FName("ArmorPenetration"),FString());
	const FRealCurve* EffectiveArmorCurve = DamageCoefficientTable->FindCurve(FName("EffectiveArmor"),FString());
	const FRealCurve* CriticalHitResistanceCurve = DamageCoefficientTable->FindCurve(FName("CriticalHitResistance"),FString());

	if (!ArmorPenetrationCurve || !EffectiveArmorCurve || !CriticalHitResistanceCurve)
	{
		UE_LOG(LogTemp,Error,TEXT("ExecCalc_Damage: A required damage calculation curve is missing."));
		return;
	}

	// Armor penetration
	const float ArmorPenetrationCoefficient = ArmorPenetrationCurve->Eval(static_cast<float>(SourcePlayerLevel));
	const float ArmorPenetrationPercent = FMath::Clamp(SourceArmorPenetration * ArmorPenetrationCoefficient,0.f,100.f);

	// Effective armor after penetration
	const float EffectiveArmor = TargetArmor * (1.f - ArmorPenetrationPercent / 100.f);
	
	// Armor damage reduction
	const float EffectiveArmorCoefficient = FMath::Max(EffectiveArmorCurve->Eval(static_cast<float>(TargetPlayerLevel)),0.f);
	const float EffectiveArmorPercent = FMath::Clamp(EffectiveArmor * EffectiveArmorCoefficient,0.f,100.f);
	Damage *= 1.f - EffectiveArmorPercent / 100.f;
	
	float SourceCriticalHitChance = 0.f;
	ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(DamageStatics().CriticalHitChanceDef,EvaluationParameters,SourceCriticalHitChance);
	SourceCriticalHitChance = FMath::Clamp(SourceCriticalHitChance, 0.f, 100.f);

	float SourceCriticalHitDamage = 0.f;
	ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(DamageStatics().CriticalHitDamageDef,EvaluationParameters,SourceCriticalHitDamage);
	SourceCriticalHitDamage = FMath::Clamp(SourceCriticalHitDamage, 1.f, 3.f);
	
	float TargetCriticalHitResistance = 0.f;
	ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(DamageStatics().CriticalHitResistanceDef,EvaluationParameters,TargetCriticalHitResistance);
	TargetCriticalHitResistance = FMath::Max(TargetCriticalHitResistance, 0.f);
	
	const float CriticalHitResistanceCoefficient =FMath::Max(CriticalHitResistanceCurve->Eval(static_cast<float>(TargetPlayerLevel)),0.f);
	const float EffectiveCriticalHitChance = FMath::Clamp(SourceCriticalHitChance -TargetCriticalHitResistance * CriticalHitResistanceCoefficient,0.f,100.f);
	const bool bCriticalHit =FMath::FRand() * 100.f < EffectiveCriticalHitChance;
	
	UAuraAbilitySystemLibrary::SetIsCriticalHit(EffectContextHandle, bCriticalHit);
	
	// Double damage
	if (bCriticalHit)
	{
		Damage *= SourceCriticalHitDamage;
	}
	UE_LOG(
	LogTemp,
	Warning,
	TEXT("[ExecCalc_Damage] FinalDamage=%.2f Target=%s"),
	Damage,
	*GetNameSafe(TargetAvatar)
);
	
	const FGameplayModifierEvaluatedData EvaluatedData(UAuraAttributeSet::GetIncomingDamageAttribute(), EGameplayModOp::Additive, Damage);
	OutExecutionOutput.AddOutputModifier(EvaluatedData);
	
	
}
