#include "Actor/AuraVolcanicArcsField.h"

#include "Magic/VolcanicArcsGeometry.h"
#include "Magic/MagicDebug.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystem/Abilities/AuraGameplayAbility.h"
#include "AuraGameplayTags.h"

#include "Components/SceneComponent.h"
#include "Components/SplineMeshComponent.h"
#include "Components/CapsuleComponent.h"

#include "Interaction/EnemyInterface.h"
#include "Net/UnrealNetwork.h"

AAuraVolcanicArcsField::AAuraVolcanicArcsField()
{
	PrimaryActorTick.bCanEverTick = false;

	bReplicates = true;
	SetReplicateMovement(false);

	USceneComponent* SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));

	SetRootComponent(SceneRoot);
}

void AAuraVolcanicArcsField::InitializeField(
	float InBaseHalfLengthCm,
	float InArcLengthScale,
	float InArcBowOffsetCm,
	const FVector& InIntersectionLocal,
	float InArcWidthCm,
	float InIntersectionRadiusCm,
	float InDurationSeconds,
	TSubclassOf<UGameplayEffect> InDamageEffectClass,
	const TMap<FGameplayTag, float>& InDamageValues,
	int32 InAbilityLevel,
	UGameplayAbility* InSourceAbility,
	AActor* InSourceActor)
{
	BaseHalfLengthCm = FMath::Max(InBaseHalfLengthCm, 1.f);
	
	ArcLengthScale = InArcLengthScale;
	ArcBowOffsetCm = FMath::Max(InArcBowOffsetCm, 0.f);
	InArcWidthCm = FMath::Max(InArcWidthCm, 1.f);
	
	IntersectionLocal = InIntersectionLocal;
	IntersectionRadiusCm = FMath::Max(InIntersectionRadiusCm, 1.f);

	DurationSeconds = FMath::Max(InDurationSeconds, 0.01f);

	DamageEffectClass = InDamageEffectClass;
	ScaledDamageValues = InDamageValues;

	AbilityLevel = FMath::Max(InAbilityLevel, 1);

	SourceAbility = InSourceAbility;
	SourceActor = InSourceActor;
}

void AAuraVolcanicArcsField::ClearVisuals()
{
	for (USplineMeshComponent* Component : ArcVisualSegments)
	{
		if (IsValid(Component))
		{
			Component->DestroyComponent();
		}
	}

	ArcVisualSegments.Reset();
}

void AAuraVolcanicArcsField::BuildVisualsForPath(const TArray<FVector>& Path)
{
	if (!ArcSegmentMesh || Path.Num() < 2)
	{
		return;
	}

	// Read the original mesh width on its local Y axis.
	const float MeshBaseWidthCm = ArcSegmentMesh->GetBounds().BoxExtent.Y * 2.f;
	// Convert desired world width into spline mesh cross-section scale.
	const float WidthScale = ArcWidthCm / FMath::Max(MeshBaseWidthCm, KINDA_SMALL_NUMBER);
	const FVector2D ArcCrossSectionScale(WidthScale,1.f);
	
	for (int32 Index = 0; Index < Path.Num() - 1; ++Index)
	{
		USplineMeshComponent* Segment = NewObject<USplineMeshComponent>(this);

		if (!IsValid(Segment))
		{
			continue;
		}

		Segment->SetMobility(EComponentMobility::Movable);
		Segment->SetupAttachment(GetRootComponent());

		Segment->SetStaticMesh(ArcSegmentMesh);
		Segment->SetForwardAxis(ESplineMeshAxis::X);

		Segment->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		Segment->SetCastShadow(false);

		Segment->SetStartScale(ArcCrossSectionScale);
		Segment->SetEndScale(ArcCrossSectionScale);

		if (ArcMaterial)
		{
			Segment->SetMaterial(0, ArcMaterial);
		}

		const FVector Start = Path[Index];
		const FVector End = Path[Index + 1];

		const FVector StartTangent = (Index == 0)
			? (Path[Index + 1] - Path[Index])
			: (Path[Index + 1] - Path[Index - 1]) * 0.5f;

		const FVector EndTangent = (Index + 2 >= Path.Num())
			? (Path[Index + 1] - Path[Index])
			: (Path[Index + 2] - Path[Index]) * 0.5f;

		Segment->SetStartAndEnd(Start,StartTangent,End,EndTangent,true);

		AddInstanceComponent(Segment);
		Segment->RegisterComponent();

		ArcVisualSegments.Add(Segment);
	}
}

void AAuraVolcanicArcsField::BuildVisuals()
{
	ClearVisuals();

	TArray<FVector> ArcA;
	TArray<FVector> ArcB;

	FVolcanicArcsGeometry::Build(IntersectionLocal,BaseHalfLengthCm,ArcLengthScale,ArcBowOffsetCm,ArcA,ArcB);

	BuildVisualsForPath(ArcA);
	BuildVisualsForPath(ArcB);
}

void AAuraVolcanicArcsField::ClearCollision()
{
	for (UCapsuleComponent* Capsule : ArcCollisionCapsules)
	{
		if (IsValid(Capsule))
		{
			Capsule->DestroyComponent();
		}
	}

	ArcCollisionCapsules.Reset();
}

void AAuraVolcanicArcsField::BuildCollisionForPath(
	const TArray<FVector>& Path)
{
	if (Path.Num() < 2)
	{
		return;
	}

	for (int32 Index = 0; Index < Path.Num() - 1; ++Index)
	{
		const FVector Start = Path[Index];
		const FVector End = Path[Index + 1];

		const FVector Segment = End - Start;
		const float SegmentLength = Segment.Size();
		const float CollisionRadiusCm = ArcWidthCm * 0.5f;
		
		if (SegmentLength <= KINDA_SMALL_NUMBER)
		{
			continue;
		}

		UCapsuleComponent* Capsule = NewObject<UCapsuleComponent>(this);

		if (!IsValid(Capsule))
		{
			continue;
		}

		Capsule->SetMobility(EComponentMobility::Movable);
		Capsule->SetupAttachment(GetRootComponent());
		Capsule->SetCapsuleSize(CollisionRadiusCm,SegmentLength * 0.5f + CollisionRadiusCm);
		Capsule->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
		Capsule->SetCollisionObjectType(ECC_WorldDynamic);
		Capsule->SetCollisionResponseToAllChannels(ECR_Ignore);
		Capsule->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
		Capsule->SetGenerateOverlapEvents(true);
		Capsule->OnComponentBeginOverlap.AddDynamic(this,&AAuraVolcanicArcsField::OnArcBeginOverlap);
		Capsule->SetRelativeLocation((Start + End) * 0.5f);

		const FVector Direction = Segment.GetSafeNormal();
		const FQuat Rotation = FQuat::FindBetweenNormals(FVector::UpVector,Direction);

		Capsule->SetRelativeRotation(Rotation);

		AddInstanceComponent(Capsule);
		Capsule->RegisterComponent();

		ArcCollisionCapsules.Add(Capsule);
	}
}

void AAuraVolcanicArcsField::BuildCollision()
{
	ClearCollision();

	TArray<FVector> ArcA;
	TArray<FVector> ArcB;

	FVolcanicArcsGeometry::Build(IntersectionLocal,BaseHalfLengthCm,ArcLengthScale,ArcBowOffsetCm,ArcA,ArcB);

	BuildCollisionForPath(ArcA);
	BuildCollisionForPath(ArcB);

	// Process actors already overlapping when the field appears.
	for (UCapsuleComponent* Capsule : ArcCollisionCapsules)
	{
		if (!IsValid(Capsule))
		{
			continue;
		}

		TArray<AActor*> OverlappingActors;
		Capsule->GetOverlappingActors(OverlappingActors);

		for (AActor* Actor : OverlappingActors)
		{
			ApplyDamageToActor(Actor);
		}
	}
}

void AAuraVolcanicArcsField::OnArcBeginOverlap(UPrimitiveComponent* OverlappedComponent,AActor* OtherActor,UPrimitiveComponent* OtherComp,int32 OtherBodyIndex,bool bFromSweep,const FHitResult& SweepResult)
{
	ApplyDamageToActor(OtherActor);
}

void AAuraVolcanicArcsField::ApplyDamageToActor(AActor* OtherActor)
{
	if (!HasAuthority() ||
		!IsValid(OtherActor) ||
		OtherActor == SourceActor)
	{
		return;
	}

	if (!OtherActor->GetClass()->ImplementsInterface(
		UEnemyInterface::StaticClass()))
	{
		return;
	}

	const TWeakObjectPtr<AActor> WeakTarget(OtherActor);

	if (AffectedActors.Contains(WeakTarget))
	{
		return;
	}

	UAbilitySystemComponent* SourceASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(SourceActor);
	UAbilitySystemComponent* TargetASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(OtherActor);

	if (!IsValid(SourceASC) ||
		!IsValid(TargetASC) ||
		!DamageEffectClass)
	{
		return;
	}

	FGameplayEffectContextHandle Context = SourceASC->MakeEffectContext();

	Context.SetAbility(SourceAbility);
	Context.AddSourceObject(SourceActor);

	FGameplayEffectSpecHandle Spec = SourceASC->MakeOutgoingSpec(DamageEffectClass,AbilityLevel,Context);

	if (!Spec.IsValid() || !Spec.Data.IsValid())
	{
		return;
	}

	const FVector IntersectionWorld =GetActorTransform().TransformPosition(IntersectionLocal);
	const bool bInsideIntersection =FVector::Dist2D(OtherActor->GetActorLocation(),IntersectionWorld) <= IntersectionRadiusCm;
	const FAuraGameplayTags& AuraTags = FAuraGameplayTags::Get();

	for (const TPair<FGameplayTag, float>& Pair : ScaledDamageValues)
	{
		const float Multiplier = (Pair.Key == AuraTags.Damage_Fire && bInsideIntersection)? 1.5f: 1.f;

		UAbilitySystemBlueprintLibrary::AssignTagSetByCallerMagnitude(Spec,Pair.Key,Pair.Value * Multiplier);
	}

	TargetASC->ApplyGameplayEffectSpecToSelf(*Spec.Data.Get()
	);

	AffectedActors.Add(WeakTarget);
}

void AAuraVolcanicArcsField::BeginPlay()
{
	Super::BeginPlay();

	BuildVisuals();

	if (HasAuthority())
	{
		SetLifeSpan(DurationSeconds);

		BuildCollision();
	}
}

void AAuraVolcanicArcsField::OnRep_ArcSettings()
{
	BuildVisuals();
}

void AAuraVolcanicArcsField::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME(AAuraVolcanicArcsField,BaseHalfLengthCm);
    DOREPLIFETIME(AAuraVolcanicArcsField,ArcLengthScale);
	DOREPLIFETIME(AAuraVolcanicArcsField,ArcBowOffsetCm);
	DOREPLIFETIME(AAuraVolcanicArcsField,IntersectionLocal);
	DOREPLIFETIME(AAuraVolcanicArcsField, ArcWidthCm);
}