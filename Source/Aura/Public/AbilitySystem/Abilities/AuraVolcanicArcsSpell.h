#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/Abilities/AuraAreaSpell.h"
#include "AuraVolcanicArcsSpell.generated.h"

class AAuraVolcanicArcsField;

UCLASS()
class AURA_API UAuraVolcanicArcsSpell : public UAuraAreaSpell
{
	GENERATED_BODY()

public:
	UAuraVolcanicArcsSpell();

	float GetBaseHalfLengthCm() const
	{
		return BaseHalfLengthCm;
	}

	float GetDefaultArcLengthScale() const
	{
		return DefaultArcLengthScale;
	}

	float GetMinArcLengthScale() const
	{
		return MinArcLengthScale;
	}

	float GetMaxArcLengthScale() const
	{
		return MaxArcLengthScale;
	}
	
	float GetArcBowOffsetCm() const
	{
		return ArcBowOffsetCm;
	}

protected:

	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle,const FGameplayAbilityActorInfo* ActorInfo,const FGameplayAbilityActivationInfo ActivationInfo,const FGameplayEventData* TriggerEventData) override;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Volcanic Arcs")
	TSubclassOf<AAuraVolcanicArcsField> VolcanicArcsFieldClass;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Volcanic Arcs")
	float BaseHalfLengthCm = 250.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Volcanic Arcs")
	float DefaultArcLengthScale = 1.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Volcanic Arcs")
	float MinArcLengthScale = 0.7f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Volcanic Arcs")
	float MaxArcLengthScale = 1.4f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Volcanic Arcs")
	float ArcCollisionRadiusCm = 35.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Volcanic Arcs")
	float IntersectionRadiusCm = 65.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Volcanic Arcs")
	float ArcBowOffsetCm = 90.f;
private:

	void RejectVolcanicArcsCast(const FGameplayAbilityActorInfo* ActorInfo) const;
};