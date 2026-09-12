#pragma once

#include "CoreMinimal.h"
#include "AIAuraThreatTypes.generated.h"

class AActor;
class AAuraEnemy;

UENUM(BlueprintType)
enum class EThreatType : uint8
{
	Sight,
	Damage,
	Explosion
};

USTRUCT(BlueprintType)
struct AURA_API FThreatAlert
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TObjectPtr<AActor> SourceActor = nullptr;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TObjectPtr<AAuraEnemy> WitnessEnemy = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector ThreatLocation = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName AlertGroupId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EThreatType ThreatType = EThreatType::Sight;
};