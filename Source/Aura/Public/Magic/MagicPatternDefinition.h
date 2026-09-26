#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "MagicTypes.h"
#include "MagicPatternDefinition.generated.h"

UCLASS(BlueprintType)
class AURA_API UMagicPatternDefinition : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Magic")
	EMagicElement Element = EMagicElement::Fire;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Validation")
	FMagicPatternValidationRules ValidationRules;
};