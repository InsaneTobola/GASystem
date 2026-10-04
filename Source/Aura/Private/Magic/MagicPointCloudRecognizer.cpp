#include "Magic/MagicPointCloudRecognizer.h"

#include "Math/UnrealMathUtility.h"


bool FMagicPointCloudRecognizer::BuildPointCloud(const TArray<FVector2D>& RawPoints,FMagicPointCloud& OutPointCloud)
{
	OutPointCloud.Reset();

	if (RawPoints.Num() < 2)
	{
		return false;
	}

	TArray<FMagicCloudInputPoint> InputPoints;
	InputPoints.Reserve(RawPoints.Num());

	for (const FVector2D& RawPoint : RawPoints)
	{
		if (!FMath::IsFinite(RawPoint.X) || !FMath::IsFinite(RawPoint.Y))
		{
			continue;
		}

		FMagicCloudInputPoint Point;
		Point.Position = RawPoint;
		Point.StrokeId = 0;

		InputPoints.Add(Point);
	}

	if (InputPoints.Num() < 2)
	{
		return false;
	}
	TArray<FMagicCloudInputPoint> ResampledPoints;

	if (!Resample(InputPoints,ResamplePointCount,ResampledPoints))
	{
		return false;
	}

	if (!Scale(ResampledPoints))
	{
		return false;
	}

	if (!TranslateToOrigin(ResampledPoints))
	{
		return false;
	}
	OutPointCloud.Points.Reserve(ResampledPoints.Num());
	for (const FMagicCloudInputPoint& Point : ResampledPoints)
	{
		OutPointCloud.Points.Add(Point.Position);
	}
	return OutPointCloud.IsValid(ResamplePointCount);
}


bool FMagicPointCloudRecognizer::BuildPointCloud(const FMagicStroke& Stroke,FMagicPointCloud& OutPointCloud)
{
	return BuildPointCloud(Stroke.Points,OutPointCloud);
}


bool FMagicPointCloudRecognizer::FlattenGesture(const FMagicGesture& Gesture,TArray<FMagicCloudInputPoint>& OutPoints)
{
	OutPoints.Reset();

	for (int32 StrokeIndex = 0;StrokeIndex < Gesture.Strokes.Num();++StrokeIndex)
	{
		const FMagicStroke& Stroke = Gesture.Strokes[StrokeIndex];
		for (const FVector2D& RawPoint : Stroke.Points)
		{
			if (!FMath::IsFinite(RawPoint.X) || !FMath::IsFinite(RawPoint.Y))
			{
				continue;
			}

			FMagicCloudInputPoint Point;

			Point.Position = RawPoint;
			Point.StrokeId = StrokeIndex;

			OutPoints.Add(Point);
		}
	}
	return OutPoints.Num() >= 2;
}

bool FMagicPointCloudRecognizer::BuildPointCloud(const FMagicGesture& Gesture,FMagicPointCloud& OutPointCloud)
{
	OutPointCloud.Reset();
	TArray<FMagicCloudInputPoint> InputPoints;

	if (!FlattenGesture(Gesture,InputPoints))
	{
		return false;
	}

	TArray<FMagicCloudInputPoint> ResampledPoints;

	if (!Resample(InputPoints,ResamplePointCount,ResampledPoints))
	{
		return false;
	}

	if (!Scale(ResampledPoints))
	{
		return false;
	}

	if (!TranslateToOrigin(ResampledPoints))
	{
		return false;
	}
	OutPointCloud.Points.Reserve( ResampledPoints.Num());
	for (const FMagicCloudInputPoint& Point : ResampledPoints)
	{
		OutPointCloud.Points.Add(Point.Position);
	}
	return OutPointCloud.IsValid(ResamplePointCount);
}

bool FMagicPointCloudRecognizer::BuildTemplate(FName TemplateId,const FMagicGesture& Gesture,FMagicPointCloudTemplate& OutTemplate)
{
	OutTemplate = FMagicPointCloudTemplate();
	OutTemplate.TemplateId =TemplateId;

	return BuildPointCloud(Gesture,OutTemplate.PointCloud);
}

float FMagicPointCloudRecognizer::PathLength(const TArray<FMagicCloudInputPoint>& Points)
{
	float Length = 0.0f;

	for (int32 Index = 1;Index < Points.Num();++Index)
	{
		const FMagicCloudInputPoint& Previous = Points[Index - 1];
		const FMagicCloudInputPoint& Current = Points[Index];
		if (Previous.StrokeId != Current.StrokeId)
		{
			continue;
		}

		Length += FVector2D::Distance(Previous.Position,Current.Position);
	}
	return Length;
}


bool FMagicPointCloudRecognizer::Resample(const TArray<FMagicCloudInputPoint>& InputPoints,int32 TargetPointCount,TArray<FMagicCloudInputPoint>& OutPoints)
{
	OutPoints.Reset();
	if (TargetPointCount < 2)
	{
		return false;
	}

	if (InputPoints.Num() < 2)
	{
		return false;
	}

	const float TotalPathLength = PathLength(InputPoints);

	if (TotalPathLength <= KINDA_SMALL_NUMBER)
	{
		return false;
	}

	const float Interval = TotalPathLength /static_cast<float>(TargetPointCount - 1);
	float DistanceSinceLastSample = 0.0f;
	TArray<FMagicCloudInputPoint> WorkingPoints = InputPoints;
	OutPoints.Add(WorkingPoints[0]);

	for (int32 Index = 1;Index < WorkingPoints.Num();++Index)
	{
		if (OutPoints.Num() >= TargetPointCount)
		{
			break;
		}

		const FMagicCloudInputPoint& Previous = WorkingPoints[Index - 1];
		const FMagicCloudInputPoint& Current = WorkingPoints[Index];

		if (Previous.StrokeId != Current.StrokeId)
		{
			continue;
		}

		const float SegmentDistance = FVector2D::Distance(Previous.Position,Current.Position);

		if (SegmentDistance <= KINDA_SMALL_NUMBER)
		{
			continue;
		}

		if (DistanceSinceLastSample + SegmentDistance >= Interval)
		{
			const float Alpha = (Interval - DistanceSinceLastSample) /SegmentDistance;
			FMagicCloudInputPoint NewPoint;
			NewPoint.Position = Previous.Position +Alpha *(Current.Position - Previous.Position);
			NewPoint.StrokeId = Current.StrokeId;
			OutPoints.Add(NewPoint);
			WorkingPoints.Insert(NewPoint, Index);
			DistanceSinceLastSample = 0.0f;
			continue;
		}

		DistanceSinceLastSample += SegmentDistance;
	}

	if (OutPoints.Num() < TargetPointCount)
	{
		const FMagicCloudInputPoint& LastPoint = WorkingPoints.Last();

		if (OutPoints.Num() == TargetPointCount - 1)
		{
			OutPoints.Add(LastPoint);
		}
	}
	return OutPoints.Num() == TargetPointCount;
}


bool FMagicPointCloudRecognizer::Scale(TArray<FMagicCloudInputPoint>& Points)
{
	if (Points.Num() == 0)
	{
		return false;
	}

	float MinX = TNumericLimits<float>::Max();
	float MinY = TNumericLimits<float>::Max();

	float MaxX = -TNumericLimits<float>::Max();
	float MaxY = -TNumericLimits<float>::Max();

	for (const FMagicCloudInputPoint& Point : Points)
	{
		MinX = FMath::Min(MinX,Point.Position.X);
		MinY = FMath::Min(MinY,Point.Position.Y);
		MaxX = FMath::Max(MaxX,Point.Position.X);
		MaxY = FMath::Max(MaxY,Point.Position.Y);}

	const float Width = MaxX - MinX;
	const float Height = MaxY - MinY;
	const float ScaleValue = FMath::Max(Width,Height);

	if (ScaleValue <= KINDA_SMALL_NUMBER)
	{
		return false;
	}

	for (FMagicCloudInputPoint& Point : Points)
	{
		Point.Position.X = (Point.Position.X - MinX) /ScaleValue;
		Point.Position.Y = (Point.Position.Y - MinY) /ScaleValue;
	}
	return true;
}


bool FMagicPointCloudRecognizer::TranslateToOrigin(TArray<FMagicCloudInputPoint>& Points)
{
	if (Points.Num() == 0)
	{
		return false;
	}

	FVector2D Centroid = FVector2D::ZeroVector;

	for (const FMagicCloudInputPoint& Point : Points)
	{
		Centroid += Point.Position;
	}

	Centroid /= static_cast<float>(Points.Num());

	for (FMagicCloudInputPoint& Point : Points)
	{
		Point.Position -= Centroid;
	}

	return true;
}


float FMagicPointCloudRecognizer::CloudDistance(const TArray<FVector2D>& Points,const TArray<FVector2D>& Template,int32 Start)
{
	const int32 PointCount = Points.Num();

	if (PointCount == 0 ||
		Template.Num() != PointCount)
	{
		return TNumericLimits<float>::Max();
	}

	TArray<uint8> Matched;
	Matched.SetNumZeroed(PointCount);

	float Sum = 0.0f;

	int32 Index = Start;

	do
	{
		float MinimumDistance = TNumericLimits<float>::Max();

		int32 BestTemplateIndex = INDEX_NONE;

		for (int32 TemplateIndex = 0;TemplateIndex < PointCount;++TemplateIndex)
		{
			if (Matched[TemplateIndex] != 0)
			{
				continue;
			}
			const float Distance = FVector2D::Distance(Points[Index],Template[TemplateIndex]);
			if (Distance < MinimumDistance)
			{
				MinimumDistance = Distance;
				BestTemplateIndex = TemplateIndex;
			}
		}
		if (BestTemplateIndex == INDEX_NONE)
		{
			return TNumericLimits<float>::Max();
		}

		Matched[BestTemplateIndex] = 1;

		const int32 RelativeIndex = (Index - Start + PointCount) % PointCount;
		const float Weight = 1.0f - (static_cast<float>(RelativeIndex) / static_cast<float>(PointCount));
		Sum += Weight * MinimumDistance;
		Index = (Index + 1) % PointCount;
	}
	while (Index != Start);

	return Sum;
}


float FMagicPointCloudRecognizer::GreedyCloudMatch(const TArray<FVector2D>& Points,const TArray<FVector2D>& Template)
{
	const int32 PointCount = Points.Num();

	if (PointCount == 0 || Template.Num() != PointCount)
	{
		return TNumericLimits<float>::Max();
	}

	const int32 Step = FMath::Max(1,FMath::FloorToInt(FMath::Pow(static_cast<float>(PointCount),1.0f - GreedyMatchEpsilon)));
	float MinimumDistance = TNumericLimits<float>::Max();

	for (int32 Start = 0;Start < PointCount;Start += Step)
	{
		const float Distance1 = CloudDistance(Points,Template,Start);
		const float Distance2 = CloudDistance(Template,Points,Start);
		MinimumDistance = FMath::Min(MinimumDistance,FMath::Min(Distance1,Distance2));
	}
	return MinimumDistance;
}


float FMagicPointCloudRecognizer::DistanceToSimilarity(float Distance)
{
	if (!FMath::IsFinite(Distance) || Distance < 0.0f)
	{
		return 0.0f;
	}

	return Distance > 1.0f ? 1.0f / Distance : 1.0f;
}


FMagicRecognitionResult FMagicPointCloudRecognizer::Recognize(const FMagicPointCloud& Candidate,const TArray<FMagicPointCloudTemplate>& Templates,float MinimumSimilarityScore)
{
	FMagicRecognitionResult Result;

	if (!Candidate.IsValid(ResamplePointCount))
	{
		return Result;
	}

	if (Templates.Num() == 0)
	{
		return Result;
	}

	float BestDistance = TNumericLimits<float>::Max();
	FName BestPatternId = NAME_None;
	FName BestTemplateId = NAME_None;
	for (const FMagicPointCloudTemplate& Template : Templates)
	{
		if (!Template.PointCloud.IsValid(ResamplePointCount))
		{
			continue;
		}

		const float Distance = GreedyCloudMatch(Candidate.Points,Template.PointCloud.Points);

		if (Distance < BestDistance)
		{
			BestDistance = Distance;
			BestPatternId = Template.PatternId;
			BestTemplateId = Template.TemplateId;
		}
	}
	if (!FMath::IsFinite(BestDistance) || BestTemplateId.IsNone())
	{
		return Result;
	}
	const float Similarity = DistanceToSimilarity(BestDistance);

	Result.CloudDistance = BestDistance;
	Result.PatternId = BestPatternId;
	Result.TemplateId = BestTemplateId;
	Result.SimilarityScore = Similarity;
	Result.bMatchFound = Similarity >= MinimumSimilarityScore;

	return Result;
}
FMagicRecognitionResult FMagicPointCloudRecognizer::Recognize(const FMagicGesture& Gesture,const TArray<FMagicPointCloudTemplate>& Templates,float MinimumSimilarityScore)
{
	FMagicPointCloud Candidate;

	if (!BuildPointCloud(Gesture,Candidate))
	{
		return FMagicRecognitionResult();
	}

	return Recognize(Candidate,Templates,MinimumSimilarityScore);
}