#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "MagicTargetingActor.generated.h"

class APlayerController;
class UStaticMeshComponent;
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

	FVector GetTargetLocation() const;

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
};