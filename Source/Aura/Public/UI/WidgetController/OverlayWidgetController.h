// Copyright Druid Mechanics

#pragma once

#include "CoreMinimal.h"
#include "GameplayEffectTypes.h"
#include "UI/WidgetController/AuraWidgetController.h"
#include "OverlayWidgetController.generated.h"

USTRUCT(BlueprintType)
struct  FUIWidgetRow : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, EditAnywhere)
	FGameplayTag MassageTag = FGameplayTag();
	
	UPROPERTY(BlueprintReadOnly, EditAnywhere)
	FText Message = FText();
	
	UPROPERTY(BlueprintReadOnly, EditAnywhere)
	TSubclassOf<class UAuraUserWidget> MessageWidget;
	
	UPROPERTY(BlueprintReadOnly, EditAnywhere)
	UTexture2D* Image = nullptr;
	
};

class UAuraUserWidget;
class UAbilitySystemComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnAttributeChangedSignature, float, NewHealth);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FMessageWidgetRowSignature, FUIWidgetRow, Row);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_FourParams(FOnAbilityCooldownChangedSignature,FGameplayTag, CooldownTag,bool, bIsActive,float, Duration,float, TimeRemaining);
/**
 * 
 */
UCLASS(BlueprintType, Blueprintable)
class AURA_API UOverlayWidgetController : public UAuraWidgetController
{
	GENERATED_BODY()
public:
	virtual void BroadcastInitialValues() override;
	virtual void BindCallbacksToDependencies() override;
	
	UPROPERTY(BlueprintAssignable, Category = "GAS|Attribute")
	FOnAttributeChangedSignature OnHealthChanged;
	
	UPROPERTY(BlueprintAssignable, Category = "GAS|Attribute")
	FOnAttributeChangedSignature OnMaxHealthChanged;
	
	UPROPERTY(BlueprintAssignable, Category = "GAS|Attribute")
	FOnAttributeChangedSignature OnManaChanged;
	
	UPROPERTY(BlueprintAssignable, Category = "GAS|Attribute")
	FOnAttributeChangedSignature OnMaxManaChanged;
	
	UPROPERTY(BlueprintAssignable, Category = "GAS|Messages")
	FMessageWidgetRowSignature MessageWidgetRowDelegate;
	
	UPROPERTY(BlueprintAssignable, Category = "GAS|XP")
	FOnAttributeChangedSignature OnXPPercentChangedDelegate;
	
	UPROPERTY(BlueprintAssignable, Category = "GAS|XP")
	FOnPlayerStatChangedSignature OnPlayerLevelChangedDelegate;
	
	UPROPERTY(BlueprintAssignable, Category = "GAS|Cooldown")
	FOnAbilityCooldownChangedSignature OnAbilityCooldownChanged;

	UFUNCTION(BlueprintPure, Category = "GAS|Cooldown")
	void GetCooldownTimes(FGameplayTag CooldownTag,float& OutRemaining,float& OutDuration) const;
	
	UFUNCTION(Client, Reliable)
	void ClientMagicAbilityFailed(FGameplayTag MessageTag);
	
protected:
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Widget Data")
	TObjectPtr<UDataTable> MessageWidgetDataTable;
	
	template<typename T>
	T* GetDataTableRowByTag(UDataTable* DataTable, const  FGameplayTag& Tag);
	
	void OnXPChanged(int32 NewXP) const;
	
	void HandleActiveEffectAdded(UAbilitySystemComponent* ASC,const FGameplayEffectSpec& Spec,FActiveGameplayEffectHandle Handle);
	void RegisterCooldownTag(const FGameplayTag& CooldownTag);
	void HandleCooldownTagChanged(const FGameplayTag CallbackTag,int32 NewCount);
	void HandleAbilityFailed(const UGameplayAbility* FailedAbility,const FGameplayTagContainer& FailureTags);

	FGameplayTagContainer RegisteredCooldownTags;
};

template <typename T>
T* UOverlayWidgetController::GetDataTableRowByTag(UDataTable* DataTable, const FGameplayTag& Tag)
{
	if (!IsValid(DataTable))
	{
		return nullptr;
	}

	return DataTable->FindRow<T>(Tag.GetTagName(),TEXT(""));
}
