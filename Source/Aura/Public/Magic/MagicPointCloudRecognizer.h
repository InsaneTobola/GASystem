#pragma once

#include "CoreMinimal.h"
#include "Magic/MagicTypes.h"
#include "Math/NumericLimits.h"

struct AURA_API FMagicCloudInputPoint
{
	FVector2D Position = FVector2D::ZeroVector;
	int32 StrokeId = INDEX_NONE;
};

struct AURA_API FMagicPointCloud
{
	TArray<FVector2D> Points;

	bool IsValid(int32 ExpectedPointCount) const
	{
		return Points.Num() == ExpectedPointCount;
	}

	void Reset()
	{
		Points.Reset();
	}
};

struct AURA_API FMagicPointCloudTemplate
{
	FName PatternId = NAME_None;
	FName TemplateId = NAME_None;
	FMagicPointCloud PointCloud;
};

struct AURA_API FMagicRecognitionResult
{
	bool bMatchFound = false;
	FName PatternId = NAME_None;
	FName TemplateId = NAME_None;
	float SimilarityScore = 0.0f;
	float CloudDistance = TNumericLimits<float>::Max();
};

class AURA_API FMagicPointCloudRecognizer
{
public:

	static constexpr int32 ResamplePointCount = 32;
	static constexpr float GreedyMatchEpsilon = 0.50f;

	// Point Cloud creation
	static bool BuildPointCloud(const TArray<FVector2D>& RawPoints,FMagicPointCloud& OutPointCloud);
	static bool BuildPointCloud(const FMagicStroke& Stroke,FMagicPointCloud& OutPointCloud);
	static bool BuildPointCloud(const FMagicGesture& Gesture,FMagicPointCloud& OutPointCloud);

	// Template creation
	static bool BuildTemplate(FName TemplateId, const FMagicGesture& Gesture, FMagicPointCloudTemplate& OutTemplate);

	// Recognition
	static FMagicRecognitionResult Recognize(const FMagicPointCloud& Candidate,const TArray<FMagicPointCloudTemplate>& Templates,float MinimumSimilarityScore = 0.30f);
	static FMagicRecognitionResult Recognize(const FMagicGesture& Gesture,const TArray<FMagicPointCloudTemplate>& Templates,float MinimumSimilarityScore = 0.30f);

private:

	static bool FlattenGesture(const FMagicGesture& Gesture,TArray<FMagicCloudInputPoint>& OutPoints);
	static float PathLength(const TArray<FMagicCloudInputPoint>& Points);
	static bool Resample(const TArray<FMagicCloudInputPoint>& InputPoints,int32 TargetPointCount,TArray<FMagicCloudInputPoint>& OutPoints);
	static bool Scale(TArray<FMagicCloudInputPoint>& Points);
	static bool TranslateToOrigin(TArray<FMagicCloudInputPoint>& Points);
	static float CloudDistance(const TArray<FVector2D>& Points,const TArray<FVector2D>& Template,int32 Start);
	static float GreedyCloudMatch(const TArray<FVector2D>& Points,const TArray<FVector2D>& Template);
	static float DistanceToSimilarity(float Distance);
};