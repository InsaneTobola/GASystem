#include "Actor/AuraAreaSpellField.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystem/AuraAttributeSet.h"

#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/Character.h"
#include "AuraGameplayTags.h"

#include "Interaction/EnemyInterface.h"
#include "Materials/MaterialInterface.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Magic/MagicDebug.h"

#include "Net/UnrealNetwork.h"
#include "UObject/ConstructorHelpers.h"

AAuraAreaSpellField::AAuraAreaSpellField()
{
    PrimaryActorTick.bCanEverTick = false;

    bReplicates = true;
    SetReplicateMovement(false);

    USceneComponent* SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));

    SetRootComponent(SceneRoot);

    AreaVisual = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("AreaVisual"));

    AreaVisual->SetupAttachment(SceneRoot);
    AreaVisual->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    AreaVisual->SetCastShadow(false);

    AreaSphere = CreateDefaultSubobject<USphereComponent>(TEXT("AreaSphere"));

    AreaSphere->SetupAttachment(SceneRoot);
    AreaSphere->SetRelativeLocation(FVector(0.f, 0.f, 90.f));

    AreaSphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    AreaSphere->SetCollisionObjectType(ECC_WorldDynamic);
    AreaSphere->SetCollisionResponseToAllChannels(ECR_Ignore);
    AreaSphere->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
    AreaSphere->SetGenerateOverlapEvents(true);

    AreaSphere->OnComponentBeginOverlap.AddDynamic(this,&AAuraAreaSpellField::OnAreaBeginOverlap);

    static ConstructorHelpers::FObjectFinder<UStaticMesh> CylinderMesh(TEXT("/Engine/BasicShapes/Cylinder.Cylinder"));

    if (CylinderMesh.Succeeded())
    {
        AreaVisual->SetStaticMesh(CylinderMesh.Object);
    }
}

void AAuraAreaSpellField::InitializeField(
    float InAreaRadiusCm,
    float InDurationSeconds,
    TSubclassOf<UGameplayEffect> InDamageEffectClass,
    TSubclassOf<UGameplayEffect> InKnockbackEffectClass,
    const TMap<FGameplayTag, float>& InDamageValues,
    int32 InAbilityLevel,
    UGameplayAbility* InSourceAbility,
    AActor* InSourceActor,
    const TArray<TSubclassOf<UGameplayEffect>>& InAdditionalEffects,
    bool bInAffectOnlyEnemies,
    bool bInApplyKnockback,
    float InKnockbackStrength)
{
    AreaRadiusCm = FMath::Max(InAreaRadiusCm, 1.f);
    DurationSeconds = FMath::Max(InDurationSeconds, 0.01f);

    DamageEffectClass = InDamageEffectClass;
    KnockbackEffectClass = InKnockbackEffectClass;
    ScaledDamageValues = InDamageValues;
    AbilityLevel = FMath::Max(InAbilityLevel, 1);

    SourceAbility = InSourceAbility;
    SourceActor = InSourceActor;

    AdditionalEffectClasses = InAdditionalEffects;

    bAffectOnlyEnemies = bInAffectOnlyEnemies;
    bApplyKnockback = bInApplyKnockback;
    KnockbackStrength = FMath::Max(InKnockbackStrength, 0.f);

    AreaSphere->SetSphereRadius(FMath::Max(AreaRadiusCm, 200.f),true);

    UpdateVisuals();
}

void AAuraAreaSpellField::BeginPlay()
{
    Super::BeginPlay();

    if (ActiveAreaMaterial)
    {
        DynamicAreaMaterial = UMaterialInstanceDynamic::Create(ActiveAreaMaterial,this);
        AreaVisual->SetMaterial(0, DynamicAreaMaterial);
    }

    UpdateVisuals();

    if (HasAuthority())
    {
        SetLifeSpan(DurationSeconds);

        AreaSphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
        AreaSphere->UpdateOverlaps();

        TArray<AActor*> OverlappingActors;
        AreaSphere->GetOverlappingActors(OverlappingActors);

        AuraMagicDebugPrint(
    FString::Printf(
        TEXT("AREA FIELD: Authority=%d Radius=%.1f Duration=%.2f InitialOverlaps=%d DamageEffect=%s"),
        HasAuthority() ? 1 : 0,
        AreaRadiusCm,
        DurationSeconds,
        OverlappingActors.Num(),
        *GetNameSafe(DamageEffectClass.Get())
    ),
    FColor::Cyan,
    5.f
);
        
        for (AActor* Actor : OverlappingActors)
        {
            ApplyToActor(Actor);
        }
    }
    else
    {
        AreaSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    }
}

void AAuraAreaSpellField::GetLifetimeReplicatedProps(
    TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME(AAuraAreaSpellField, AreaRadiusCm);
    DOREPLIFETIME(AAuraAreaSpellField, DurationSeconds);
}

void AAuraAreaSpellField::OnRep_FieldSettings()
{
    UpdateVisuals();
}

void AAuraAreaSpellField::UpdateVisuals()
{
    if (AreaVisual)
    {
        const float MeshScale = AreaRadiusCm / 50.f;

        AreaVisual->SetWorldScale3D(FVector(MeshScale, MeshScale, 1.f));
        AreaVisual->SetRelativeLocation(FVector(0.f, 0.f, 3.f));
    }

    if (AreaSphere)
    {
        AreaSphere->SetSphereRadius(FMath::Max(AreaRadiusCm, 200.f),true);
    }

    if (DynamicAreaMaterial)
    {
        DynamicAreaMaterial->SetVectorParameterValue(FName("Tint"),FLinearColor(1.f, 0.25f, 0.02f, 1.f));
        DynamicAreaMaterial->SetScalarParameterValue(FName("Opacity"),0.15f
        );
    }
}

void AAuraAreaSpellField::OnAreaBeginOverlap(UPrimitiveComponent* OverlappedComponent,AActor* OtherActor,UPrimitiveComponent* OtherComp,int32 OtherBodyIndex,bool bFromSweep,const FHitResult& SweepResult)
{
    AuraMagicDebugPrint(
    FString::Printf(
        TEXT("AREA FIELD: Overlap event with %s"),
        *GetNameSafe(OtherActor)
    ),
    FColor::Yellow
);
    
    ApplyToActor(OtherActor);
}

void AAuraAreaSpellField::ApplyToActor(AActor* OtherActor)
{
    if (!HasAuthority())
    {
        return;
    }

    if (!IsValid(OtherActor) || OtherActor == SourceActor)
    {
        return;
    }

    AuraMagicDebugPrint(
        FString::Printf(
            TEXT("AREA FIELD: Candidate=%s"),
            *GetNameSafe(OtherActor)
        ),
        FColor::Cyan
    );

    if (bAffectOnlyEnemies &&
        !OtherActor->GetClass()->ImplementsInterface(
            UEnemyInterface::StaticClass()
        ))
    {
        AuraMagicDebugPrint(
            FString::Printf(
                TEXT("AREA FIELD SKIP: %s does not implement EnemyInterface."),
                *GetNameSafe(OtherActor)
            ),
            FColor::Orange
        );

        return;
    }

    const float Distance2D = FVector::Dist2D(
        GetActorLocation(),
        OtherActor->GetActorLocation()
    );

    if (Distance2D > AreaRadiusCm)
    {
        AuraMagicDebugPrint(
            FString::Printf(
                TEXT("AREA FIELD SKIP: %s outside radius. Distance=%.1f Radius=%.1f"),
                *GetNameSafe(OtherActor),
                Distance2D,
                AreaRadiusCm
            ),
            FColor::Orange
        );

        return;
    }

    const TWeakObjectPtr<AActor> WeakTarget(OtherActor);

    if (AffectedActors.Contains(WeakTarget))
    {
        AuraMagicDebugPrint(
            FString::Printf(
                TEXT("AREA FIELD SKIP: %s already affected."),
                *GetNameSafe(OtherActor)
            ),
            FColor::Orange
        );

        return;
    }

    UAbilitySystemComponent* SourceASC =
        UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(
            SourceActor
        );

    UAbilitySystemComponent* TargetASC =
        UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(
            OtherActor
        );

    if (!IsValid(SourceASC) ||
        !IsValid(TargetASC) ||
        !DamageEffectClass)
    {
        AuraMagicDebugPrint(
            FString::Printf(
                TEXT("AREA FIELD ERROR: SourceASC=%d TargetASC=%d DamageEffect=%s"),
                IsValid(SourceASC) ? 1 : 0,
                IsValid(TargetASC) ? 1 : 0,
                *GetNameSafe(DamageEffectClass.Get())
            ),
            FColor::Red,
            6.f
        );

        return;
    }

    FGameplayEffectContextHandle ContextHandle =
        SourceASC->MakeEffectContext();

    ContextHandle.SetAbility(SourceAbility);
    ContextHandle.AddSourceObject(SourceActor);

    const FGameplayEffectSpecHandle DamageSpecHandle =
        SourceASC->MakeOutgoingSpec(
            DamageEffectClass,
            AbilityLevel,
            ContextHandle
        );

    if (!DamageSpecHandle.IsValid() ||
        !DamageSpecHandle.Data.IsValid())
    {
        AuraMagicDebugPrint(
            TEXT("AREA FIELD ERROR: Damage GameplayEffectSpec is invalid."),
            FColor::Red
        );

        return;
    }

    for (const TPair<FGameplayTag, float>& Pair : ScaledDamageValues)
    {
        UAbilitySystemBlueprintLibrary::AssignTagSetByCallerMagnitude(
            DamageSpecHandle,
            Pair.Key,
            Pair.Value
        );
    }

    AuraMagicDebugPrint(
        FString::Printf(
            TEXT("AREA FIELD: Applying %s to %s, DamageEntries=%d"),
            *GetNameSafe(DamageEffectClass.Get()),
            *GetNameSafe(OtherActor),
            ScaledDamageValues.Num()
        ),
        FColor::Green,
        5.f
    );

    TargetASC->ApplyGameplayEffectSpecToSelf(
        *DamageSpecHandle.Data.Get()
    );

    // Nie powtarzaj obrażeń temu aktorowi w czasie życia tego pola.
    AffectedActors.Add(WeakTarget);

    for (const TSubclassOf<UGameplayEffect>& EffectClass :
         AdditionalEffectClasses)
    {
        if (!EffectClass)
        {
            AuraMagicDebugPrint(
                TEXT("AREA FIELD WARNING: One additional effect class is empty."),
                FColor::Orange
            );

            continue;
        }

        FGameplayEffectContextHandle AdditionalContext =
            SourceASC->MakeEffectContext();

        AdditionalContext.SetAbility(SourceAbility);
        AdditionalContext.AddSourceObject(SourceActor);

        const FGameplayEffectSpecHandle AdditionalSpec =
            SourceASC->MakeOutgoingSpec(
                EffectClass,
                AbilityLevel,
                AdditionalContext
            );

        if (!AdditionalSpec.IsValid() ||
            !AdditionalSpec.Data.IsValid())
        {
            AuraMagicDebugPrint(
                FString::Printf(
                    TEXT("AREA FIELD ERROR: Could not create spec for %s"),
                    *GetNameSafe(EffectClass.Get())
                ),
                FColor::Red
            );

            continue;
        }

        AuraMagicDebugPrint(
            FString::Printf(
                TEXT("AREA FIELD: Applying additional effect %s to %s"),
                *GetNameSafe(EffectClass.Get()),
                *GetNameSafe(OtherActor)
            ),
            FColor::Green
        );

        TargetASC->ApplyGameplayEffectSpecToSelf(
            *AdditionalSpec.Data.Get()
        );
    }

    if (bApplyKnockback)
    {
        AuraMagicDebugPrint(
            FString::Printf(
                TEXT("AREA FIELD: Knockback requested. EffectClass=%s"),
                *GetNameSafe(KnockbackEffectClass.Get())
            ),
            KnockbackEffectClass ? FColor::Green : FColor::Red
        );

        ApplyKnockbackEffect(OtherActor, SourceASC, TargetASC);
    }
}

void AAuraAreaSpellField::ApplyKnockbackEffect(
    AActor* OtherActor,
    UAbilitySystemComponent* SourceASC,
    UAbilitySystemComponent* TargetASC)
{
    if (!bApplyKnockback ||
        !IsValid(OtherActor) ||
        !IsValid(SourceASC) ||
        !IsValid(TargetASC))
    {
        return;
    }

    if (!KnockbackEffectClass)
    {
        UE_LOG(
            LogTemp,
            Error,
            TEXT("[Knockback] KnockbackEffectClass is not assigned.")
        );
        return;
    }

    const float SourceKnockback =
        SourceASC->GetNumericAttribute(
            UAuraAttributeSet::GetKnockbackAttribute()
        );

    const float EffectiveStrength =
        KnockbackStrength *
        (1.f + FMath::Max(SourceKnockback, 0.f) * 0.01f);

    FGameplayEffectContextHandle ContextHandle =
        SourceASC->MakeEffectContext();

    ContextHandle.SetAbility(SourceAbility);
    ContextHandle.AddSourceObject(SourceActor);
    ContextHandle.AddOrigin(GetActorLocation());

    const FGameplayEffectSpecHandle SpecHandle =
        SourceASC->MakeOutgoingSpec(
            KnockbackEffectClass,
            AbilityLevel,
            ContextHandle
        );

    if (!SpecHandle.IsValid() ||
        !SpecHandle.Data.IsValid())
    {
        UE_LOG(
            LogTemp,
            Error,
            TEXT("[Knockback] Failed to create GameplayEffectSpec.")
        );
        return;
    }

    UAbilitySystemBlueprintLibrary::AssignTagSetByCallerMagnitude(
        SpecHandle,
        FAuraGameplayTags::Get().Status_Knockback,
        EffectiveStrength
    );

    TargetASC->ApplyGameplayEffectSpecToSelf(
        *SpecHandle.Data.Get()
    );

    UE_LOG(
        LogTemp,
        Warning,
        TEXT("[Knockback] GE applied to %s | Strength=%.1f"),
        *GetNameSafe(OtherActor),
        EffectiveStrength
    );
}