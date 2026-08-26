// Copyright Druid Mechanics

#pragma once

#include "CoreMinimal.h"
#include "Character/AuraCharacterBase.h"
#include "Interaction/EnemyInterface.h"
#include "UI/WidgetController/OverlayWidgetController.h"
#include "Data/CharacterClassInfo.h"
#include "Perception/AIPerceptionTypes.h"
#include "AuraEnemy.generated.h"


class UWidgetComponent;
class UCombatProfileInfo;
class UAIPerceptionComponent;
class UAISenseConfig_Sight;

/**
 * 
 */
UCLASS()
class AURA_API AAuraEnemy : public AAuraCharacterBase, public IEnemyInterface
{
	GENERATED_BODY()
public:
	
	AAuraEnemy();
	
	/** Enemy Interface */
	virtual void HighlightActor() override;
	virtual void UnHighlightActor() override;
	/** End Enemy Interface */

	/** Combat Interface */
	virtual int32 GetPlayerLevel() override;
	virtual void Die() override;
	/** End Combat Interface */
	
	UPROPERTY(BlueprintAssignable)
	FOnAttributeChangedSignature OnHealthChanged;
	
	UPROPERTY(BlueprintAssignable)
	FOnAttributeChangedSignature OnMaxHealthChanged;
	
	void HitReactTagChanged(const FGameplayTag CallbackTag, int32 NewCount);
	
	UPROPERTY(BlueprintReadOnly, Category = "Combat")
	bool bHitReacting = false;
	
	UPROPERTY(BlueprintReadOnly,BlueprintReadOnly, Category = "Combat")
	float BaseWalkSpeed = 100.f;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat")
	float LifeSpan = 5.f;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat")
	TObjectPtr<UCombatProfileInfo> CombatProfile;

protected:
	virtual void BeginPlay() override;
	virtual void InitAbilityActorInfo() override;
	virtual void InitializeDefaultAttributes() const override;

	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Character Class Default")
	int32 Level = 1;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Character Class Default")
	EcharacterClass CharacterClass = EcharacterClass::Warrior;
	
	UPROPERTY(VisibleAnywhere, BLueprintReadOnly)
	TObjectPtr<UWidgetComponent> HealthBar;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AI|Perception")
	float SightRadius = 1500.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AI|Perception")
	float LoseSightRadius = 1800.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AI|Perception")
	float PeripheralVisionAngle = 80.f;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AI|Perception")
	float LoseSightDelay = 5.f;
	
	FTimerHandle LoseSightTimerHandle;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI")
	TObjectPtr<UAIPerceptionComponent> AIPerception;
	
	UPROPERTY()
	TObjectPtr<UAISenseConfig_Sight> SightConfig;
	
	UFUNCTION(BlueprintImplementableEvent, Category = "AI|StateTree")
	void OnPlayerDetected(AActor* Player);

	UFUNCTION(BlueprintImplementableEvent, Category = "AI|StateTree")
	void OnPlayerLost();
	
	UFUNCTION()
	void OnTargetPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus);
	
	void HandlePlayerDetected(AActor* Player);
	void HandlePlayerLost();
	void ExecuteStopChase();
	
};
