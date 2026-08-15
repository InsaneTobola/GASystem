// Copyright Druid Mechanics


#include "AuraGameplayTags.h"
#include "GameplayTagsManager.h"

FAuraGameplayTags FAuraGameplayTags::GameplayTags;

void FAuraGameplayTags::InitializeNativeGameplayTags()
{
	/*
	 *Primary Attributes
	 */
	GameplayTags.Attributes_Primary_Fire = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("Attributes.Primary.Fire"), 
		FString("Fire magic")
		);
	GameplayTags.Attributes_Primary_Water = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("Attributes.Primary.Water"), 
		FString("Water magic")
		);
	GameplayTags.Attributes_Primary_Earth = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("Attributes.Primary.Earth"), 
		FString("Earth magic")
		);
	GameplayTags.Attributes_Primary_Air = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("Attributes.Primary.Air"), 
		FString("Air magic")
		);
	
	/*
	 * Damage Type
	 */
	
	GameplayTags.Damage_Fire = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("Damage.Fire"), 
		FString("Fire Damage Type")
		);
	GameplayTags.Damage_Water = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("Damage.Water"), 
		FString("Water Damage Type")
		);
	GameplayTags.Damage_Earth = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("Damage.Earth"), 
		FString("Earth Damage Type")
		);
	GameplayTags.Damage_Air = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("Damage.Air"), 
		FString("Air Damage Type")
		);
	
	/*
	 * Damage Resistances
	 */
	GameplayTags.Attributes_Resistance_Fire = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("Resistance.Fire"), 
		FString("Fire Resistance")
		);
	GameplayTags.Attributes_Resistance_Water = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("Resistance.Water"), 
		FString("Water Resistance")
		);
	GameplayTags.Attributes_Resistance_Earth = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("Resistance.Earth"), 
		FString("Earth Resistance")
		);
	GameplayTags.Attributes_Resistance_Air = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("Resistance.Air"), 
		FString("Air Resistance")
		);
	
	/*
	 * Map of Damage Types to Resistance
	 */
	
	GameplayTags.DamageTypesToResistances.Add(GameplayTags.Damage_Fire, GameplayTags.Attributes_Resistance_Fire);
	GameplayTags.DamageTypesToResistances.Add(GameplayTags.Damage_Water, GameplayTags.Attributes_Resistance_Water);
	GameplayTags.DamageTypesToResistances.Add(GameplayTags.Damage_Earth, GameplayTags.Attributes_Resistance_Earth);
	GameplayTags.DamageTypesToResistances.Add(GameplayTags.Damage_Air, GameplayTags.Attributes_Resistance_Air);
	
	/*
	 *Secondary Attributes
	 */
	GameplayTags.Attributes_Secondary_DamageOnTime = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("Attributes.Secondary.DamageOnTime"), 
		FString("Increases Damage on time")
		);
	GameplayTags.Attributes_Secondary_Armor = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("Attributes.Secondary.Armor"), 
		FString("Reduces damage taken, improves Block Chance")
		);
	GameplayTags.Attributes_Secondary_CriticalHitChance = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("Attributes.Secondary.CriticalHitChance"), 
		FString("Improves crit hit Chance")
		);
	GameplayTags.Attributes_Secondary_CriticalHitDamage = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("Attributes.Secondary.CriticalHitDamage"), 
		FString("Improves crit hit damage")
		);
	GameplayTags.Attributes_Secondary_CriticalHitResistance = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("Attributes.Secondary.CriticalHitResistance"), 
		FString("Improves crit hit resistance")
		);
	GameplayTags.Attributes_Secondary_HealthRegeneration = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("Attributes.Secondary.HealthRegeneration"), 
		FString("Increases health regen")
		);
	GameplayTags.Attributes_Secondary_ManaRegeneration = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("Attributes.Secondary.ManaRegeneration"), 
		FString("Increases mana regen")
		);
	GameplayTags.Attributes_Secondary_Slow = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("Attributes.Secondary.Slow"), 
		FString("Extends slow duration")
		);
	GameplayTags.Attributes_Secondary_Knockback = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("Attributes.Secondary.Knockback"), 
		FString("Extends knockback distance")
		);
	GameplayTags.Attributes_Secondary_Stun = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("Attributes.Secondary.Stun"), 
		FString("Extends stun duration")
		);
	GameplayTags.Attributes_Secondary_Speed = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("Attributes.Secondary.Speed"), 
		FString("Increases movement speed")
		);
	GameplayTags.Attributes_Secondary_AreaOfEffect = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("Attributes.Secondary.AreaOfEffect"), 
		FString("Increases the area of effect of AoE skills")
		);
	GameplayTags.Attributes_Secondary_MaxHealth = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("Attributes.Secondary.MaxHealth"), 
		FString("Increases Max Health")
		);
	GameplayTags.Attributes_Secondary_MaxMana = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("Attributes.Secondary.MaxMana"), 
		FString("Increases Max Mana")
		);
	
	GameplayTags.Attributes_Secondary_ArmorPenetration = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("Attributes.Secondary.ArmorPenetration"), 
		FString("Increases ArmorPenetration")
		);
	
	GameplayTags.Attributes_Secondary_BlockChance = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("Attributes.Secondary.BlockChance"), 
		FString("Increases BlockChance")
		);
	
	/*
	 * Input Tags
	 */
	GameplayTags.InputTag_Q = UGameplayTagsManager::Get().AddNativeGameplayTag(
	FName("InputTag.Q"), 
FString("InputTag for button Q")
	);
	GameplayTags.InputTag_W = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("InputTag.W"), 
	FString("InputTag for button W")
		);
	GameplayTags.InputTag_E = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("InputTag.E"), 
	FString("InputTag for button E")
		);
	GameplayTags.InputTag_R = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("InputTag.R"), 
	FString("InputTag for button R")
		);
	
	/*
	 * Input Tags Variant
	 */
	GameplayTags.InputTag_1 = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("InputTag.1"), 
		FString("InputTag for button 1")
		);
	GameplayTags.InputTag_2 = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("InputTag.2"), 
	FString("InputTag for button 2")
		);
	GameplayTags.InputTag_3 = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("InputTag.3"), 
	FString("InputTag for button 3")
		);
	
	
	GameplayTags.InputTag_LMB = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("InputTag.LMB"), 
	FString("InputTag for Left Mouse Button, confirmation of a skill roll")
		);
	GameplayTags.InputTag_RMB = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("InputTag.RMB"), 
	FString("InputTag for Right Mouse Button, direction of travel")
		);
	
	/*
	 * Reaction
	 */
	GameplayTags.Damage = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("Damage"), 
	FString("Damage")
		);
	GameplayTags.HitReact = UGameplayTagsManager::Get().AddNativeGameplayTag(
			FName("HitReact"), 
		FString("HitReact")
			);
	
	/*
	 * State tree
	 */
	GameplayTags.State_Idle = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("State.Idle"), 
	FString("Idle")
		);
	GameplayTags.State_Rotate = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("State.Rotate"), 
	FString("Rotate")
		);
	GameplayTags.State_StartChase = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("State.StartChase"), 
	FString("Start Chase")
		);
	GameplayTags.State_StopChase = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("State.StopChase"), 
	FString("Stop Chase")
		);
	GameplayTags.State_Attack= UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("State.Attack"), 
	FString("Attack")
		);
	GameplayTags.State_Dead = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("State.Dead"), 
	FString("Dead")
		);
	
	/*
	 * Event Effect
	 */
	GameplayTags.Event_PlayerDetected = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("Event.PlayerDetected"), 
	FString("PlayerDetected")
		);
	GameplayTags.Event_PlayerLost = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("Event.PlayerLost"), 
	FString("PlayerLost")
		);
	GameplayTags.Event_AttackFinished = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("Event.AttackFinished"), 
	FString("AttackFinished")
		);
	GameplayTags.Event_Hit = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("Event.Hit"), 
	FString("Hit")
		);
	GameplayTags.Event_TargetDead = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("Event.TargetDead"), 
	FString("TargetDead")
		);
	GameplayTags.Event_Knockup = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("Event.Knockup"), 
	FString("Knockup")
		);
	GameplayTags.Event_Knockback = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("Event.Knockback"), 
	FString("Knockback")
		);
	
	/*
	 * Status Effect
	 */
	GameplayTags.Status_Stun = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("Status.Stun"), 
	FString("Stun")
		);
	GameplayTags.Status_Slow = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("Status.Slow"), 
	FString("Slow")
		);
	GameplayTags.Status_Burn= UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("Status.Burn"), 
	FString("Burn")
		);
	GameplayTags.Status_Frost = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("Status.Frost"), 
	FString("Frost")
		);
	
}

