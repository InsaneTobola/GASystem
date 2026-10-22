#include "Magic/UAuraMagicComponent.h"
#include "Magic/MagicGestureValidator.h"
#include "Magic/MagicValidationSettings.h"
#include "Magic/MagicPointCloudRecognizer.h"
#include "Magic/MagicPatternDefinition.h"
#include "HAL/PlatformTime.h"
#include "AbilitySystemComponent.h"
#include "GameplayAbilitySpec.h"

UAuraMagicComponent::UAuraMagicComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UAuraMagicComponent::BeginPlay()
{
	Super::BeginPlay();
	PrimaryComponentTick.bCanEverTick = false;
}

void UAuraMagicComponent::ResetStroke()
{
	CurrentStroke.Reset();
}

void UAuraMagicComponent::SelectElement(EMagicElement NewElement)
{
	SelectedElement = NewElement;
	StartDrawing();
}

EMagicElement UAuraMagicComponent::GetSelectedElement() const
{
	return SelectedElement;
}

void UAuraMagicComponent::StartDrawing()
{
	CurrentGesture.Reset();
	CurrentStroke.Reset();
	LastValidatedGesture.Reset();
	LastValidationResult = FMagicValidationResult();

	bIsStrokeActive = false;
	bSafetyLimitExceeded = false;

	CurrentGesture.StartTime = static_cast<float>(FPlatformTime::Seconds());

	MagicState = EMagicState::Drawing;
}

bool UAuraMagicComponent::StartStroke()
{
	if (!IsDrawing())
	{
		return false;
	}

	if (bIsStrokeActive)
	{
		return false;
	}

	const UMagicValidationSettings* Settings = GetDefault<UMagicValidationSettings>();

	if (!Settings)
	{
		return false;
	}

	if (CurrentGesture.Strokes.Num() >= Settings->MaxStrokeCount)
	{
		bSafetyLimitExceeded = true;
		return false;
	}

	ResetStroke();

	CurrentStroke.StartTime = static_cast<float>(FPlatformTime::Seconds());

	bIsStrokeActive = true;

	return true;
}

bool UAuraMagicComponent::FinishStroke()
{
	if (!bIsStrokeActive)
	{
		return false;
	}

	CurrentStroke.EndTime = static_cast<float>(FPlatformTime::Seconds());
	bIsStrokeActive = false;

	if (CurrentStroke.Points.Num() == 0)
	{
		ResetStroke();
		return false;
	}

	CurrentGesture.Strokes.Add(CurrentStroke);
	ResetStroke();
	return true;
}

FMagicValidationResult UAuraMagicComponent::ConfirmGesture()
{
	if (!IsDrawing())
	{
		LastValidationResult = FMagicValidationResult();

		LastValidationResult.FailureReason = EMagicValidationFailure::NoStrokes;

		return LastValidationResult;
	}

	if (bIsStrokeActive)
	{
		FinishStroke();
	}

	CurrentGesture.EndTime = static_cast<float>(FPlatformTime::Seconds());

	if (bSafetyLimitExceeded)
	{
		LastValidationResult=FMagicValidationResult();
		LastValidationResult.StrokeCount = CurrentGesture.GetStrokeCount();
		LastValidationResult.TotalPointCount = CurrentGesture.GetTotalPointCount();
		LastValidationResult.GestureDuration = CurrentGesture.GetDuration();
		LastValidationResult.FailureReason = EMagicValidationFailure::SafetyLimitExceeded;

		return LastValidationResult;
	}

	const UMagicValidationSettings* Settings =GetDefault<UMagicValidationSettings>();

	if (!Settings)
	{
		LastValidationResult =FMagicValidationResult();

		LastValidationResult.FailureReason = EMagicValidationFailure::SafetyLimitExceeded;

		return LastValidationResult;
	}

	LastValidationResult =FMagicGestureValidator::Validate(CurrentGesture,*Settings);

	if (LastValidationResult.bIsValid)
	{
		LastValidatedGesture = CurrentGesture;
	}
	
	return LastValidationResult;
}

bool UAuraMagicComponent::IsDrawing() const
{
	return MagicState == EMagicState::Drawing;
}

bool UAuraMagicComponent::IsStrokeActive() const
{
	return bIsStrokeActive;
}

const FMagicStroke& UAuraMagicComponent::GetCurrentStroke() const
{
	return CurrentStroke;
}

const FMagicGesture& UAuraMagicComponent::GetCurrentGesture() const
{
	return CurrentGesture;
}

const FMagicGesture& UAuraMagicComponent::GetLastValidatedGesture() const
{
	return LastValidatedGesture;
}

const FMagicValidationResult& UAuraMagicComponent::GetLastValidationResult() const
{
	return LastValidationResult;
}

FMagicRecognitionResult UAuraMagicComponent::RecognizeCurrentGesture() const
{
	FMagicRecognitionResult BestResult;
	const FMagicGesture& Gesture = GetCurrentGesture();

	if (Gesture.GetTotalPointCount() < 2)
	{
		return BestResult;
	}

	FMagicPointCloud Candidate;

	if (!FMagicPointCloudRecognizer::BuildPointCloud(Gesture, Candidate))
	{
		return BestResult;
	}

	for (const UMagicPatternDefinition* Pattern : PatternDefinitions)
	{
		if (!IsValid(Pattern))
		{
			continue;
		}
		
		if (Pattern->Element != SelectedElement)
		{
			continue;
		}
		
		if (!FMagicGestureValidator::ValidateAgainstPatternRules(Gesture,Pattern->ValidationRules))
		{
			continue;
		}
		
		TArray<FMagicPointCloudTemplate> RuntimeTemplates;
		Pattern->BuildRuntimeTemplates(RuntimeTemplates);

		if (RuntimeTemplates.Num() == 0)
		{
			continue;
		}
		const FMagicRecognitionResult PatternResult = FMagicPointCloudRecognizer::Recognize(Candidate,RuntimeTemplates,Pattern->MinimumSimilarityScore);

		if (!PatternResult.bMatchFound)
		{
			continue;
		}
		if (!BestResult.bMatchFound || PatternResult.SimilarityScore > BestResult.SimilarityScore)
		{
			BestResult = PatternResult;
		}
	}

	return BestResult;
}

UMagicPatternDefinition* UAuraMagicComponent::FindPatternDefinition(FName PatternId) const
{
	if (PatternId.IsNone())
	{
		return nullptr;
	}

	for (UMagicPatternDefinition* Pattern : PatternDefinitions)
	{
		if (IsValid(Pattern) && Pattern->PatternId == PatternId)
		{
			return Pattern;
		}
	}

	return nullptr;
}

void UAuraMagicComponent::GrantPatternAbilities(UAbilitySystemComponent* ASC)
{
	if (!IsValid(ASC))
	{
		return;
	}

	AActor* Owner = GetOwner();

	if (!IsValid(Owner) || !Owner->HasAuthority())
	{
		return;
	}

	for (const UMagicPatternDefinition* Pattern : PatternDefinitions)
	{
		if (!IsValid(Pattern) || !Pattern->SkillAbilityClass)
		{
			continue;
		}

		if (ASC->FindAbilitySpecFromClass(Pattern->SkillAbilityClass))
		{
			continue;
		}

		FGameplayAbilitySpec AbilitySpec(
			Pattern->SkillAbilityClass,
			1
		);

		ASC->GiveAbility(AbilitySpec);
	}
}

bool UAuraMagicComponent::BeginTargeting(UMagicPatternDefinition* Pattern)
{
	if (!IsValid(Pattern) || Pattern->Element != SelectedElement || !Pattern->SkillAbilityClass)
	{
		return false;
	}

	SelectedPattern = Pattern;

	bIsStrokeActive = false;
	MagicState = EMagicState::Targeting;

	return true;
}

UMagicPatternDefinition* UAuraMagicComponent::GetSelectedPattern() const
{
	return SelectedPattern;
}

bool UAuraMagicComponent::IsTargeting() const
{
	return MagicState == EMagicState::Targeting;
}

void UAuraMagicComponent::SetPendingCast(FName PatternId, const FVector& TargetLocation)
{
	PendingPatternId = PatternId;
	PendingTargetLocation = TargetLocation;
	bHasPendingCast = true;
}

bool UAuraMagicComponent::ConsumePendingCast(FName& OutPatternId, FVector& OutTargetLocation)
{
	if (!bHasPendingCast || PendingPatternId.IsNone())
	{
		return false;
	}

	OutPatternId = PendingPatternId;
	OutTargetLocation = PendingTargetLocation;

	ClearPendingCast();

	return true;
}

void UAuraMagicComponent::ClearPendingCast()
{
	PendingPatternId = NAME_None;
	PendingTargetLocation = FVector::ZeroVector;
	bHasPendingCast = false;
}

bool UAuraMagicComponent::AddStrokePoint(const FVector2D& NormalizedPoint)
{
	if (!IsDrawing() || !bIsStrokeActive)
	{
		return false;
	}

	const UMagicValidationSettings* Settings = GetDefault<UMagicValidationSettings>();

	if (!Settings)
	{
		return false;
	}

	if (bSafetyLimitExceeded)
	{
		return false;
	}

	// Max points in current stroke
	if (CurrentStroke.Points.Num() >= Settings->MaxPointsPerStroke)
	{
		bSafetyLimitExceeded = true;
		return false;
	}

	// Max points in entire gesture
	if (CurrentGesture.GetTotalPointCount() + CurrentStroke.Points.Num() + 1 >Settings->MaxTotalPointCount)
	{
		bSafetyLimitExceeded = true;
		return false;
	}

	const FVector2D ClampedPoint(FMath::Clamp(NormalizedPoint.X,0.0f,1.0f),
		FMath::Clamp(NormalizedPoint.Y,0.0f,1.0f)
	);

	if (CurrentStroke.Points.Num() > 0)
	{
		const FVector2D& LastPoint = CurrentStroke.Points.Last();

		if (FVector2D::Distance(LastPoint,ClampedPoint)< Settings->MinPointDistance)
		{
			return false;
		}
	}

	CurrentStroke.Points.Add(ClampedPoint
	);

	return true;
}

void UAuraMagicComponent::StopDrawing()
{
	bIsStrokeActive = false;
	CurrentGesture.EndTime = static_cast<float>(FPlatformTime::Seconds());
	MagicState = EMagicState::Inactive;
}

void UAuraMagicComponent::ResetMagic()
{
	CurrentGesture.Reset();
	CurrentStroke.Reset();
	LastValidatedGesture.Reset();
	LastValidationResult = FMagicValidationResult();

	bIsStrokeActive = false;
	bSafetyLimitExceeded = false;

	CurrentGesture.StartTime = static_cast<float>(FPlatformTime::Seconds());

	MagicState = EMagicState::Drawing;
}

void UAuraMagicComponent::CancelMagic()
{
	CurrentGesture.Reset();
	CurrentStroke.Reset();
	LastValidatedGesture.Reset();

	bIsStrokeActive = false;
	bSafetyLimitExceeded = false;
	
	SelectedPattern = nullptr;
	ClearPendingCast();
	
	MagicState = EMagicState::Inactive;
}

EMagicState UAuraMagicComponent::GetMagicState() const
{
	return MagicState;
}
