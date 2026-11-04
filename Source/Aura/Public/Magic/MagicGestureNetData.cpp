#include "Magic/MagicGestureNetData.h"

#include "Magic/MagicValidationSettings.h"

void FMagicGestureNetData::Reset()
{
    QuantizedCoordinates.Reset();
    StrokePointCounts.Reset();

    StrokeStartOffsets.Reset();
    StrokeEndOffsets.Reset();

    GestureDurationSeconds = 0.f;
}

bool FMagicGestureNetData::Encode(
    const FMagicGesture& SourceGesture,
    const UMagicValidationSettings& Settings)
{
    Reset();

    const int32 StrokeCount = SourceGesture.GetStrokeCount();
    const int32 TotalPointCount = SourceGesture.GetTotalPointCount();

    // Structural limits.
    if (StrokeCount <= 0 ||
        StrokeCount > Settings.MaxStrokeCount)
    {
        return false;
    }

    if (TotalPointCount < Settings.MinTotalPointCount ||
        TotalPointCount > Settings.MaxTotalPointCount)
    {
        return false;
    }

    const float Duration = SourceGesture.GetDuration();

    if (!FMath::IsFinite(Duration) || Duration < 0.f)
    {
        return false;
    }

    GestureDurationSeconds = Duration;

    StrokePointCounts.Reserve(StrokeCount);
    StrokeStartOffsets.Reserve(StrokeCount);
    StrokeEndOffsets.Reserve(StrokeCount);

    QuantizedCoordinates.Reserve(TotalPointCount * 2);

    for (const FMagicStroke& Stroke : SourceGesture.Strokes)
    {
        const int32 PointCount = Stroke.Points.Num();

        if (PointCount < Settings.MinPointsPerStroke ||
            PointCount > Settings.MaxPointsPerStroke ||
            PointCount > TNumericLimits<uint16>::Max())
        {
            Reset();
            return false;
        }

        const float StrokeStartOffset = Stroke.StartTime - SourceGesture.StartTime;
        const float StrokeEndOffset = Stroke.EndTime - SourceGesture.StartTime;

        if (!FMath::IsFinite(StrokeStartOffset) || !FMath::IsFinite(StrokeEndOffset))
        {
            Reset();
            return false;
        }

        StrokePointCounts.Add(static_cast<uint16>(PointCount));

        StrokeStartOffsets.Add(StrokeStartOffset);
        StrokeEndOffsets.Add(StrokeEndOffset);

        for (const FVector2D& Point : Stroke.Points)
        {
            if (!FMath::IsFinite(Point.X) ||
                !FMath::IsFinite(Point.Y) ||
                Point.X < 0.f || Point.X > 1.f ||
                Point.Y < 0.f || Point.Y > 1.f)
            {
                Reset();
                return false;
            }

            const uint16 X = static_cast<uint16>(FMath::RoundToInt(Point.X * 65535.f));
            const uint16 Y = static_cast<uint16>(FMath::RoundToInt(Point.Y * 65535.f));

            QuantizedCoordinates.Add(X);
            QuantizedCoordinates.Add(Y);
        }
    }

    return true;
}

bool FMagicGestureNetData::Decode(FMagicGesture& OutGesture,const UMagicValidationSettings& Settings) const
{
    OutGesture.Reset();

    const int32 StrokeCount = StrokePointCounts.Num();

    if (StrokeCount <= 0 || StrokeCount > Settings.MaxStrokeCount)
    {
        return false;
    }

    if (StrokeStartOffsets.Num() != StrokeCount || StrokeEndOffsets.Num() != StrokeCount)
    {
        return false;
    }

    if (!FMath::IsFinite(GestureDurationSeconds) || GestureDurationSeconds < 0.f)
    {
        return false;
    }

    int32 TotalPointCount = 0;

    for (const uint16 PointCount : StrokePointCounts)
    {
        if (PointCount < Settings.MinPointsPerStroke || PointCount > Settings.MaxPointsPerStroke)
        {
            return false;
        }

        TotalPointCount += PointCount;
    }

    if (TotalPointCount < Settings.MinTotalPointCount || TotalPointCount > Settings.MaxTotalPointCount)
    {
        return false;
    }

    if (QuantizedCoordinates.Num() != TotalPointCount * 2)
    {
        return false;
    }

    FMagicGesture DecodedGesture;

    // Reconstruct gesture duration.
    DecodedGesture.StartTime = 0.f;
    DecodedGesture.EndTime = GestureDurationSeconds;

    int32 CoordinateIndex = 0;

    for (int32 StrokeIndex = 0;StrokeIndex < StrokeCount;++StrokeIndex)
    {
        const float StartOffset = StrokeStartOffsets[StrokeIndex];
        const float EndOffset = StrokeEndOffsets[StrokeIndex];

        if (!FMath::IsFinite(StartOffset) || !FMath::IsFinite(EndOffset))
        {
            return false;
        }

        FMagicStroke Stroke;

        Stroke.StartTime = StartOffset;
        Stroke.EndTime = EndOffset;

        const int32 PointCount = StrokePointCounts[StrokeIndex];

        Stroke.Points.Reserve(PointCount);

        for (int32 PointIndex = 0;PointIndex < PointCount;++PointIndex)
        {
            const uint16 PackedX = QuantizedCoordinates[CoordinateIndex++];
            const uint16 PackedY = QuantizedCoordinates[CoordinateIndex++];
            const float X = static_cast<float>(PackedX) / 65535.f;
            const float Y = static_cast<float>(PackedY) / 65535.f;

            Stroke.Points.Add(FVector2D(X, Y));
        }

        DecodedGesture.Strokes.Add(MoveTemp(Stroke));
    }

    OutGesture = MoveTemp(DecodedGesture);

    return true;
}