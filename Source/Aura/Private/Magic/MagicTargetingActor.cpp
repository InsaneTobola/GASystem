#include "Magic/MagicTargetingActor.h"
#include "Magic/VolcanicArcsGeometry.h"

#include "Components/SceneComponent.h"
#include "Components/StaticMeshComponent.h"

#include "Materials/MaterialInstanceDynamic.h"
#include "Materials/MaterialInterface.h"

#include "Engine/StaticMesh.h"
#include "Engine/World.h"
#include "Engine/EngineTypes.h"

#include "CollisionQueryParams.h"
#include "Components/CapsuleComponent.h"
#include "Components/SplineMeshComponent.h"

#include "GameFramework/PlayerController.h"
#include "GameFramework/Actor.h"
#include "GameFramework/Character.h"

#include "UObject/ConstructorHelpers.h"

AAuraMagicTargetingActor::AAuraMagicTargetingActor()
{
    PrimaryActorTick.bCanEverTick = true;

    bReplicates = false;

    SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));

    SetRootComponent(SceneRoot);

    AreaMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("AreaMesh"));

    AreaMesh->SetupAttachment(SceneRoot);
    AreaMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    AreaMesh->SetCastShadow(false);

    RangeMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("RangeMesh"));

    RangeMesh->SetupAttachment(SceneRoot);
    RangeMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    RangeMesh->SetCastShadow(false);

    static ConstructorHelpers::FObjectFinder<UStaticMesh> CylinderMesh(TEXT("/Engine/BasicShapes/Cylinder.Cylinder"));

    if (CylinderMesh.Succeeded())
    {
        AreaMesh->SetStaticMesh(CylinderMesh.Object);
        RangeMesh->SetStaticMesh(CylinderMesh.Object);
    }
}

void AAuraMagicTargetingActor::BeginPlay()
{
    Super::BeginPlay();

    if (AreaPreviewMaterial)
    {
        DynamicAreaMaterial = UMaterialInstanceDynamic::Create(AreaPreviewMaterial,this);
        AreaMesh->SetMaterial(0, DynamicAreaMaterial);
    }

    if (RangeRingMaterial)
    {
        DynamicRangeMaterial = UMaterialInstanceDynamic::Create(RangeRingMaterial,this);
        RangeMesh->SetMaterial(0, DynamicRangeMaterial);
    }

    if (ArcPreviewMaterial)
    {
        DynamicArcPreviewMaterial = UMaterialInstanceDynamic::Create(ArcPreviewMaterial, this);
    }
    
    AreaMesh->SetVisibility(false);
}

void AAuraMagicTargetingActor::InitializeTargeting(APlayerController* InPlayerController,AActor* InCaster,float InMaxRangeCm,float InAreaRadiusCm)
{
    OwningPlayerController = InPlayerController;
    Caster = InCaster;

    MaxRangeCm = FMath::Max(InMaxRangeCm, 1.f);
    AreaRadiusCm = FMath::Max(InAreaRadiusCm, 1.f);

    UpdateMaterials();
    UpdateRangeRing();
}

void AAuraMagicTargetingActor::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if (!IsValid(OwningPlayerController) || !IsValid(Caster))
    {
        bHasTarget = false;
        AreaMesh->SetVisibility(false);
        return;
    }

    FHitResult Hit;

    bHasTarget = OwningPlayerController->GetHitResultUnderCursorByChannel(UEngineTypes::ConvertToTraceType(ECC_GameTraceChannel3),false,Hit);

    if (!bHasTarget)
    {
        AreaMesh->SetVisibility(false);
        bTargetValid = false;

        if (bVolcanicArcsMode)
        {
            HideArcPreview();
        }

        UpdateMaterials();
        UpdateRangeRing();
        return;
    }
    
    if (const UPrimitiveComponent* HitComponent = Hit.GetComponent())
    {
        UE_LOG(
            LogTemp,
            Warning,
            TEXT(
                "[MagicTargeting] Actor=%s | Component=%s | "
                "Location=(%.1f, %.1f, %.1f) | "
                "ObjectType=%d | WorldStaticResponse=%d | "
                "VisibilityResponse=%d"
            ),
            *GetNameSafe(Hit.GetActor()),
            *GetNameSafe(HitComponent),
            Hit.Location.X,
            Hit.Location.Y,
            Hit.Location.Z,
            static_cast<int32>(HitComponent->GetCollisionObjectType()),
            static_cast<int32>(HitComponent->GetCollisionResponseToChannel(ECC_WorldStatic)),
            static_cast<int32>(HitComponent->GetCollisionResponseToChannel(ECC_Visibility))
        );
    }
    CurrentTargetLocation = Hit.Location;
    
    if (bVolcanicArcsMode)
    {
        // The cursor target is the intersection point of both arcs.
        const FVector ToTarget = (CurrentTargetLocation - Caster->GetActorLocation()).GetSafeNormal2D();
        const FVector FacingDirection =ToTarget.IsNearlyZero()? Caster->GetActorForwardVector(): ToTarget;

        // Keep local arc geometry centered at the cursor.
        SetActorLocationAndRotation(CurrentTargetLocation + FVector(0.f, 0.f, 3.f),            FRotator(0.f, FacingDirection.Rotation().Yaw, 0.f));

        const float Distance2D = FVector::Dist2D(Caster->GetActorLocation(),CurrentTargetLocation);
        const bool bPreviousTargetValid = bTargetValid;
        bTargetValid = Distance2D <= MaxRangeCm &&Hit.ImpactNormal.Z >= 0.7f;
        const float NewArcLengthScale = FVolcanicArcsGeometry::CalculateLengthScale(Distance2D,MaxRangeCm,MinArcLengthScale,MaxArcLengthScale);
        const bool bLengthChanged =! FMath::IsNearlyEqual(ArcLengthScale,NewArcLengthScale,0.001f);

        ArcLengthScale = NewArcLengthScale;

        if (bLengthChanged || bPreviousTargetValid != bTargetValid)
        {
            UpdateArcPreview();
        }

        AreaMesh->SetVisibility(false);

        UpdateRangeRing();

        return;
    }
    
    if (bVolcanicArcsMode)
    {
        const FVector Forward = (CurrentTargetLocation - Caster->GetActorLocation()).GetSafeNormal2D();

        SetActorLocation(CurrentTargetLocation + FVector(0.f, 0.f, 3.f));

        SetActorRotation(FRotator(0.f, Forward.Rotation().Yaw, 0.f));

        const float Distance2D = FVector::Dist2D(Caster->GetActorLocation(),CurrentTargetLocation);

        bTargetValid = Distance2D <= MaxRangeCm && Hit.ImpactNormal.Z >= 0.7f;

        AreaMesh->SetVisibility(false);

        UpdateArcPreview();
        UpdateRangeRing();

        return;
    }
    
    HideArcPreview();
    
    AreaMesh->SetVisibility(true);
    AreaMesh->SetWorldLocation(Hit.Location + Hit.Normal * 3.f);

    const float Diameter = AreaRadiusCm * 2.f;
    const float MeshScale = Diameter / 100.f;

    AreaMesh->SetWorldScale3D(FVector(MeshScale, MeshScale, 1.f));

    const float Distance2D = FVector::Dist2D(Caster->GetActorLocation(),CurrentTargetLocation);

    const bool bWalkableSurface = Hit.ImpactNormal.Z >= 0.7f;

    bTargetValid = bWalkableSurface && Distance2D <= MaxRangeCm;

    UpdateMaterials();
    UpdateRangeRing();
}

bool AAuraMagicTargetingActor::HasValidTarget() const
{
    return bHasTarget && bTargetValid;
}

FVector AAuraMagicTargetingActor::GetMagicTargetLocation() const
{
    return CurrentTargetLocation;
}

void AAuraMagicTargetingActor::UpdateMaterials()
{
    if (!DynamicAreaMaterial)
    {
        return;
    }

    const FLinearColor Color = bTargetValid
        ? FLinearColor(0.05f, 0.9f, 0.2f, 1.f)
        : FLinearColor(1.f, 0.03f, 0.02f, 1.f);

    DynamicAreaMaterial->SetVectorParameterValue(FName("Tint"),Color);
    DynamicAreaMaterial->SetScalarParameterValue(FName("Opacity"),0.2f);
}

void AAuraMagicTargetingActor::UpdateRangeRing()
{
    if (!IsValid(Caster) || !IsValid(RangeMesh))
    {
        if (IsValid(RangeMesh))
        {
            RangeMesh->SetVisibility(false);
        }

        return;
    }

    const FVector CasterLocation = Caster->GetActorLocation();
    float CapsuleHalfHeight = 0.f;

    if (const ACharacter* CasterCharacter = Cast<ACharacter>(Caster))
    {
        if (const UCapsuleComponent* Capsule = CasterCharacter->GetCapsuleComponent())
        {
            CapsuleHalfHeight = Capsule->GetScaledCapsuleHalfHeight();
        }
    }

    FVector GroundLocation = CasterLocation - FVector(0.f, 0.f, CapsuleHalfHeight);

    if (UWorld* World = GetWorld())
    {
        FHitResult GroundHit;

        FCollisionQueryParams Params(SCENE_QUERY_STAT(MagicRangeGroundTrace),true);

        Params.AddIgnoredActor(Caster);
        Params.AddIgnoredActor(this);

        const FVector TraceStart = GroundLocation + FVector(0.f, 0.f, 50.f);
        const FVector TraceEnd = GroundLocation - FVector(0.f, 0.f, 100.f);

        if (World->LineTraceSingleByChannel(
            GroundHit,
            TraceStart,
            TraceEnd,
            ECC_WorldStatic,
            Params))
        {
            GroundLocation = GroundHit.Location;
        }
    }

    // Środek pierścienia zawsze odpowiada pozycji gracza w osi X/Y.
    RangeMesh->SetWorldLocation(FVector(CasterLocation.X,CasterLocation.Y,GroundLocation.Z + 2.f));
    RangeMesh->SetWorldRotation(FRotator::ZeroRotator);
    
    const float MeshScale = MaxRangeCm / 50.f;

    RangeMesh->SetWorldScale3D(FVector(MeshScale, MeshScale, 1.f));
    RangeMesh->SetVisibility(true);

    if (DynamicRangeMaterial)
    {
        DynamicRangeMaterial->SetVectorParameterValue(FName("Tint"),FLinearColor(1.f, 1.f, 1.f, 1.f));
    }
}

void AAuraMagicTargetingActor::InitializeVolcanicArcsTargeting(APlayerController* InPlayerController,AActor* InCaster,
    float InMaxRangeCm,
    float InBaseHalfLengthCm,
    float InDefaultScale,
    float InMinScale,
    float InMaxScale)
{
    OwningPlayerController = InPlayerController;
    Caster = InCaster;

    MaxRangeCm = FMath::Max(InMaxRangeCm, 1.f);

    ArcHalfLengthCm = FMath::Max(InBaseHalfLengthCm, 1.f);

    MinArcLengthScale = InMinScale;
    MaxArcLengthScale = InMaxScale;

    ArcLengthScale = FMath::Clamp(InDefaultScale,MinArcLengthScale,MaxArcLengthScale);

    bVolcanicArcsMode = true;

    AreaMesh->SetVisibility(false);

    UpdateRangeRing();

    EnsureArcPreviewComponents();
    UpdateArcPreview();
}

void AAuraMagicTargetingActor::EnsureArcPreviewComponents()
{
    if (!ArcSegmentMesh)
    {
        UE_LOG(
            LogTemp,
            Error,
            TEXT("Volcanic Arcs: ArcSegmentMesh is not assigned.")
        );

        return;
    }

    const int32 RequiredSegments = FVolcanicArcsGeometry::SamplesPerArc * 2;

    if (ArcPreviewSegments.Num() == RequiredSegments)
    {
        return;
    }

    HideArcPreview();

    for (USplineMeshComponent* Component : ArcPreviewSegments)
    {
        if (IsValid(Component))
        {
            Component->DestroyComponent();
        }
    }

    ArcPreviewSegments.Reset();

    for (int32 Index = 0; Index < RequiredSegments; ++Index)
    {
        USplineMeshComponent* Segment = NewObject<USplineMeshComponent>(this);

        if (!IsValid(Segment))
        {
            continue;
        }

        Segment->SetupAttachment(SceneRoot);
        Segment->SetStaticMesh(ArcSegmentMesh);
        Segment->SetForwardAxis(ESplineMeshAxis::X);
        Segment->SetCollisionEnabled(ECollisionEnabled::NoCollision);
        Segment->SetCastShadow(false);
        Segment->SetStartScale(FVector2D(1.f, 1.f));
        Segment->SetEndScale(FVector2D(1.f, 1.f));

        if (DynamicArcPreviewMaterial)
        {
            Segment->SetMaterial(0, DynamicArcPreviewMaterial);
        }

        AddInstanceComponent(Segment);
        Segment->RegisterComponent();

        ArcPreviewSegments.Add(Segment);
    }
}

void AAuraMagicTargetingActor::UpdateArcPreview()
{
    if (!bVolcanicArcsMode || !ArcSegmentMesh)
    {
        return;
    }

    EnsureArcPreviewComponents();

    TArray<FVector> ArcA;
    TArray<FVector> ArcB;

    FVolcanicArcsGeometry::Build(ArcHalfLengthCm,ArcLengthScale,ArcA,ArcB);

    int32 SegmentIndex = 0;

    auto UpdatePath = [&](const TArray<FVector>& Path)
    {
        for (int32 Index = 0; Index < Path.Num() - 1; ++Index)
        {
            if (!ArcPreviewSegments.IsValidIndex(SegmentIndex))
            {
                return;
            }

            USplineMeshComponent* Segment = ArcPreviewSegments[SegmentIndex++];

            if (!IsValid(Segment))
            {
                continue;
            }

            const FVector Start = Path[Index];
            const FVector End = Path[Index + 1];
            const FVector StartTangent = Index == 0? End - Start: (Path[Index + 1] - Path[Index - 1]) * 0.5f;
            const FVector EndTangent = Index + 2 >= Path.Num()? End - Start: (Path[Index + 2] - Path[Index]) * 0.5f;

            Segment->SetStartScale(FVector2D(1.f, 1.f));
            Segment->SetEndScale(FVector2D(1.f, 1.f));
            Segment->SetStartAndEnd(Start,StartTangent,End,EndTangent,true);
            Segment->SetVisibility(bHasTarget);
        }
    };

    SegmentIndex = 0;

    UpdatePath(ArcA);
    UpdatePath(ArcB);

    if (DynamicArcPreviewMaterial)
    {
        const FLinearColor Color = bTargetValid
            ? FLinearColor(1.f, 0.12f, 0.02f, 1.f)
            : FLinearColor(1.f, 0.f, 0.f, 1.f);

        DynamicArcPreviewMaterial->SetVectorParameterValue(FName("Tint"),Color);
    }
}

void AAuraMagicTargetingActor::HideArcPreview()
{
    for (USplineMeshComponent* Segment : ArcPreviewSegments)
    {
        if (IsValid(Segment))
        {
            Segment->SetVisibility(false);
        }
    }
}

