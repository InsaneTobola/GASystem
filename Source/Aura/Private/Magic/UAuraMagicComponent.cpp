#include "Magic/UAuraMagicComponent.h"
#include "HAL/PlatformTime.h"
#include "Magic/MagicGestureValidator.h"
#include "Magic/MagicValidationSettings.h"

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
	CurrentGesture.Reset();CurrentStroke.Reset();
	LastValidatedGesture.Reset();
    
	bIsStrokeActive = false;
	bSafetyLimitExceeded = false;
    
	CurrentGesture.StartTime =static_cast<float>(FPlatformTime::Seconds());
    
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

	CurrentStroke.StartTime =static_cast<float>(FPlatformTime::Seconds());

	bIsStrokeActive = true;

	return true;
}

bool UAuraMagicComponent::FinishStroke()
{
	if (!bIsStrokeActive)
	{
		return false;
	}

	CurrentStroke.EndTime =static_cast<float>(FPlatformTime::Seconds());
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
		FMagicValidationResult Result;
		Result.FailureReason = EMagicValidationFailure::NoStrokes;
		return Result;
	}

	if (bIsStrokeActive)
	{
		FinishStroke();
	}
	CurrentGesture.EndTime = static_cast<float>(FPlatformTime::Seconds());

	if (bSafetyLimitExceeded)
	{
		FMagicValidationResult Result;

		Result.StrokeCount = CurrentGesture.GetStrokeCount();
		Result.TotalPointCount = CurrentGesture.GetTotalPointCount();
		Result.GestureDuration = CurrentGesture.GetDuration();
		Result.FailureReason = EMagicValidationFailure::SafetyLimitExceeded;
		return Result;
	}

	const UMagicValidationSettings* Settings =
		GetDefault<UMagicValidationSettings>();

	if (!Settings)
	{
		FMagicValidationResult Result;Result.FailureReason =EMagicValidationFailure::SafetyLimitExceeded;
		return Result;
	}
	FMagicValidationResult Result =FMagicGestureValidator::Validate(CurrentGesture,*Settings);

	if (Result.bIsValid)
	{
		LastValidatedGesture = CurrentGesture;
	}
	return Result;
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

bool UAuraMagicComponent::AddStrokePoint(const FVector2D& NormalizedPoint)
{
	if (!IsDrawing() || !bIsStrokeActive)
	{
		return false;
	}

	const UMagicValidationSettings* Settings =GetDefault<UMagicValidationSettings>();

	if (!Settings)
	{
		return false;
	}

	if (bSafetyLimitExceeded)
	{
		return false;
	}

	if (CurrentStroke.Points.Num() >=Settings->MaxPointsPerStroke)
	{
		bSafetyLimitExceeded = true;
		return false;
	}

	if (CurrentGesture.GetTotalPointCount() +CurrentStroke.Points.Num() >=Settings->MaxTotalPointCount)
	{
		bSafetyLimitExceeded = true;
		return false;
	}

	const FVector2D ClampedPoint(FMath::Clamp(NormalizedPoint.X, 0.0f, 1.0f),FMath::Clamp(NormalizedPoint.Y, 0.0f, 1.0f));

	if (CurrentStroke.Points.Num() > 0)
	{
		const FVector2D& LastPoint =CurrentStroke.Points.Last();

		if (FVector2D::Distance(LastPoint,ClampedPoint)< Settings->MinPointDistance)
		{
			return false;
		}
	}
	CurrentStroke.Points.Add(ClampedPoint);
	return true;
}

void UAuraMagicComponent::StopDrawing()
{
	bIsStrokeActive = false;
	CurrentGesture.EndTime =static_cast<float>(FPlatformTime::Seconds());
	MagicState = EMagicState::Inactive;
}

void UAuraMagicComponent::CancelMagic()
{
	CurrentGesture.Reset();
	CurrentStroke.Reset();
	LastValidatedGesture.Reset();

	bIsStrokeActive = false;
	bSafetyLimitExceeded = false;

	MagicState = EMagicState::Inactive;
}

EMagicState UAuraMagicComponent::GetMagicState() const
{
	return MagicState;
}
