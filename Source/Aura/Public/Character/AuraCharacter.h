// Copyright Druid Mechanics

#pragma once

#include "CoreMinimal.h"
#include "Character/AuraCharacterBase.h"
#include "Interaction/PlayerInterface.h"
#include "AuraCharacter.generated.h"

class UAuraMagicComponent;
/**
 * 
 */
UCLASS()
class AURA_API AAuraCharacter : public AAuraCharacterBase, public IPlayerInterface
{
	GENERATED_BODY()
public:
	AAuraCharacter();
	virtual void PossessedBy(AController* NewController) override;
	virtual void OnRep_PlayerState() override;
	
	/** Combat Interface */
	virtual int32 GetPlayerLevel_Implementation() override;
	
	/** End Combat Interface */
	
	/** Player Interface */
	virtual void AddToXP_Implementation(int32 IxXP) override;
	virtual void LevelUp_Implementation() override;
	
	/** End Player Interface */
	
	/** Magic Interface**/
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Magic")
	TObjectPtr<UAuraMagicComponent> MagicComponent;
	
	/** End Magic Interface */
	
private:
	virtual void InitAbilityActorInfo() override;
};
