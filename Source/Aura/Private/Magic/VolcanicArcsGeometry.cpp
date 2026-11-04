#include "Magic/VolcanicArcsGeometry.h"


float FVolcanicArcsGeometry::CalculateLengthScale(float DistanceCm,float MaxRangeCm,float MinScale,float MaxScale)
{
	const float SafeMaxRange = FMath::Max(MaxRangeCm, 1.f);

	const float Alpha = FMath::Clamp(DistanceCm / SafeMaxRange,0.f,1.f);

	return FMath::Lerp(MinScale,MaxScale,Alpha);
}
