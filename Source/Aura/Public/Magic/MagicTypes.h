#pragma once

#include "CoreMinimal.h"
#include "MagicTypes.generated.h"

UENUM(BlueprintType)
enum class EMagicElement : uint8
{
	Fire,
	Water,
	Earth,
	Air
};

UENUM(BlueprintType)
enum class EMagicState : uint8
{
	Inactive,
	Drawing,
	Targeting
};

UENUM(BlueprintType)
enum class EMagicValidationFailure : uint8
{
    None,

    NoStrokes,

    TooFewPoints,
    TooManyPoints,

    TooManyStrokes,

    PathTooShort,
    PathTooLong,

    BoundingBoxTooSmall,
    BoundingBoxTooLarge,

    AspectRatioTooSmall,
    AspectRatioTooLarge,

    SafetyLimitExceeded
};

USTRUCT(BlueprintType)
struct FMagicStroke
{
    GENERATED_BODY()

    UPROPERTY()
    TArray<FVector2D> Points;

    float StartTime = 0.0f;
    float EndTime = 0.0f;

    void Reset();

    float GetDuration() const;
};

USTRUCT(BlueprintType)
struct FMagicGesture
{
    GENERATED_BODY()

    UPROPERTY()
    TArray<FMagicStroke> Strokes;

    float StartTime = 0.0f;
    float EndTime = 0.0f;

    void Reset();

    int32 GetStrokeCount() const;

    int32 GetTotalPointCount() const;

    float GetDuration() const;
};

USTRUCT(BlueprintType)
struct FMagicValidationResult
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly)
    bool bIsValid = false;

    UPROPERTY(BlueprintReadOnly)
    EMagicValidationFailure FailureReason = EMagicValidationFailure::None;

    UPROPERTY(BlueprintReadOnly)
    int32 StrokeCount = 0;

    UPROPERTY(BlueprintReadOnly)
    int32 TotalPointCount = 0;

    UPROPERTY(BlueprintReadOnly)
    float TotalPathLength = 0.0f;

    UPROPERTY(BlueprintReadOnly)
    FVector2D BoundingBoxMin = FVector2D::ZeroVector;

    UPROPERTY(BlueprintReadOnly)
    FVector2D BoundingBoxMax = FVector2D::ZeroVector;

    UPROPERTY(BlueprintReadOnly)
    FVector2D BoundingBoxSize = FVector2D::ZeroVector;

    UPROPERTY(BlueprintReadOnly)
    float AspectRatio = 0.0f;

    UPROPERTY(BlueprintReadOnly)
    float GestureDuration = 0.0f;
};

USTRUCT(BlueprintType)
struct FMagicPatternValidationRules
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    int32 MinStrokeCount = 1;

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    int32 MaxStrokeCount = 16;

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    int32 MinPointCount = 2;

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    int32 MaxPointCount = 4096;

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    float MinPathLength = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    float MaxPathLength = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    float MinWidth = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    float MaxWidth = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    float MinHeight = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    float MaxHeight = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    float MinAspectRatio = 0.01f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    float MaxAspectRatio = 100.0f;
};