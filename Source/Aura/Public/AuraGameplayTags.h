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
	FGameplayTag Attributes_Secondary_Knockup;
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
	
	// Meta
	FGameplayTag Attributes_Meta_IncomingXP;
	TMap<FGameplayTag, FGameplayTag> DamageTypesToPrimaryAttributes;
	TMap<FGameplayTag, FGameplayTag> DamageTypesToResistances;
	
	// Inputs
	FGameplayTag InputTag_Q;
	FGameplayTag InputTag_E;
	FGameplayTag InputTag_R;
	
	FGameplayTag InputTag_1;
	FGameplayTag InputTag_2;
	FGameplayTag InputTag_3;
	FGameplayTag InputTag_4;
	
	FGameplayTag InputTag_LMB;
	FGameplayTag InputTag_RMB;
	FGameplayTag InputTag_W;
	FGameplayTag InputTag_A;
	FGameplayTag InputTag_S;
	FGameplayTag InputTag_D;
	
	// State Tree
	FGameplayTag State_Idle;
	FGameplayTag State_Rotate;
	FGameplayTag State_StartChase;
	FGameplayTag State_StopChase;
	FGameplayTag State_Attack;
	FGameplayTag State_Dead;
	FGameplayTag State_StartCombat;
	FGameplayTag State_FinishedFacingTarget;
	FGameplayTag State_ThreatFacingFinished;
	FGameplayTag State_Alert;
	FGameplayTag State_StartSearch;
	
	// Event Combat
	FGameplayTag Event_PlayerDetected;
	FGameplayTag Event_PlayerLost;
	FGameplayTag Event_AttackFinished;
	FGameplayTag Event_Hit;
	FGameplayTag Event_TargetDead;
	FGameplayTag Event_Knockback;
	FGameplayTag Event_Knockup;
	
	FGameplayTag Damage;
	FGameplayTag HitReact;
	
	// Status Combat
	FGameplayTag Status_Stun;
	FGameplayTag Status_Slow;
	FGameplayTag Status_Burn;
	FGameplayTag Status_Frost;
	FGameplayTag Status_Knockback;
	
	// SetByCaller data
	FGameplayTag Data_ManaCost;
	FGameplayTag Data_Cooldown_Duration;

	// Ability activation failures
	FGameplayTag Ability_Fail_Cost_Mana;
	FGameplayTag Ability_Fail_Cooldown;
	FGameplayTag Ability_Fail_Configuration;

	// UI messages
	FGameplayTag Message_AbilityFail_Mana;
	FGameplayTag Message_AbilityFail_Cooldown;
	FGameplayTag Message_AbilityFail_Generic;
	
private:
	static FAuraGameplayTags GameplayTags;
};