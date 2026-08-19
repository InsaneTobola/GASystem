// Copyright Druid Mechanics

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "AICombatComponent.generated.h"

class UCombatProfileInfo;

UCLASS( ClassGroup=(AI), meta=(BlueprintSpawnableComponent) )
class AURA_API UAICombatComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	
	UAICombatComponent();

protected:
	
	virtual void BeginPlay() override;

public:	
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AI|Combat")
	TObjectPtr<UCombatProfileInfo> CombatProfile;
		
};
