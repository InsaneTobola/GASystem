#pragma once

#include "CoreMinimal.h"
#include "MagicTypes.h"

class UMagicValidationSettings;

class AURA_API FMagicGestureValidator
{
public:

	static FMagicValidationResult Validate(const FMagicGesture& Gesture,const UMagicValidationSettings& Settings);

private:

	static float CalculatePathLength(const FMagicGesture& Gesture);

	static void CalculateBoundingBox(const FMagicGesture& Gesture,FVector2D& OutMin,FVector2D& OutMax);

	static float CalculateAspectRatio(const FVector2D& BoundingBoxSize);
};