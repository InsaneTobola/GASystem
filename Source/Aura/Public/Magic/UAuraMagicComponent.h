#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Magic/MagicTypes.h"
#include "UAuraMagicComponent.generated.h"

UCLASS(ClassGroup=(Magic), meta=(BlueprintSpawnableComponent))
class AURA_API UAuraMagicComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UAuraMagicComponent();
	virtual void BeginPlay() override;
	
	// Element
	void SelectElement(EMagicElement NewElement);
	EMagicElement GetSelectedElement() const;

	// Drawing
	void StartDrawing();
	bool StartStroke();
	bool AddStrokePoint(const FVector2D& NormalizedPoint);
	bool FinishStroke();
	FMagicValidationResult ConfirmGesture();
	void StopDrawing();
	void ResetMagic();
	void CancelMagic();

	// State
	EMagicState GetMagicState() const;

	bool IsDrawing() const;

	// Stroke
	bool IsStrokeActive() const;
	const FMagicStroke& GetCurrentStroke() const;
	const FMagicGesture& GetCurrentGesture() const;
	const FMagicGesture& GetLastValidatedGesture() const;
	const FMagicValidationResult& GetLastValidationResult() const;
	
private:
	
	void ResetStroke();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Magic", meta = (AllowPrivateAccess = "true"))
	EMagicElement SelectedElement = EMagicElement::Fire;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Magic", meta = (AllowPrivateAccess = "true"))
	EMagicState MagicState = EMagicState::Inactive;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Magic", meta = (AllowPrivateAccess = "true"))
	FMagicStroke CurrentStroke;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Magic", meta = (AllowPrivateAccess = "true"))
	FMagicGesture CurrentGesture;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Magic", meta = (AllowPrivateAccess = "true"))
	FMagicGesture LastValidatedGesture;
	
	UPROPERTY(VisibleAnywhere,BlueprintReadOnly,Category = "Magic",meta = (AllowPrivateAccess = "true"))
	FMagicValidationResult LastValidationResult;
	
	bool bIsStrokeActive = false;

	bool bSafetyLimitExceeded = false;
	
	
	
};