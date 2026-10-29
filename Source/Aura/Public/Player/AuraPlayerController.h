// Copyright Druid Mechanics

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "GameplayTagContainer.h"
#include "Magic/MagicTypes.h"
#include "AuraPlayerController.generated.h"

class UInputMappingContext;
class UInputAction;
class UDamageTextComponent;
class IEnemyInterface;
class UAuraInputConfig;
class UAuraAbilitySystemComponent;
class USplineComponent;
class UAuraMagicDrawingWidget;
class UAuraMagicComponent;
class SUAuraMagicDrawingWidget;
class AAuraMagicTargetingActor;
class UMagicPatternDefinition;

struct FInputActionValue;
/**
 * 
 */
UCLASS()
class AURA_API AAuraPlayerController : public APlayerController
{
	GENERATED_BODY()
public:
	AAuraPlayerController();
	virtual void PlayerTick(float DeltaTime) override;

	UAuraMagicComponent* GetMagicComponent() const;
	void MagicInputPressed(FGameplayTag InputTag);
	void HandleMagicInput(const FGameplayTag& InputTag);
	
	UFUNCTION(Client, Reliable)
	void ShowDamageNumber(float DamageAmount, ACharacter* TargetCharacter, bool bBlockedHit, bool bCriticalHit);
	void StartMagicDrawingMode();
	void StopMagicDrawingMode();
	void ConfirmMagicDrawingMode();
	
	void StartMagicTargeting(UMagicPatternDefinition* Pattern);
	void CancelMagicTargeting();
	void ConfirmMagicTargeting();
	
	UFUNCTION(Client, Reliable)
	void ClientMagicTargetAccepted();

	UFUNCTION(Client, Reliable)
	void ClientMagicTargetRejected();
	
	UFUNCTION(Server, Reliable)
	void ServerConfirmMagicSelfCast(FName PatternId, EMagicElement Element);

	UFUNCTION(Client, Reliable)
	void ClientMagicSelfCastAccepted();

	UFUNCTION(Client, Reliable)
	void ClientMagicSelfCastRejected();
protected:
	virtual void BeginPlay() override;
	virtual void SetupInputComponent() override;

private:
	UPROPERTY(EditAnywhere, Category="Input")
	TObjectPtr<UInputMappingContext> AuraContext;

	UPROPERTY(EditAnywhere, Category="Input")
	TObjectPtr<UInputAction> MoveAction;

	void Move(const FInputActionValue& InputActionValue);

	void CursorTrace();
	IEnemyInterface* LastActor;
	IEnemyInterface* ThisActor;
	FHitResult CursorHit;

	void AbilityInputTagPressed(FGameplayTag InputTag);
	void AbilityInputTagReleased(FGameplayTag InputTag);
	void AbilityInputTagHeld(FGameplayTag InputTag);

	UPROPERTY(EditDefaultsOnly, Category="Input")
	TObjectPtr<UAuraInputConfig> InputConfig;

	UPROPERTY()
	TObjectPtr<UAuraAbilitySystemComponent> AuraAbilitySystemComponent;

	UAuraAbilitySystemComponent* GetASC();

	
	FVector CachedDestination = FVector::ZeroVector;
	float FollowTime = 0.f;
	float ShortPressThreshold = 0.5f;
	bool bAutoRunning = false;
	bool bTargeting = false;

	UPROPERTY(EditDefaultsOnly)
	float AutoRunAcceptanceRadius = 50.f;

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<USplineComponent> Spline;
	
	void AutoRun();
	
	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UDamageTextComponent> DamageTextComponentClass;
	
	// Magic
	UPROPERTY(EditDefaultsOnly, Category = "Magic")
	TSubclassOf<UAuraMagicDrawingWidget> MagicDrawingWidgetClass;

	UPROPERTY()
	TObjectPtr<UAuraMagicDrawingWidget> MagicDrawingWidget;
	
	bool bMagicDrawingMode = false;

	
	UPROPERTY(EditDefaultsOnly, Category = "Magic|Targeting")
	TSubclassOf<AAuraMagicTargetingActor> MagicTargetingActorClass;

	UPROPERTY()
	TObjectPtr<AAuraMagicTargetingActor> MagicTargetingActor;

	FName PendingMagicPatternId = NAME_None;

	EMagicElement PendingMagicElement = EMagicElement::Fire;

	bool bMagicTargetingMode = false;

	bool bWaitingForMagicCastResponse = false;

	UFUNCTION(Server, Reliable)
	void ServerConfirmMagicTarget(FName PatternId,EMagicElement Element,FVector TargetLocation);

	
	
};