#pragma once

#include "CoreMinimal.h"

struct AURA_API FVolcanicArcsGeometry
{
    static constexpr int32 SamplesPerArc = 24;

    static FVector EvaluateBezier(
        const FVector& P0,
        const FVector& P1,
        const FVector& P2,
        const FVector& P3,
        float T)
    {
        const float U = 1.f - T;

        return
            U * U * U * P0 +
            3.f * U * U * T * P1 +
            3.f * U * T * T * P2 +
            T * T * T * P3;
    }

    static void AppendBezierSegment(
        const FVector& P0,
        const FVector& P1,
        const FVector& P2,
        const FVector& P3,
        TArray<FVector>& OutPoints,
        bool bSkipFirstSample = false)
    {
        const int32 FirstSample = bSkipFirstSample ? 1 : 0;

        for (int32 Index = FirstSample;
             Index <= SamplesPerArc;
             ++Index)
        {
            const float T =
                static_cast<float>(Index) /
                static_cast<float>(SamplesPerArc);

            OutPoints.Add(
                EvaluateBezier(P0, P1, P2, P3, T)
            );
        }
    }

    static void Build(
        const FVector& IntersectionLocal,
        float BaseHalfLengthCm,
        float ArcLengthScale,
        float ArcBowOffsetCm,
        TArray<FVector>& OutArcA,
        TArray<FVector>& OutArcB)
    {
        OutArcA.Reset();
        OutArcB.Reset();

        const float Distance =
            IntersectionLocal.Size2D();

        if (Distance <= KINDA_SMALL_NUMBER)
        {
            return;
        }

        const FVector Direction =
            IntersectionLocal.GetSafeNormal2D();

        const FVector Side(
            -Direction.Y,
             Direction.X,
             0.f
        );

        const float Bow =
            FMath::Max(ArcBowOffsetCm, 0.f);

        const float SafeScale =
            FMath::Clamp(ArcLengthScale, 0.1f, 3.f);

        const float ExtraLength =
            FMath::Max(BaseHalfLengthCm, 1.f) *
            SafeScale;

        // Tangent length controls the shape around the intersection.
        // It does not change the width of the mesh.
        const float ForwardTangentLength =
            FMath::Min(Distance * 0.20f, 120.f);

        const float SideTangentLength =
            FMath::Min(Bow * 0.35f, Distance * 0.15f);

        const FVector Origin = FVector::ZeroVector;

        const FVector Intersection =
            IntersectionLocal;

        auto BuildPath =
            [&](float SideSign, TArray<FVector>& OutPath)
        {
            const FVector Tangent =
                Direction * ForwardTangentLength +
                Side * (SideTangentLength * SideSign);

            // First curve:
            // Character -> cursor intersection.
            const FVector P0 = Origin;

            const FVector P1 =
                Intersection * 0.30f +
                Side * (Bow * SideSign);

            const FVector P2 =
                Intersection - Tangent;

            const FVector P3 =
                Intersection;

            AppendBezierSegment(
                P0, P1, P2, P3,
                OutPath
            );

            // Second curve:
            // Cursor intersection -> extended arc end.
            const FVector Q0 = Intersection;

            const FVector Q1 = Intersection + Tangent;

            const FVector Q2 =
                Intersection +
                Direction * (ExtraLength * 0.70f) +
                Side * (Bow * SideSign);

            const FVector Q3 =
                Intersection +
                Direction * ExtraLength +
                Side * (Bow * 0.50f * SideSign);

            // Skip first point because Q0 is the previous segment's end.
            AppendBezierSegment(
                Q0, Q1, Q2, Q3,
                OutPath,
                true
            );
        };

        // Two arcs cross at the cursor intersection.
        // Their lateral directions are mirrored.
        BuildPath(1.f, OutArcA);
        BuildPath(-1.f, OutArcB);
    }

    static float CalculateLengthScale(float DistanceCm,float MaxRangeCm,float MinScale,float MaxScale);
    
};