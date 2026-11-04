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

    static void Build(float HalfLengthCm,float LengthScale,TArray<FVector>& OutArcA,TArray<FVector>& OutArcB)
    {
        OutArcA.Reset();
        OutArcB.Reset();

        const float L = FMath::Max(1.f,HalfLengthCm * LengthScale);

        // Arc A: first diagonal curve.
        const FVector E1(L, L * 0.45f, 0.f);
        const FVector C1(L * 0.45f, -L * 0.20f, 0.f);

        // Arc B: mirrored diagonal curve.
        const FVector E2(L, -L * 0.45f, 0.f);
        const FVector C2(L * 0.45f, L * 0.20f, 0.f);

        for (int32 Index = 0; Index <= SamplesPerArc; ++Index)
        {
            const float T = static_cast<float>(Index) /static_cast<float>(SamplesPerArc);
            const FVector PointA = EvaluateBezier(-E1,-C1,C1,E1,T);
            const FVector PointB = EvaluateBezier(-E2,-C2,C2,E2,T);

            OutArcA.Add(PointA);
            OutArcB.Add(PointB);
        }
    }
    static float CalculateLengthScale(float DistanceCm,float MaxRangeCm,float MinScale,float MaxScale);
};