#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "MagicValidationSettings.generated.h"

UCLASS(
    config = Game,
    defaultconfig,
    meta = (DisplayName = "Magic Validation")
)
class AURA_API UMagicValidationSettings : public UDeveloperSettings
{
    GENERATED_BODY()

public:

    UPROPERTY(Config,EditAnywhere,BlueprintReadOnly,Category = "Strokes")
    int32 MaxStrokeCount = 16;

    UPROPERTY(Config,EditAnywhere,BlueprintReadOnly,Category = "Points")
    int32 MaxPointsPerStroke = 1024;

    UPROPERTY(Config,EditAnywhere,BlueprintReadOnly,Category = "Points")
    int32 MaxTotalPointCount = 4096;

    UPROPERTY(Config, EditAnywhere,BlueprintReadOnly,Category = "Points")
    float MinPointDistance = 0.0025f;

    UPROPERTY(Config,EditAnywhere,BlueprintReadOnly,Category = "Points")
    int32 MinTotalPointCount = 2;

    UPROPERTY(Config,EditAnywhere,Category = "Path")
    float MinPathLength = 0.01f;

    UPROPERTY(Config,EditAnywhere,BlueprintReadOnly,Category = "Path")
    float MaxPathLength = 12.0f;

    UPROPERTY(Config,EditAnywhere,BlueprintReadOnly,Category = "Bounding Box")
    float MinBoundingBoxDimension = 0.02f;

    UPROPERTY(Config,EditAnywhere,BlueprintReadOnly,Category = "Bounding Box")
    float MaxBoundingBoxWidth = 0.95f;

    UPROPERTY(Config,EditAnywhere,BlueprintReadOnly,Category = "Bounding Box")
    float MaxBoundingBoxHeight = 0.95f;

    UPROPERTY(Config,EditAnywhere,BlueprintReadOnly,Category = "Aspect Ratio")
    float MinAspectRatio = 0.05f;

    UPROPERTY(Config,EditAnywhere,BlueprintReadOnly,Category = "Aspect Ratio")
    float MaxAspectRatio = 20.0f;
};