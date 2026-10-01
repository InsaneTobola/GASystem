#include "Magic/MagicGestureValidator.h"
#include "Magic/MagicValidationSettings.h"


FMagicValidationResult FMagicGestureValidator::Validate(const FMagicGesture& Gesture,const UMagicValidationSettings& Settings)
{
    FMagicValidationResult Result;
    Result.StrokeCount =Gesture.GetStrokeCount();
    Result.TotalPointCount = Gesture.GetTotalPointCount();
    Result.GestureDuration = Gesture.GetDuration();
    EMagicValidationFailure Failure =EMagicValidationFailure::None;
    // Stroke Count
    if (!ValidateStrokeCount(Gesture,Settings,Failure))
    {
        Result.FailureReason = Failure;
        return Result;
    }
    // Point Count
    if (!ValidatePointCount(Gesture,Settings,Failure))
    {
        Result.FailureReason = Failure;
        return Result;
    }
    // Path Length
    Result.TotalPathLength =CalculatePathLength(Gesture);
    if (!ValidatePathLength(Gesture,Result.TotalPathLength,Settings,Failure))
    {
        Result.FailureReason = Failure;
        return Result;
    }

    // Bounding Box
    CalculateBoundingBox(Gesture,Result.BoundingBoxMin,Result.BoundingBoxMax);
    Result.BoundingBoxSize = Result.BoundingBoxMax - Result.BoundingBoxMin;
    if (!ValidateBoundingBox(Result.BoundingBoxSize,Settings,Failure))
    {
        Result.FailureReason = Failure;
        return Result;
    }


    // Aspect Ratio
    Result.AspectRatio =CalculateAspectRatio(Result.BoundingBoxSize);

    if (!ValidateAspectRatio(Result.AspectRatio,Settings,Failure))
    {
        Result.FailureReason = Failure;
        return Result;
    }
    Result.bIsValid = true;
    Result.FailureReason =EMagicValidationFailure::None;

    return Result;
}


bool FMagicGestureValidator::ValidateStrokeCount(const FMagicGesture& Gesture,   const UMagicValidationSettings& Settings,    EMagicValidationFailure& OutFailure)
{
    if (Gesture.Strokes.Num() == 0)
    {
        OutFailure =EMagicValidationFailure::NoStrokes;
        return false;
    }

    if (Gesture.Strokes.Num() > Settings.MaxStrokeCount)
    {
        OutFailure =EMagicValidationFailure::TooManyStrokes;
        return false;
    }
    return true;
}


bool FMagicGestureValidator::ValidatePointCount(const FMagicGesture& Gesture,const UMagicValidationSettings& Settings,EMagicValidationFailure& OutFailure)
{
    const int32 TotalPointCount = Gesture.GetTotalPointCount();

    if (TotalPointCount <Settings.MinTotalPointCount)
    {
        OutFailure =EMagicValidationFailure::TooFewPoints;
        return false;
    }

    if (TotalPointCount > Settings.MaxTotalPointCount)
    {
        OutFailure = EMagicValidationFailure::TooManyPoints;
        return false;
    }

    for (const FMagicStroke& Stroke : Gesture.Strokes)
    {
        const int32 PointCount = Stroke.Points.Num();

        if (PointCount < Settings.MinPointsPerStroke)
        {
            OutFailure = EMagicValidationFailure::TooFewPoints;
            return false;
        }

        if (PointCount > Settings.MaxPointsPerStroke)
        {
            OutFailure = EMagicValidationFailure::TooManyPoints;
            return false;
        }
    }

    return true;
}


float FMagicGestureValidator::CalculateStrokePathLength(const FMagicStroke& Stroke)
{
    float TotalLength = 0.0f;

    for (int32 Index = 1;Index < Stroke.Points.Num();++Index)
    {
        TotalLength +=FVector2D::Distance(Stroke.Points[Index - 1],Stroke.Points[Index]);
    }
    return TotalLength;
}


float FMagicGestureValidator::CalculatePathLength(const FMagicGesture& Gesture)
{
    float TotalLength = 0.0f;

    for (const FMagicStroke& Stroke : Gesture.Strokes)
    {
        TotalLength += CalculateStrokePathLength(Stroke);
    }
    return TotalLength;
}


bool FMagicGestureValidator::ValidatePathLength(const FMagicGesture& Gesture,const float TotalPathLength,const UMagicValidationSettings& Settings,EMagicValidationFailure& OutFailure)
{
    for (const FMagicStroke& Stroke : Gesture.Strokes)
    {
        const float StrokePathLength = CalculateStrokePathLength(Stroke);

        if (StrokePathLength > Settings.MaxPathLengthPerStroke)
        {
            OutFailure = EMagicValidationFailure::StrokePathTooLong;
            return false;
        }
    }

    if (TotalPathLength <Settings.MinPathLength)
    {
        OutFailure = EMagicValidationFailure::PathTooShort;
        return false;
    }

    if (TotalPathLength > Settings.MaxPathLength)
    {
        OutFailure = EMagicValidationFailure::PathTooLong;
        return false;
    }

    return true;
}


void FMagicGestureValidator::CalculateBoundingBox(const FMagicGesture& Gesture,FVector2D& OutMin,FVector2D& OutMax)
{
    OutMin = FVector2D(TNumericLimits<float>::Max(),TNumericLimits<float>::Max());
    OutMax = FVector2D(-TNumericLimits<float>::Max(),-TNumericLimits<float>::Max());

    for (const FMagicStroke& Stroke : Gesture.Strokes)
    {
        for (const FVector2D& Point :Stroke.Points)
        {
            OutMin.X =FMath::Min(OutMin.X,Point.X);
            OutMin.Y =FMath::Min(OutMin.Y,Point.Y);
            OutMax.X =FMath::Max(OutMax.X,Point.X);
            OutMax.Y =FMath::Max(OutMax.Y,Point.Y);
        }
    }
}


bool FMagicGestureValidator::ValidateBoundingBox(const FVector2D& BoundingBoxSize,const UMagicValidationSettings& Settings,EMagicValidationFailure& OutFailure)
{
    const float MaxDimension =FMath::Max(BoundingBoxSize.X,BoundingBoxSize.Y);
    if (MaxDimension <Settings.MinBoundingBoxDimension)
    {
        OutFailure =EMagicValidationFailure::BoundingBoxTooSmall;
        return false;
    }

    if (BoundingBoxSize.X >Settings.MaxBoundingBoxWidth ||BoundingBoxSize.Y >Settings.MaxBoundingBoxHeight)
    {
        OutFailure = EMagicValidationFailure::BoundingBoxTooLarge;
        return false;
    }

    return true;
}


float FMagicGestureValidator::CalculateAspectRatio(const FVector2D& BoundingBoxSize)
{
    if (BoundingBoxSize.Y <= KINDA_SMALL_NUMBER)
    {
        return TNumericLimits<float>::Max();
    }

    return BoundingBoxSize.X /BoundingBoxSize.Y;
}


bool FMagicGestureValidator::ValidateAspectRatio(const float AspectRatio,const UMagicValidationSettings& Settings,EMagicValidationFailure& OutFailure)
{
    if (AspectRatio < Settings.MinAspectRatio)
    {
        OutFailure = EMagicValidationFailure::AspectRatioTooSmall;
        return false;
    }

    if (AspectRatio >Settings.MaxAspectRatio)
    {
        OutFailure = EMagicValidationFailure::AspectRatioTooLarge;
        return false;
    }

    return true;
}