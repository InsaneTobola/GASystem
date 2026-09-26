#include "Magic/MagicGestureValidator.h"
#include "Magic/MagicValidationSettings.h"

FMagicValidationResult FMagicGestureValidator::Validate(
    const FMagicGesture& Gesture,
    const UMagicValidationSettings& Settings)
{
    FMagicValidationResult Result;

    Result.StrokeCount = Gesture.GetStrokeCount();
    Result.TotalPointCount = Gesture.GetTotalPointCount();
    Result.GestureDuration = Gesture.GetDuration();

    if (Gesture.Strokes.Num() == 0)
    {
        Result.FailureReason = EMagicValidationFailure::NoStrokes;
        return Result;
    }
    if (Gesture.Strokes.Num() > Settings.MaxStrokeCount)
    {
        Result.FailureReason = EMagicValidationFailure::TooManyStrokes;
        return Result;
    }
    if (Result.TotalPointCount < Settings.MinTotalPointCount)
    {
        Result.FailureReason = EMagicValidationFailure::TooFewPoints;
        return Result;
    }
    if (Result.TotalPointCount > Settings.MaxTotalPointCount)
    {
        Result.FailureReason = EMagicValidationFailure::TooManyPoints;
        return Result;
    }
    for (const FMagicStroke& Stroke : Gesture.Strokes)
    {
        if (Stroke.Points.Num() < 2)
        {
            Result.FailureReason = EMagicValidationFailure::TooFewPoints;
            return Result;
        }

        if (Stroke.Points.Num() > Settings.MaxPointsPerStroke)
        {
            Result.FailureReason = EMagicValidationFailure::TooManyPoints;
            return Result;
        }
    }

    Result.TotalPathLength = CalculatePathLength(Gesture);

    if (Result.TotalPathLength < Settings.MinPathLength)
    {
        Result.FailureReason = EMagicValidationFailure::PathTooShort;
        return Result;
    }

    if (Result.TotalPathLength > Settings.MaxPathLength)
    {
        Result.FailureReason = EMagicValidationFailure::PathTooLong;
        return Result;
    }

    CalculateBoundingBox(Gesture,Result.BoundingBoxMin,Result.BoundingBoxMax);

    Result.BoundingBoxSize =Result.BoundingBoxMax - Result.BoundingBoxMin;

    const float MaxDimension = FMath::Max(Result.BoundingBoxSize.X,Result.BoundingBoxSize.Y);

    if (MaxDimension < Settings.MinBoundingBoxDimension)
    {
        Result.FailureReason = EMagicValidationFailure::BoundingBoxTooSmall;
        return Result;
    }

    if (Result.BoundingBoxSize.X > Settings.MaxBoundingBoxWidth || Result.BoundingBoxSize.Y > Settings.MaxBoundingBoxHeight)
    {
        Result.FailureReason = EMagicValidationFailure::BoundingBoxTooLarge;
        return Result;
    }

    Result.AspectRatio =CalculateAspectRatio(Result.BoundingBoxSize);

    if (Result.AspectRatio < Settings.MinAspectRatio)
    {
        Result.FailureReason = EMagicValidationFailure::AspectRatioTooSmall;
        return Result;
    }

    if (Result.AspectRatio > Settings.MaxAspectRatio)
    {
        Result.FailureReason = EMagicValidationFailure::AspectRatioTooLarge;
        return Result;
    }

    Result.bIsValid = true;
    Result.FailureReason = EMagicValidationFailure::None;
    return Result;
}

float FMagicGestureValidator::CalculatePathLength(const FMagicGesture& Gesture)
{
    float TotalLength = 0.0f;
    for (const FMagicStroke& Stroke : Gesture.Strokes)
    {
        for (int32 Index = 1; Index < Stroke.Points.Num(); ++Index)
        {
            TotalLength += FVector2D::Distance(Stroke.Points[Index - 1],Stroke.Points[Index]
            );
        }
    }

    return TotalLength;
}

void FMagicGestureValidator::CalculateBoundingBox(const FMagicGesture& Gesture,FVector2D& OutMin,FVector2D& OutMax)
{
    OutMin = FVector2D(TNumericLimits<float>::Max(),TNumericLimits<float>::Max());
    OutMax = FVector2D(-TNumericLimits<float>::Max(),-TNumericLimits<float>::Max());

    for (const FMagicStroke& Stroke : Gesture.Strokes)
    {
        for (const FVector2D& Point : Stroke.Points)
        {
            OutMin.X = FMath::Min(OutMin.X, Point.X);
            OutMin.Y = FMath::Min(OutMin.Y, Point.Y);

            OutMax.X = FMath::Max(OutMax.X, Point.X);
            OutMax.Y = FMath::Max(OutMax.Y, Point.Y);
        }
    }
}

float FMagicGestureValidator::CalculateAspectRatio(const FVector2D& BoundingBoxSize)
{
    if (BoundingBoxSize.Y <= KINDA_SMALL_NUMBER)
    {
        return TNumericLimits<float>::Max();
    }

    return BoundingBoxSize.X / BoundingBoxSize.Y;
}