#pragma once

#include "CoreMinimal.h"
#include "MagicTypes.h"

class UMagicValidationSettings;

class AURA_API FMagicGestureValidator
{
public:

	static FMagicValidationResult Validate(const FMagicGesture& Gesture,const UMagicValidationSettings& Settings);

private:

	static bool ValidateStrokeCount(const FMagicGesture& Gesture,const UMagicValidationSettings& Settings,EMagicValidationFailure& OutFailure);

	static bool ValidatePointCount(const FMagicGesture& Gesture,const UMagicValidationSettings& Settings,EMagicValidationFailure& OutFailure);

	static float CalculatePathLength(const FMagicGesture& Gesture);

	static float CalculateStrokePathLength(const FMagicStroke& Stroke);

	static bool ValidatePathLength(const FMagicGesture& Gesture,float TotalPathLength,const UMagicValidationSettings& Settings,EMagicValidationFailure& OutFailure);

	static void CalculateBoundingBox(const FMagicGesture& Gesture,FVector2D& OutMin,FVector2D& OutMax);

	static bool ValidateBoundingBox(const FVector2D& BoundingBoxSize,const UMagicValidationSettings& Settings,EMagicValidationFailure& OutFailure);

	static float CalculateAspectRatio(const FVector2D& BoundingBoxSize);

	static bool ValidateAspectRatio(float AspectRatio,const UMagicValidationSettings& Settings,EMagicValidationFailure& OutFailure);
};