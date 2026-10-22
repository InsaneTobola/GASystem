#include "Magic/MagicTargetingActor.h"

#include "Components/SceneComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Materials/MaterialInterface.h"
#include "Engine/StaticMesh.h"
#include "Engine/World.h"
#include "Engine/EngineTypes.h"
#include "CollisionQueryParams.h"
#include "Components/CapsuleComponent.h"
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

    bHasTarget =OwningPlayerController->GetHitResultUnderCursorByChannel(UEngineTypes::ConvertToTraceType(ECC_WorldStatic),false,Hit);

    if (!bHasTarget)
    {
        AreaMesh->SetVisibility(false);
        bTargetValid = false;
        UpdateMaterials();
        UpdateRangeRing();
        return;
    }

    CurrentTargetLocation = Hit.Location;
    AreaMesh->SetVisibility(true);
    AreaMesh->SetWorldLocation(Hit.Location + Hit.Normal * 3.f);

    const float Diameter = AreaRadiusCm * 2.f;
    const float MeshScale = Diameter / 100.f;

    AreaMesh->SetWorldScale3D(FVector(MeshScale, MeshScale, 1.f));

    const float Distance2D = FVector::Dist2D(Caster->GetActorLocation(),CurrentTargetLocation);

    bTargetValid = Distance2D <= MaxRangeCm;

    UpdateMaterials();
    UpdateRangeRing();
}

bool AAuraMagicTargetingActor::HasValidTarget() const
{
    return bHasTarget && bTargetValid;
}

FVector AAuraMagicTargetingActor::GetTargetLocation() const
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