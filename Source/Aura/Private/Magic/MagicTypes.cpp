#include "Magic/MagicTypes.h"

void FMagicStroke::Reset()
{
	Points.Reset();
	StartTime = 0.0f;
	EndTime = 0.0f;
}

float FMagicStroke::GetDuration() const
{
	return FMath::Max(0.0f, EndTime - StartTime);
}

void FMagicGesture::Reset()
{
	Strokes.Reset();
	StartTime = 0.0f;
	EndTime = 0.0f;
}

int32 FMagicGesture::GetStrokeCount() const
{
	return Strokes.Num();
}

int32 FMagicGesture::GetTotalPointCount() const
{
	int32 TotalPoints = 0;

	for (const FMagicStroke& Stroke : Strokes)
	{
		TotalPoints += Stroke.Points.Num();
	}

	return TotalPoints;
}

float FMagicGesture::GetDuration() const
{
	return FMath::Max(0.0f, EndTime - StartTime);
}