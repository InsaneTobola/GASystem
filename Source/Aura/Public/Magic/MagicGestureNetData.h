#pragma once

#include "CoreMinimal.h"
#include "Magic/MagicTypes.h"
#include "MagicGestureNetData.generated.h"

class UMagicValidationSettings;

USTRUCT()
struct AURA_API FMagicGestureNetData
{
	GENERATED_BODY()

public:

	// Quantized X and Y coordinates.
	UPROPERTY()
	TArray<uint16> QuantizedCoordinates;

	// Number of points in each individual stroke.
	UPROPERTY()
	TArray<uint16> StrokePointCounts;

	// Stroke timing relative to gesture start.
	UPROPERTY()
	TArray<float> StrokeStartOffsets;

	UPROPERTY()
	TArray<float> StrokeEndOffsets;

	// Preserves FMagicGesture::GetDuration().
	UPROPERTY()
	float GestureDurationSeconds = 0.f;

	bool Encode(
		const FMagicGesture& SourceGesture,
		const UMagicValidationSettings& Settings
	);

	bool Decode(
		FMagicGesture& OutGesture,
		const UMagicValidationSettings& Settings
	) const;

	void Reset();
};