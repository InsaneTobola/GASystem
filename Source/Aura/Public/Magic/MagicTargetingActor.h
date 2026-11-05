#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "MagicTargetingActor.generated.h"

class APlayerController;
class UStaticMesh;
class UStaticMeshComponent;
class USplineMeshComponent;
class UMaterialInterface;
class UMaterialInstanceDynamic;


UCLASS()
class AURA_API AAuraMagicTargetingActor : public AActor
{
	GENERATED_BODY()

public:
	AAuraMagicTargetingActor();

	virtual void Tick(float DeltaTime) override;

	void InitializeTargeting(APlayerController* InPlayerController,AActor* InCaster,float InMaxRangeCm,float InAreaRadiusCm);

	bool HasValidTarget() const;

	FVector GetMagicTargetLocation() const;

	void InitializeVolcanicArcsTargeting(APlayerController* InPlayerController,AActor* InCaster,float InMaxRangeCm,float InBaseHalfLengthCm,float InDefaultScale,float InMinScale,float InMaxScale,float InArcBowOffsetCm,float InArcWidthCm);
	
protected:
	virtual void BeginPlay() override;

private:
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<USceneComponent> SceneRoot;

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UStaticMeshComponent> AreaMesh;

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UStaticMeshComponent> RangeMesh;

	UPROPERTY(EditDefaultsOnly, Category = "Visual")
	TObjectPtr<UMaterialInterface> AreaPreviewMaterial;

	UPROPERTY(EditDefaultsOnly, Category = "Visual")
	TObjectPtr<UMaterialInterface> RangeRingMaterial;

	UPROPERTY(Transient)
	TObjectPtr<UMaterialInstanceDynamic> DynamicAreaMaterial;

	UPROPERTY(Transient)
	TObjectPtr<UMaterialInstanceDynamic> DynamicRangeMaterial;

	UPROPERTY()
	TObjectPtr<APlayerController> OwningPlayerController;

	UPROPERTY()
	TObjectPtr<AActor> Caster;

	float MaxRangeCm = 1000.f;
	float AreaRadiusCm = 75.f;

	FVector CurrentTargetLocation = FVector::ZeroVector;

	bool bHasTarget = false;
	bool bTargetValid = false;

	void UpdateMaterials();
	void UpdateRangeRing();
	
	UPROPERTY(EditDefaultsOnly, Category = "Magic|Volcanic Arcs")
	TObjectPtr<UStaticMesh> ArcSegmentMesh;

	UPROPERTY(EditDefaultsOnly, Category = "Magic|Volcanic Arcs")
	TObjectPtr<UMaterialInterface> ArcPreviewMaterial;

	UPROPERTY(Transient)
	TObjectPtr<UMaterialInstanceDynamic> DynamicArcPreviewMaterial;

	UPROPERTY(Transient)
	TArray<TObjectPtr<USplineMeshComponent>> ArcPreviewSegments;

	bool bVolcanicArcsMode = false;

	float ArcHalfLengthCm = 250.f;

	float ArcLengthScale = 1.f;
	float ArcWidthCm = 12.f;
	float MinArcLengthScale = 0.7f;

	float MaxArcLengthScale = 1.4f;
	
	float ArcBowOffsetCm = 90.f;

	FVector CurrentIntersectionLocal = FVector::ZeroVector;

	FVector LastPreviewIntersectionLocal = FVector::ZeroVector;
	
	FVector GetCasterGroundLocation() const;

	bool bHasBuiltArcPreview = false;

	void EnsureArcPreviewComponents();

	void UpdateArcPreview();

	void HideArcPreview();
};