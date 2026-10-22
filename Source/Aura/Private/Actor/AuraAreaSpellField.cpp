#include "Actor/AuraAreaSpellField.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystem/AuraAttributeSet.h"

#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/Character.h"


#include "Interaction/EnemyInterface.h"
#include "Materials/MaterialInterface.h"
#include "Materials/MaterialInstanceDynamic.h"

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

void AAuraAreaSpellField::OnAreaBeginOverlap(
    UPrimitiveComponent* OverlappedComponent,
    AActor* OtherActor,
    UPrimitiveComponent* OtherComp,
    int32 OtherBodyIndex,
    bool bFromSweep,
    const FHitResult& SweepResult)
{
    ApplyToActor(OtherActor);
}

void AAuraAreaSpellField::ApplyToActor(AActor* OtherActor)
{
    if (!HasAuthority() ||!IsValid(OtherActor) ||OtherActor == SourceActor)
    {
        return;
    }

    if (bAffectOnlyEnemies &&!OtherActor->GetClass()->ImplementsInterface(UEnemyInterface::StaticClass()))
    {
        return;
    }

    if (FVector::Dist2D(GetActorLocation(),OtherActor->GetActorLocation()) > AreaRadiusCm)
    {
        return;
    }

    const TWeakObjectPtr<AActor> WeakTarget(OtherActor);

    if (AffectedActors.Contains(WeakTarget))
    {
        return;
    }

    UAbilitySystemComponent* SourceASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(SourceActor);
    UAbilitySystemComponent* TargetASC =UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(OtherActor);

    if (!IsValid(SourceASC) || !IsValid(TargetASC) || !DamageEffectClass)
    {
        return;
    }

    FGameplayEffectContextHandle ContextHandle = SourceASC->MakeEffectContext();

    ContextHandle.SetAbility(SourceAbility);
    ContextHandle.AddSourceObject(SourceActor);

    const FGameplayEffectSpecHandle DamageSpecHandle =SourceASC->MakeOutgoingSpec(DamageEffectClass,AbilityLevel,ContextHandle);

    if (!DamageSpecHandle.IsValid() || !DamageSpecHandle.Data.IsValid())
    {
        return;
    }

    for (const TPair<FGameplayTag, float>& Pair : ScaledDamageValues)
    {
        UAbilitySystemBlueprintLibrary::AssignTagSetByCallerMagnitude(DamageSpecHandle,Pair.Key,Pair.Value);
    }

    AffectedActors.Add(WeakTarget);

    TargetASC->ApplyGameplayEffectSpecToSelf(*DamageSpecHandle.Data.Get()
    );

    for (const TSubclassOf<UGameplayEffect>& EffectClass : AdditionalEffectClasses)
    {
        if (!EffectClass)
        {
            continue;
        }

        FGameplayEffectContextHandle AdditionalContext = SourceASC->MakeEffectContext();

        AdditionalContext.SetAbility(SourceAbility);
        AdditionalContext.AddSourceObject(SourceActor);

        const FGameplayEffectSpecHandle AdditionalSpec = SourceASC->MakeOutgoingSpec(EffectClass,AbilityLevel,AdditionalContext);

        if (AdditionalSpec.IsValid() && AdditionalSpec.Data.IsValid())
        {
            TargetASC->ApplyGameplayEffectSpecToSelf(*AdditionalSpec.Data.Get()
            );
        }
    }

    if (bApplyKnockback)
    {
        if (ACharacter* TargetCharacter = Cast<ACharacter>(OtherActor))
        {
            FVector Direction = TargetCharacter->GetActorLocation() -GetActorLocation();

            Direction.Z = 0.f;
            Direction = Direction.GetSafeNormal();

            if (!Direction.IsNearlyZero())
            {
                const float SourceKnockback = SourceASC->GetNumericAttribute(UAuraAttributeSet::GetKnockbackAttribute());
                const float EffectiveStrength =KnockbackStrength *(1.f + FMath::Max(SourceKnockback, 0.f) * 0.01f);

                TargetCharacter->LaunchCharacter(Direction * EffectiveStrength +FVector(0.f, 0.f, EffectiveStrength * 0.2f),true,true
                );
            }
        }
    }
}