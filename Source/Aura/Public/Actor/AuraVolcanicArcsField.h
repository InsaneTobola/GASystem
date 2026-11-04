#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GameplayTagContainer.h"
#include "AuraVolcanicArcsField.generated.h"

class UStaticMesh;
class UMaterialInterface;
class USplineMeshComponent;
class UCapsuleComponent;
class UGameplayEffect;
class UGameplayAbility;
class UAbilitySystemComponent;
class UPrimitiveComponent;

UCLASS()
class AURA_API AAuraVolcanicArcsField : public AActor
{
    GENERATED_BODY()

public:

    AAuraVolcanicArcsField();

    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

    void InitializeField(
        float InBaseHalfLengthCm,
        float InArcLengthScale,
        float InArcBowOffsetCm,
        const FVector& InIntersectionLocal,
        float InArcCollisionRadiusCm,
        float InIntersectionRadiusCm,
        float InDurationSeconds,
        TSubclassOf<UGameplayEffect> InDamageEffectClass,
        const TMap<FGameplayTag, float>& InDamageValues,
        int32 InAbilityLevel,
        UGameplayAbility* InSourceAbility,
        AActor* InSourceActor
    );

protected:

    virtual void BeginPlay() override;

private:

    UPROPERTY(EditDefaultsOnly, Category = "Visual")
    TObjectPtr<UStaticMesh> ArcSegmentMesh;

    UPROPERTY(EditDefaultsOnly, Category = "Visual")
    TObjectPtr<UMaterialInterface> ArcMaterial;

    UPROPERTY(ReplicatedUsing = OnRep_ArcSettings)
    float BaseHalfLengthCm = 250.f;

    UPROPERTY(ReplicatedUsing = OnRep_ArcSettings)
    float ArcLengthScale = 1.f;

    UPROPERTY(ReplicatedUsing = OnRep_ArcSettings)
    float ArcBowOffsetCm = 90.f;

    UPROPERTY(ReplicatedUsing = OnRep_ArcSettings)
    FVector IntersectionLocal = FVector::ZeroVector;
    
    UPROPERTY(Transient)
    float ArcCollisionRadiusCm = 35.f;

    UPROPERTY(Transient)
    float IntersectionRadiusCm = 65.f;

    UPROPERTY(Transient)
    float DurationSeconds = 0.5f;

    UPROPERTY(Transient)
    TObjectPtr<AActor> SourceActor;

    UPROPERTY(Transient)
    TObjectPtr<UGameplayAbility> SourceAbility;

    UPROPERTY(Transient)
    TSubclassOf<UGameplayEffect> DamageEffectClass;

    UPROPERTY(Transient)
    TMap<FGameplayTag, float> ScaledDamageValues;

    UPROPERTY(Transient)
    int32 AbilityLevel = 1;

    UPROPERTY(Transient)
    TArray<TObjectPtr<USplineMeshComponent>> ArcVisualSegments;

    UPROPERTY(Transient)
    TArray<TObjectPtr<UCapsuleComponent>> ArcCollisionCapsules;

    TSet<TWeakObjectPtr<AActor>> AffectedActors;

    UFUNCTION()
    void OnRep_ArcSettings();

    UFUNCTION()
    void OnArcBeginOverlap(UPrimitiveComponent* OverlappedComponent,AActor* OtherActor,UPrimitiveComponent* OtherComp,int32 OtherBodyIndex,bool bFromSweep,const FHitResult& SweepResult
    );

    void BuildVisuals();
    void BuildVisualsForPath(const TArray<FVector>& Path);
    void ClearVisuals();
    void BuildCollision();
    void BuildCollisionForPath(const TArray<FVector>& Path);
    void ClearCollision();
    void ApplyDamageToActor(AActor* OtherActor);
};