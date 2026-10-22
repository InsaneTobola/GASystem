#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GameplayTagContainer.h"
#include "GameplayEffectTypes.h"
#include "AuraAreaSpellField.generated.h"

class USphereComponent;
class UStaticMeshComponent;
class UMaterialInterface;
class UMaterialInstanceDynamic;
class UGameplayEffect;
class UGameplayAbility;
class UAbilitySystemComponent;
class UPrimitiveComponent;

UCLASS()
class AURA_API AAuraAreaSpellField : public AActor
{
    GENERATED_BODY()

public:
    AAuraAreaSpellField();

    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

    void InitializeField(float InAreaRadiusCm,float InDurationSeconds,TSubclassOf<UGameplayEffect> InDamageEffectClass,
        const TMap<FGameplayTag, float>& InDamageValues,int32 InAbilityLevel,UGameplayAbility* InSourceAbility,
        AActor* InSourceActor,const TArray<TSubclassOf<UGameplayEffect>>& InAdditionalEffects,bool bInAffectOnlyEnemies,
        bool bInApplyKnockback,float InKnockbackStrength
    );

protected:
    virtual void BeginPlay() override;

private:
    UPROPERTY(VisibleAnywhere)
    TObjectPtr<USphereComponent> AreaSphere;

    UPROPERTY(VisibleAnywhere)
    TObjectPtr<UStaticMeshComponent> AreaVisual;

    UPROPERTY(EditDefaultsOnly, Category = "Visual")
    TObjectPtr<UMaterialInterface> ActiveAreaMaterial;

    UPROPERTY(Transient)
    TObjectPtr<UMaterialInstanceDynamic> DynamicAreaMaterial;

    UPROPERTY(ReplicatedUsing = OnRep_FieldSettings)
    float AreaRadiusCm = 75.f;

    UPROPERTY(ReplicatedUsing = OnRep_FieldSettings)
    float DurationSeconds = 0.5f;

    UPROPERTY()
    TObjectPtr<AActor> SourceActor;

    UPROPERTY()
    TObjectPtr<UGameplayAbility> SourceAbility;

    UPROPERTY(Transient)
    TSubclassOf<UGameplayEffect> DamageEffectClass;

    UPROPERTY(Transient)
    TArray<TSubclassOf<UGameplayEffect>> AdditionalEffectClasses;

    UPROPERTY(Transient)
    TMap<FGameplayTag, float> ScaledDamageValues;

    UPROPERTY(Transient)
    int32 AbilityLevel = 1;

    UPROPERTY(Transient)
    bool bAffectOnlyEnemies = true;

    UPROPERTY(Transient)
    bool bApplyKnockback = false;

    UPROPERTY(Transient)
    float KnockbackStrength = 600.f;

    TSet<TWeakObjectPtr<AActor>> AffectedActors;

    UFUNCTION()
    void OnRep_FieldSettings();

    UFUNCTION()
    void OnAreaBeginOverlap(UPrimitiveComponent* OverlappedComponent,AActor* OtherActor,UPrimitiveComponent* OtherComp,
        int32 OtherBodyIndex,bool bFromSweep,const FHitResult& SweepResult);

    void UpdateVisuals();

    void ApplyToActor(AActor* OtherActor);
};