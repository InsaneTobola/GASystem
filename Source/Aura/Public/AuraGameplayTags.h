// Copyright Druid Mechanics

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"

/**
 * AuraGameplayTags
 * 
 * Singleton containing native GameplayTags
 */
struct FAuraGameplayTags
{
public:
	static const FAuraGameplayTags& Get() {return GameplayTags;}
	static void InitializeNativeGameplayTags();
	
	FGameplayTag Attributes_Primary_Fire;
	FGameplayTag Attributes_Primary_Water;
	FGameplayTag Attributes_Primary_Earth;
	FGameplayTag Attributes_Primary_Air;
	
	FGameplayTag Damage_Fire;
	FGameplayTag Damage_Water;
	FGameplayTag Damage_Earth;
	FGameplayTag Damage_Air;
	
	FGameplayTag Attributes_Secondary_DamageOnTime;
	FGameplayTag Attributes_Secondary_Armor;
	FGameplayTag Attributes_Secondary_CriticalHitChance;
	FGameplayTag Attributes_Secondary_CriticalHitDamage;
	FGameplayTag Attributes_Secondary_CriticalHitResistance;
	FGameplayTag Attributes_Secondary_HealthRegeneration;
	FGameplayTag Attributes_Secondary_ManaRegeneration;
	FGameplayTag Attributes_Secondary_Slow;
	FGameplayTag Attributes_Secondary_Knockback;
	FGameplayTag Attributes_Secondary_Stun;
	FGameplayTag Attributes_Secondary_Speed;
	FGameplayTag Attributes_Secondary_AreaOfEffect;
	FGameplayTag Attributes_Secondary_MaxHealth;
	FGameplayTag Attributes_Secondary_MaxMana;
	FGameplayTag Attributes_Secondary_ArmorPenetration;
	FGameplayTag Attributes_Secondary_BlockChance;
	
	FGameplayTag Attributes_Resistance_Fire;
	FGameplayTag Attributes_Resistance_Water;
	FGameplayTag Attributes_Resistance_Earth;
	FGameplayTag Attributes_Resistance_Air;
	
	FGameplayTag InputTag_Q;
	FGameplayTag InputTag_W;
	FGameplayTag InputTag_E;
	FGameplayTag InputTag_R;
	
	FGameplayTag InputTag_1;
	FGameplayTag InputTag_2;
	FGameplayTag InputTag_3;
	
	FGameplayTag InputTag_LMB;
	FGameplayTag InputTag_RMB;
	
	FGameplayTag Damage;
	FGameplayTag HitReact;
	
	TMap<FGameplayTag, FGameplayTag> DamageTypesToResistances;
	
	FGameplayTag State_Idle;
	FGameplayTag State_Rotate;
	FGameplayTag State_StartChase;
	FGameplayTag State_StopChase;
	FGameplayTag State_Attack;
	FGameplayTag State_Dead;
	FGameplayTag State_StartCombat;
	
	FGameplayTag Event_PlayerDetected;
	FGameplayTag Event_PlayerLost;
	FGameplayTag Event_AttackFinished;
	FGameplayTag Event_Hit;
	FGameplayTag Event_TargetDead;
	FGameplayTag Event_Knockback;
	FGameplayTag Event_Knockup;
	
	FGameplayTag Status_Stun;
	FGameplayTag Status_Slow;
	FGameplayTag Status_Burn;
	FGameplayTag Status_Frost;
	
	
	
private:
	static FAuraGameplayTags GameplayTags;
};