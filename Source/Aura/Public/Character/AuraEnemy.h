// Copyright Druid Mechanics

#pragma once

#include "CoreMinimal.h"
#include "Character/AuraCharacterBase.h"
#include "Interaction/EnemyInterface.h"
#include "UI/WidgetController/OverlayWidgetController.h"
#include "Data/CharacterClassInfo.h"
#include "Perception/AIPerceptionTypes.h"
#include "Navigation/PathFollowingComponent.h"
#include "AuraEnemy.generated.h"


class UWidgetComponent;
class UCombatProfileInfo;
class UAIPerceptionComponent;
class UAISenseConfig_Sight;
class UStateTreeComponent;

/**
 * 
 */
UCLASS()
class AURA_API AAuraEnemy : public AAuraCharacterBase, public IEnemyInterface
{
	GENERATED_BODY()
public:
	
	AAuraEnemy();
	
	virtual void Tick(float DeltaTime) override;
	
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
	
	UPROPERTY(BlueprintReadOnly, Category = "Combat")
	float BaseWalkSpeed = 150.f;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat")
	float LifeSpan = 5.f;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat")
	TObjectPtr<UCombatProfileInfo> CombatProfile;

	UPROPERTY()
	TObjectPtr<UStateTreeComponent> StateTreeComponent;
	
	UPROPERTY(EditDefaultsOnly, Category = "Combat")
	float AttackRange = 180.f;
	
	UFUNCTION(BlueprintCallable, Category = "Combat")
	bool IsTargetInAttackRange() const;
	
	
	// ==========================================
	// Chase 
	// ==========================================
	UFUNCTION(BlueprintCallable, Category = "AI")
	void StartChasing(AActor* TargetActor);
	
	// ==========================================
	//Search
	// ==========================================
	UFUNCTION(BlueprintCallable, Category = "AI|Search")
	void StartSearching();

	UFUNCTION(BlueprintCallable, Category = "AI|Search")
	void StopSearching();
	
	void UpdateLastKnownTargetLocation(AActor* TargetActor);
	
	UPROPERTY(BlueprintReadOnly, Category = "AI|Search")
	FVector LastKnownTargetLocation;

	UPROPERTY(BlueprintReadOnly, Category = "AI|Search")
	bool bHasLastKnownTargetLocation = false;
	
	// ==========================================
	//Look Around
	// ==========================================
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AI|Search")
	float LookAroundAngle = 90.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AI|Search")
	float LookAroundRotationSpeed = 4.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AI|Search")
	float LookAroundPause = 1.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AI|Search")
	float LookAroundTolerance = 2.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AI|Search")
	int32 MinLookAroundCount = 3;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AI|Search")
	int32 MaxLookAroundCountSetting = 5;
	
	// ==========================================
	//Face Target Combat
	// ==========================================
	UFUNCTION(BlueprintCallable, Category = "Combat")
	void FaceCurrentTarget();
	
	bool bIsFacingCurrentTarget = false;
	
	UFUNCTION(BlueprintCallable, Category = "Combat")
	bool IsFacingCurrentTarget() const;
	
	UPROPERTY(EditAnywhere, Category = "Combat")
	float RotationInterpSpeed = 8.f;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat")
	float FacingTolerance = 5.f;
	
	

protected:
	// ==========================================
    // Lifecycle
    // ==========================================

    virtual void BeginPlay() override;
    virtual void InitAbilityActorInfo() override;
    virtual void InitializeDefaultAttributes() const override;


    // ==========================================
    // Character Class
    // ==========================================

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Character Class Default")
    int32 Level = 1;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Character Class Default")
    EcharacterClass CharacterClass = EcharacterClass::Warrior;


    // ==========================================
    // Components
    // ==========================================

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    TObjectPtr<UWidgetComponent> HealthBar;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI|Perception")
    TObjectPtr<UAIPerceptionComponent> AIPerception;

    UPROPERTY()
    TObjectPtr<UAISenseConfig_Sight> SightConfig;


    // ==========================================
    // AI - Target
    // ==========================================

    UPROPERTY(BlueprintReadOnly, Category = "AI|Target")
    TObjectPtr<APawn> TargetPawn = nullptr;

    UPROPERTY()
    TObjectPtr<AActor> CurrentTarget = nullptr;


    // ==========================================
    // AI - Perception Settings
    // ==========================================

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AI|Perception")
    float SightRadius = 1500.f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AI|Perception")
    float LoseSightRadius = 1800.f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AI|Perception")
    float PeripheralVisionAngle = 80.f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AI|Perception")
    float LoseSightDelay = 2.f;

    FTimerHandle LoseSightTimerHandle;


    // ==========================================
    // AI - Perception Logic
    // ==========================================

    UFUNCTION()
    void OnTargetPerceptionUpdated(AActor* Actor,FAIStimulus Stimulus);

    void HandlePlayerDetected(AActor* Player);

    void HandlePlayerLost();

    void ExecuteStopChase();


    // ==========================================
    // AI - Search
    // ==========================================

    void OnSearchMoveCompleted(FAIRequestID RequestID, const FPathFollowingResult& Result);

    void FinishSearch();

    bool bIsSearching = false;

    FAIRequestID SearchMoveRequestID;


    // ==========================================
    // AI - Search / Look Around
    // ==========================================

    void StartLookingAround();

    void StartNextLookAround();

    void FinishLookingAround();

    bool bIsLookingAround = false;

    bool bIsTurningDuringSearch = false;

    FRotator SearchLookTargetRotation;

    int32 CurrentLookAroundCount = 0;

    int32 MaxLookAroundCount = 0;

    FTimerHandle LookAroundTimerHandle;


    // ==========================================
    // StateTree Events
    // ==========================================

    UFUNCTION(BlueprintImplementableEvent, Category = "AI|StateTree")
    void OnPlayerDetected(AActor* Player);

    UFUNCTION(BlueprintImplementableEvent, Category = "AI|StateTree")
    void OnPlayerLost();

    UFUNCTION(BlueprintImplementableEvent, Category = "AI|StateTree")
    void OnSearchFinished();

    UFUNCTION(BlueprintImplementableEvent, Category = "AI|StateTree")
    void OnFinishedFacingTarget();
};
