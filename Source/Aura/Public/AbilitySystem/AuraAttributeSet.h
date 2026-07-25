// Copyright Druid Mechanics

#pragma once

#include "CoreMinimal.h"
#include "AttributeSet.h"
#include "AbilitySystemComponent.h"
#include "AuraAttributeSet.generated.h"

#define ATTRIBUTE_ACCESSORS(ClassName, PropertyName) \
	GAMEPLAYATTRIBUTE_PROPERTY_GETTER(ClassName, PropertyName) \
	GAMEPLAYATTRIBUTE_VALUE_GETTER(PropertyName) \
	GAMEPLAYATTRIBUTE_VALUE_SETTER(PropertyName) \
	GAMEPLAYATTRIBUTE_VALUE_INITTER(PropertyName)

USTRUCT()
struct FEffectProperties
{
	GENERATED_BODY()

	FEffectProperties(){}
	
	FGameplayEffectContextHandle EffectContextHandle;
	
	/** Source */
	UPROPERTY()
	UAbilitySystemComponent* SourceASC = nullptr;

	UPROPERTY()
	AActor* SourceAvatarActor = nullptr;

	UPROPERTY()
	AController* SourceController = nullptr;
	
	UPROPERTY()
	ACharacter* SourceCharacter = nullptr;
	
	/** Target */
	UPROPERTY()
	UAbilitySystemComponent* TargetASC = nullptr;

	UPROPERTY()
	AActor* TargetAvatarActor = nullptr;
	
	UPROPERTY()
	AController* TargetController = nullptr;
	
	UPROPERTY()
	ACharacter* TargetCharacter = nullptr;
};
//type def is specific to the FGameplayAttribute() signature but TStaticFuncPtr is generic to any signature chosen
//typedef TBaseStaticDelegateInstance<FGameplayAttribute(), FDefaultDelegateUserPolicy>::FFuncPtr FAttributeFuncPtr;
template<class T>
using TStaticFuncPtr= typename TBaseStaticDelegateInstance<T, FDefaultDelegateUserPolicy>::FFuncPtr;
/**
 * 
 */
UCLASS()
class AURA_API UAuraAttributeSet : public UAttributeSet
{
	GENERATED_BODY()
public:
	UAuraAttributeSet();
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	
	virtual void PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue) override;
	
	virtual void PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data) override;
	
	
	TMap<FGameplayTag, TStaticFuncPtr<FGameplayAttribute()>> TagsToAttributes;
	
	
	
	/*
	 * Elemental Attributes
	 */
	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_Fire, Category = "Elemental Attributes")
	FGameplayAttributeData Fire;
	ATTRIBUTE_ACCESSORS(UAuraAttributeSet, Fire);
	
	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_Water, Category = "Elemental Attributes")
	FGameplayAttributeData Water;
	ATTRIBUTE_ACCESSORS(UAuraAttributeSet, Water);
	
	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_Earth, Category = "Elemental Attributes")
	FGameplayAttributeData Earth;
	ATTRIBUTE_ACCESSORS(UAuraAttributeSet, Earth);
	
	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_Air, Category = "Elemental Attributes")
	FGameplayAttributeData Air;
	ATTRIBUTE_ACCESSORS(UAuraAttributeSet, Air);
	
	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_Intelligence, Category = "Elemental Attributes")
	FGameplayAttributeData Intelligence;
	ATTRIBUTE_ACCESSORS(UAuraAttributeSet, Intelligence);
	
	/*
	 * Secondary Attributes
	 */
	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_DamageOnTime, Category = "Secondary Attributes")
	FGameplayAttributeData DamageOnTime;
	ATTRIBUTE_ACCESSORS(UAuraAttributeSet, DamageOnTime);
	
	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_AreaOfEffect, Category = "Secondary Attributes")
	FGameplayAttributeData AreaOfEffect;
	ATTRIBUTE_ACCESSORS(UAuraAttributeSet, AreaOfEffect);
	
	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_Armor, Category = "Secondary Attributes")
	FGameplayAttributeData Armor;
	ATTRIBUTE_ACCESSORS(UAuraAttributeSet, Armor);
	
	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_Armor, Category = "Secondary Attributes")
	FGameplayAttributeData ArmorPenetration;
	ATTRIBUTE_ACCESSORS(UAuraAttributeSet, ArmorPenetration); 
	
	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_CriticalHitChance, Category = "Secondary Attributes")
	FGameplayAttributeData CriticalHitChance;
	ATTRIBUTE_ACCESSORS(UAuraAttributeSet, CriticalHitChance);
	
	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_CriticalHitDamage, Category = "Secondary Attributes")
	FGameplayAttributeData CriticalHitDamage;
	ATTRIBUTE_ACCESSORS(UAuraAttributeSet, CriticalHitDamage);
	
	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_CriticalHitResistance, Category = "Secondary Attributes")
	FGameplayAttributeData CriticalHitResistance;
	ATTRIBUTE_ACCESSORS(UAuraAttributeSet, CriticalHitResistance);
	
	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_Slow, Category = "Secondary Attributes")
	FGameplayAttributeData Slow;
	ATTRIBUTE_ACCESSORS(UAuraAttributeSet, Slow);
	
	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_Knockback, Category = "Secondary Attributes")
	FGameplayAttributeData Knockback;
	ATTRIBUTE_ACCESSORS(UAuraAttributeSet, Knockback);
	
	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_Stun, Category = "Secondary Attributes")
	FGameplayAttributeData Stun;
	ATTRIBUTE_ACCESSORS(UAuraAttributeSet, Stun);
	
	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_Speed, Category = "Secondary Attributes")
	FGameplayAttributeData Speed;
	ATTRIBUTE_ACCESSORS(UAuraAttributeSet, Speed);
	
	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_HealthRegeneration, Category = "Secondary Attributes")
	FGameplayAttributeData HealthRegeneration;
	ATTRIBUTE_ACCESSORS(UAuraAttributeSet, HealthRegeneration);
	
	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_ManaRegeneration, Category = "Secondary Attributes")
	FGameplayAttributeData ManaRegeneration;
	ATTRIBUTE_ACCESSORS(UAuraAttributeSet, ManaRegeneration);
	
	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_MaxHealth, Category = "Vital Attributes")
	FGameplayAttributeData MaxHealth;
	ATTRIBUTE_ACCESSORS(UAuraAttributeSet, MaxHealth);
	
	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_MaxMana, Category = "Vital Attributes")
	FGameplayAttributeData MaxMana;
	ATTRIBUTE_ACCESSORS(UAuraAttributeSet, MaxMana);
	
	/*
	 * Vital Attributes
	 */
	
	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_Health, Category = "Vital Attributes")
	FGameplayAttributeData Health;
	ATTRIBUTE_ACCESSORS(UAuraAttributeSet, Health);
	
	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_Mana, Category = "Vital Attributes")
	FGameplayAttributeData Mana;
	ATTRIBUTE_ACCESSORS(UAuraAttributeSet, Mana);
	
	
	/*
	 * Meta Attributes
	 */
	
	UPROPERTY(BlueprintReadOnly, Category = "Meta Attributes")
	FGameplayAttributeData IncomingDamage;
	ATTRIBUTE_ACCESSORS(UAuraAttributeSet, IncomingDamage);
	
	UPROPERTY(BlueprintReadOnly, Category = "Meta Attributes")
	FGameplayAttributeData HitReact;
	ATTRIBUTE_ACCESSORS(UAuraAttributeSet, HitReact);
	
	UPROPERTY(BlueprintReadOnly, Category = "Meta Attributes")
	FGameplayAttributeData BlockChance;
	ATTRIBUTE_ACCESSORS(UAuraAttributeSet, BlockChance);
	
	UFUNCTION()
	void OnRep_Health(const FGameplayAttributeData& OldHealth) const;
	
	UFUNCTION()
	void OnRep_Mana(const FGameplayAttributeData& OldMana) const;
	
	
	
	UFUNCTION()
	void OnRep_Fire(const FGameplayAttributeData& OldFire) const;
	
	UFUNCTION()
	void OnRep_Water(const FGameplayAttributeData& OldWater) const;
	
	UFUNCTION()
	void OnRep_Earth(const FGameplayAttributeData& OldEarth) const;
	
	UFUNCTION()
	void OnRep_Air(const FGameplayAttributeData& OldAir) const;
	
	UFUNCTION()
	void OnRep_Intelligence(const FGameplayAttributeData& OldIntelligence) const;
	
	
	
	UFUNCTION()
	void OnRep_DamageOnTime(const FGameplayAttributeData& OldDamageOnTime) const;
	
	UFUNCTION()
	void OnRep_AreaOfEffect(const FGameplayAttributeData& OldAreaOfEffect) const;
	
	UFUNCTION()
	void OnRep_Armor(const FGameplayAttributeData& OldArmor) const;
	
	UFUNCTION()
	void OnRep_ArmorPenetration(const FGameplayAttributeData& OldArmorPenetration) const;
	
	UFUNCTION()
	void OnRep_CriticalHitChance(const FGameplayAttributeData& OldCriticalHitChance) const;
	
	UFUNCTION()
	void OnRep_CriticalHitDamage(const FGameplayAttributeData& OldCriticalHitDamage) const;
    	
	UFUNCTION()
	void OnRep_CriticalHitResistance(const FGameplayAttributeData& OldCriticalHitResistance) const;
	
	UFUNCTION()
	void OnRep_Slow(const FGameplayAttributeData& OldSlow) const;
            	
	UFUNCTION()
	void OnRep_Knockback(const FGameplayAttributeData& OldKnockback) const;
	
	UFUNCTION()
	void OnRep_Stun(const FGameplayAttributeData& OldStun) const;
	
	UFUNCTION()
	void OnRep_Speed(const FGameplayAttributeData& OldSpeed) const;
	
	UFUNCTION()
	void OnRep_HealthRegeneration(const FGameplayAttributeData& OldHealthRegeneration) const;
	
	UFUNCTION()
	void OnRep_ManaRegeneration(const FGameplayAttributeData& OldManaRegeneration) const;
	
	UFUNCTION()
	void OnRep_MaxMana(const FGameplayAttributeData& OldMaxMana) const;
	
	UFUNCTION()
	void OnRep_MaxHealth(const FGameplayAttributeData& OldMaxHealth) const;
	
	UFUNCTION()
	void OnRep_BlockChance(const FGameplayAttributeData& OldBlockChance) const;

	
private:
	
	void SetEffectProperties(const FGameplayEffectModCallbackData& Data, FEffectProperties& Props) const;
	void ShowFloatingText(const FEffectProperties& Props, float Damage, bool bBlockedHit, bool bCriticalHit) const;
};
