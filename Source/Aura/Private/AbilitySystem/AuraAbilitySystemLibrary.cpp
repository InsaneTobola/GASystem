// Copyright Druid Mechanics


#include "AbilitySystem/AuraAbilitySystemLibrary.h"

#include "AbilitySystemComponent.h"
#include "AuraAbilityTypes.h"
#include "AbilitySystem/Abilities/AuraGameplayAbility.h"
#include "Game/AuraGameModeBase.h"
#include "Kismet/GameplayStatics.h"
#include "Player/AuraPlayerState.h"
#include "UI/HUD/AuraHUD.h"
#include "UI/WidgetController/AuraWidgetController.h"
#include "Data/CombatProfileInfo.h"
#include "Data/AttackDefinition.h"

UOverlayWidgetController* UAuraAbilitySystemLibrary::GetOverlayWidgetController(const UObject* WorldContextObject)
{
	if (APlayerController* PC = UGameplayStatics::GetPlayerController(WorldContextObject, 0))
	{
		if (AAuraHUD* AuraHUD = Cast<AAuraHUD>(PC->GetHUD()))
		{
			AAuraPlayerState* PS = PC->GetPlayerState<AAuraPlayerState>();
			UAbilitySystemComponent* ASC = PS->GetAbilitySystemComponent();
			UAttributeSet* AS = PS->GetAttributeSet();
			const FWidgetControllerParams WidgetControllerParams(PC,PS, ASC, AS);
			return AuraHUD->GetOverlayWidgetController(WidgetControllerParams);
		}
	}
	return nullptr;
}

UAttributeMenuWidgetController* UAuraAbilitySystemLibrary::GetAttributeMenuWidgetController(
	const UObject* WorldContextObject)
{
	if (APlayerController* PC = UGameplayStatics::GetPlayerController(WorldContextObject, 0))
	{
		if (AAuraHUD* AuraHUD = Cast<AAuraHUD>(PC->GetHUD()))
		{
			AAuraPlayerState* PS = PC->GetPlayerState<AAuraPlayerState>();
			UAbilitySystemComponent* ASC = PS->GetAbilitySystemComponent();
			UAttributeSet* AS = PS->GetAttributeSet();
			const FWidgetControllerParams WidgetControllerParams(PC,PS, ASC, AS);
			return AuraHUD->GetAttributeMenuWidgetController(WidgetControllerParams);
		}
	}
	return nullptr;
}

void UAuraAbilitySystemLibrary::InitializeDefaultAttributes(const UObject* WorldContextObject, ECharacterClass CharacterClass, float Level, UAbilitySystemComponent* ASC)
{
 if (!IsValid(ASC))
    {
        UE_LOG(LogTemp,Error,TEXT("InitializeDefaultAttributes: ASC is invalid."));
 		return;
    }

    AActor* AvatarActor = ASC->GetAvatarActor();

    if (!IsValid(AvatarActor))
    {
        UE_LOG(LogTemp,Error,TEXT("InitializeDefaultAttributes: AvatarActor is invalid."));
        return;
    }

    UCharacterClassInfo* CharacterClassInfo = GetCharacterClassInfo(WorldContextObject);

    if (!IsValid(CharacterClassInfo))
    {
        UE_LOG(LogTemp,Error,TEXT("InitializeDefaultAttributes: CharacterClassInfo is missing."));
        return;
    }

    const FCharacterClassDefaultInfo ClassDefaultInfo =CharacterClassInfo->GetClassDefaultInfo(CharacterClass);
    auto ApplyEffectToSelf =[ASC, AvatarActor, Level](TSubclassOf<UGameplayEffect> EffectClass)
        {
            if (!EffectClass)
            {
                UE_LOG(LogTemp,Error,TEXT("InitializeDefaultAttributes: GameplayEffectClass is missing."));
                return;
            }

            FGameplayEffectContextHandle ContextHandle =ASC->MakeEffectContext();
            ContextHandle.AddSourceObject(AvatarActor);
            const FGameplayEffectSpecHandle SpecHandle =ASC->MakeOutgoingSpec(EffectClass,Level,ContextHandle);

            if (!SpecHandle.IsValid() || !SpecHandle.Data.IsValid())
            {
                UE_LOG(LogTemp,Error,TEXT("InitializeDefaultAttributes: Failed to create GameplayEffectSpec."));
                return;
            }

            ASC->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());
        };

    ApplyEffectToSelf(ClassDefaultInfo.PrimaryAttributes);
    ApplyEffectToSelf(CharacterClassInfo->SecondaryAttributes);
    ApplyEffectToSelf(CharacterClassInfo->VitalAttributes);
}

void UAuraAbilitySystemLibrary::GiveStartupAbilities(const UObject* WorldContextObject, UAbilitySystemComponent* ASC)
{
	UCharacterClassInfo* CharacterClassInfo = GetCharacterClassInfo(WorldContextObject);
	
	for (TSubclassOf<UGameplayAbility> AbilityClass: CharacterClassInfo->CommonAbilities)
	{
		FGameplayAbilitySpec AbilitySpec = FGameplayAbilitySpec(AbilityClass, 1);
		ASC->GiveAbility(AbilitySpec);
	}
}

UCharacterClassInfo* UAuraAbilitySystemLibrary::GetCharacterClassInfo(const UObject* WorldContextObject)
{
	AAuraGameModeBase* AuraGameMode = Cast<AAuraGameModeBase>(UGameplayStatics::GetGameMode(WorldContextObject));
	if (AuraGameMode == nullptr) return nullptr;
	
	return AuraGameMode->CharacterClassInfo;
}

bool UAuraAbilitySystemLibrary::IsBlockedHit(const FGameplayEffectContextHandle& EffectContextHandle)
{
	if (const FAuraGameplayEffectContext* AuraEffectContext = static_cast<const FAuraGameplayEffectContext*>(EffectContextHandle.Get()))
	{
		return AuraEffectContext->IsBlockedHit();
	}
	return false;
}

bool UAuraAbilitySystemLibrary::IsCriticalHit(const FGameplayEffectContextHandle& EffectContextHandle)
{
	if (const FAuraGameplayEffectContext* AuraEffectContext = static_cast<const FAuraGameplayEffectContext*>(EffectContextHandle.Get()))
	{
		return AuraEffectContext->IsCriticalHit();
	}
	return false;
}

void UAuraAbilitySystemLibrary::SetIsBlockedHit(FGameplayEffectContextHandle& EffectContextHandle, bool bInIsBlockedHit)
{
		if (FAuraGameplayEffectContext* AuraEffectContext = static_cast<FAuraGameplayEffectContext*>(EffectContextHandle.Get())) 
	{
			AuraEffectContext->SetIsBlockedHit(bInIsBlockedHit);
	}
}

void UAuraAbilitySystemLibrary::SetIsCriticalHit(FGameplayEffectContextHandle& EffectContextHandle,
	bool bInIsCriticalHit)
{
	if (FAuraGameplayEffectContext* AuraEffectContext = static_cast<FAuraGameplayEffectContext*>(EffectContextHandle.Get())) 
	{
		AuraEffectContext->SetIsCriticalHit(bInIsCriticalHit);
	}
}

void UAuraAbilitySystemLibrary::GiveCombatAbilities(const UObject* WorldContextObject, UAbilitySystemComponent* ASC,
	UCombatProfileInfo* CombatProfile)
{
	if (!ASC || !CombatProfile)
	{
		return;
	}

	for (const TObjectPtr<UAttackDefinition>& AttackDefinition : CombatProfile->Attacks)
	{
		if (!AttackDefinition)
		{
			continue;
		}

		if (!AttackDefinition->GameplayAbility)
		{
			continue;
		}

		FGameplayAbilitySpec AbilitySpec(AttackDefinition->GameplayAbility,1);

		ASC->GiveAbility(AbilitySpec);
	}
}

int32 UAuraAbilitySystemLibrary::GetXPRewardForClassAndLevel(const UObject* WorldContextObject, ECharacterClass CharacterClass, int32 CharacterLevel)
{
	UCharacterClassInfo* CharacterClassInfo = GetCharacterClassInfo(WorldContextObject);
	if (CharacterClassInfo == nullptr) return 0;
	
	const FCharacterClassDefaultInfo Info = CharacterClassInfo->GetClassDefaultInfo(CharacterClass);
	const float XPReward = Info.XPReward.GetValueAtLevel(CharacterLevel);
	
	return static_cast<int32>(XPReward);
}
