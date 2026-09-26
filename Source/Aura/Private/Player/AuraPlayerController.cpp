// Copyright Druid Mechanics


#include "Player/AuraPlayerController.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AuraGameplayTags.h"
#include "EnhancedInputSubsystems.h"
#include "AbilitySystem/AuraAbilitySystemComponent.h"
#include "Components/SplineComponent.h"
#include "Input/AuraInputComponent.h"
#include "Interaction/EnemyInterface.h"
#include "GameFramework/Character.h"
#include "UI/Widget/DamageTextComponent.h"
#include "Character/AuraCharacter.h"
#include "Magic/UAuraMagicComponent.h"
#include "UI/Widget/SUAuraMagicDrawingWidget.h"


AAuraPlayerController::AAuraPlayerController()
{
	bReplicates = true;
	Spline = CreateDefaultSubobject<USplineComponent>("Spline");
}

void AAuraPlayerController::PlayerTick(float DeltaTime)
{
	Super::PlayerTick(DeltaTime);
	if (bMagicDrawingMode)
	{
		return;
	}

	CursorTrace();
	AutoRun();
}

void AAuraPlayerController::StartMagicDrawingMode()
{
	UAuraMagicComponent* MagicComponent = GetMagicComponent();

	if (!MagicComponent || !MagicComponent->IsDrawing())
	{
		return;
	}

	if (!MagicDrawingWidgetClass)
	{
		UE_LOG(
			LogTemp,
			Error,
			TEXT("MagicDrawingWidgetClass is not set.")
		);

		return;
	}

	if (!MagicDrawingWidget)
	{
		MagicDrawingWidget = CreateWidget<UAuraMagicDrawingWidget>(this,MagicDrawingWidgetClass);

		if (MagicDrawingWidget)
		{
			MagicDrawingWidget->AddToViewport();
			MagicDrawingWidget->InitializeDrawing(GetMagicComponent());
		}
	}

	MagicDrawingWidget->InitializeDrawing(MagicComponent);

	if (!MagicDrawingWidget->IsInViewport())
	{
		MagicDrawingWidget->AddToViewport(100);
	}

	MagicDrawingWidget->SetVisibility(ESlateVisibility::Visible);

	bMagicDrawingMode = true;

	bShowMouseCursor = true;

	FInputModeUIOnly InputModeData;

	InputModeData.SetWidgetToFocus(MagicDrawingWidget->TakeWidget());

	SetInputMode(InputModeData);
}

void AAuraPlayerController::StopMagicDrawingMode()
{
	bMagicDrawingMode = false;

	if (MagicDrawingWidget)
	{
		MagicDrawingWidget->RemoveFromParent();
		MagicDrawingWidget = nullptr;
	}

	FInputModeGameAndUI InputModeData;

	InputModeData.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);

	InputModeData.SetHideCursorDuringCapture(false);

	SetInputMode(InputModeData);

	bShowMouseCursor = true;
}

void AAuraPlayerController::ConfirmMagicDrawingMode()
{
	UAuraMagicComponent* MagicComponent =GetMagicComponent();
	if (!MagicComponent)
	{
		return;
	}

	const FMagicValidationResult Result =MagicComponent->ConfirmGesture();
	if (!Result.bIsValid)
	{
		UE_LOG(
			LogTemp,
			Warning,
			TEXT("Magic Gesture rejected. Reason: %d"),
			static_cast<int32>(Result.FailureReason)
		);

		return;
	}

	UE_LOG(
		LogTemp,
		Log,
		TEXT(
			"Magic Gesture accepted. Strokes: %d | Points: %d | Path: %.3f | Aspect: %.3f"
		),
		Result.StrokeCount,
		Result.TotalPointCount,
		Result.TotalPathLength,
		Result.AspectRatio
	);
	
	StopMagicDrawingMode();
}

UAuraMagicComponent* AAuraPlayerController::GetMagicComponent() const
{
	const AAuraCharacter* AuraCharacter = Cast<AAuraCharacter>(GetPawn());

	if (!AuraCharacter)
	{
		return nullptr;
	}

	return AuraCharacter->MagicComponent;
}

void AAuraPlayerController::MagicInputPressed(FGameplayTag InputTag)
{
	UAuraMagicComponent* MagicComponent = GetMagicComponent();

	if (!MagicComponent)
	{
		return;
	}

	const FAuraGameplayTags& GameplayTags = FAuraGameplayTags::Get();

	if (InputTag.MatchesTagExact(GameplayTags.InputTag_1))
	{
		MagicComponent->SelectElement(EMagicElement::Fire);
	}
	else if (InputTag.MatchesTagExact(GameplayTags.InputTag_2))
	{
		MagicComponent->SelectElement(EMagicElement::Water);
	}
	else if (InputTag.MatchesTagExact(GameplayTags.InputTag_3))
	{
		MagicComponent->SelectElement(EMagicElement::Earth);
	}
	else if (InputTag.MatchesTagExact(GameplayTags.InputTag_4))
	{
		MagicComponent->SelectElement(EMagicElement::Air);
	}
	else
	{
		return;
	}

	StartMagicDrawingMode();
}

void AAuraPlayerController::HandleMagicInput(const FGameplayTag& InputTag)
{
	
}

void AAuraPlayerController::ShowDamageNumber_Implementation(float DamageAmount, ACharacter* TargetCharacter, bool bBlockedHit, bool bCriticalHit)
{
	if (IsValid(TargetCharacter) && DamageTextComponentClass)
	{
		UDamageTextComponent* DamageText = NewObject<UDamageTextComponent>(TargetCharacter, DamageTextComponentClass);
		DamageText->RegisterComponent();
		DamageText->AttachToComponent(TargetCharacter->GetRootComponent(), FAttachmentTransformRules::KeepRelativeTransform);
		DamageText->DetachFromComponent(FDetachmentTransformRules::KeepWorldTransform);
		DamageText->SetDamageText(DamageAmount, bBlockedHit, bCriticalHit);
	}
}

void AAuraPlayerController::AutoRun()
{
	if (!bAutoRunning) return;
	if (APawn* ControlledPawn = GetPawn())
	{
		const FVector LocationOnSpline = Spline->FindLocationClosestToWorldLocation(ControlledPawn->GetActorLocation(), ESplineCoordinateSpace::World);
		const FVector Direction = Spline->FindDirectionClosestToWorldLocation(LocationOnSpline, ESplineCoordinateSpace::World);
		ControlledPawn->AddMovementInput(Direction);
		
		const float DistanceToDestination = (LocationOnSpline - CachedDestination).Length();
		if (DistanceToDestination <= AutoRunAcceptanceRadius)
		{
			bAutoRunning = false;
		}
	}
}


void AAuraPlayerController::CursorTrace()
{
	GetHitResultUnderCursor(ECC_Visibility, false, CursorHit);
	if (!CursorHit.bBlockingHit) return;

	LastActor = ThisActor;
	ThisActor = Cast<IEnemyInterface>(CursorHit.GetActor());

	if (LastActor != ThisActor)
	{
		if (LastActor)LastActor->UnHighlightActor();
		if (ThisActor)ThisActor->HighlightActor();
	}
}

void AAuraPlayerController::AbilityInputTagPressed(FGameplayTag InputTag)
{
	const FAuraGameplayTags& GameplayTags = FAuraGameplayTags::Get();

	if (InputTag.MatchesTagExact(GameplayTags.InputTag_1) ||
		InputTag.MatchesTagExact(GameplayTags.InputTag_2) ||
		InputTag.MatchesTagExact(GameplayTags.InputTag_3) ||
		InputTag.MatchesTagExact(GameplayTags.InputTag_4))
	{
		HandleMagicInput(InputTag);
		return;
	}
	
	if (bMagicDrawingMode)
	{
		return;
	}
	
	if (InputTag.MatchesTagExact(FAuraGameplayTags::Get().InputTag_RMB))
	{
		bTargeting = ThisActor ? true : false;
		bAutoRunning = false;
	}
}

void AAuraPlayerController::AbilityInputTagReleased(FGameplayTag InputTag)
{
	if (!InputTag.MatchesTagExact(FAuraGameplayTags::Get().InputTag_RMB))
	{
		if (GetASC())
		{
			GetASC()->AbilityInputTagReleased(InputTag);
		}
		return;
	}

	if (bTargeting)
	{
		if (GetASC())
		{
			GetASC()->AbilityInputTagReleased(InputTag);
		}
	}
	/*else
	{
		const APawn* ControlledPawn = GetPawn();
		if (FollowTime <= ShortPressThreshold && ControlledPawn)
		{
			if (UNavigationPath* NavPath = UNavigationSystemV1::FindPathToLocationSynchronously(this, ControlledPawn->GetActorLocation(), CachedDestination))
	
			{
				if (NavPath->PathPoints.Num() > 0)
				{
					Spline->ClearSplinePoints();
					for (const FVector& PointLoc: NavPath->PathPoints)
					{
						Spline->AddSplinePoint(PointLoc, ESplineCoordinateSpace::World);
						DrawDebugSphere(GetWorld(), PointLoc, 8.f, 8, FColor::Green, false, 5.f);
					}
					CachedDestination = NavPath->PathPoints[NavPath->PathPoints.Num() - 1];
					bAutoRunning = true;
				}
			}
		}*/
		FollowTime = 0.f;
		bTargeting = false;
	
}

void AAuraPlayerController::AbilityInputTagHeld(FGameplayTag InputTag)
{
	if (GetASC() == nullptr)
	{
		return;
	}

	if (!InputTag.MatchesTagExact(FAuraGameplayTags::Get().InputTag_RMB))
	{
		GetASC()->AbilityInputTagHeld(InputTag);
		return;
	}

	if (bTargeting)
	{
		GetASC()->AbilityInputTagHeld(InputTag);
	}
	
	/*else
	{
		FollowTime += GetWorld()->GetDeltaSeconds();
		if (CursorHit.bBlockingHit) CachedDestination = CursorHit.ImpactPoint;
		if (APawn* ControlledPawn = GetPawn())
		{
			const FVector WorldDirection = (CachedDestination - ControlledPawn->GetActorLocation()).GetSafeNormal();
			ControlledPawn->AddMovementInput(WorldDirection);
		}
	}*/
}

UAuraAbilitySystemComponent* AAuraPlayerController::GetASC()
{
	if (AuraAbilitySystemComponent == nullptr)
	{
		AuraAbilitySystemComponent = Cast<UAuraAbilitySystemComponent>(UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(GetPawn<APawn>()));
	}
	return AuraAbilitySystemComponent;
}



void AAuraPlayerController::BeginPlay()
{
	Super::BeginPlay();
	check(AuraContext);
	UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer());
	if (Subsystem)
	{
		Subsystem->AddMappingContext(AuraContext, 0);
	}
	bShowMouseCursor = true;
	DefaultMouseCursor = EMouseCursor::Default;
	FInputModeGameAndUI InputModeData;
	InputModeData.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
	InputModeData.SetHideCursorDuringCapture(false);
	SetInputMode(InputModeData);
}

void AAuraPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();
	UAuraInputComponent* AuraInputComponent = CastChecked<UAuraInputComponent>(InputComponent);
	AuraInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &AAuraPlayerController::Move);
	AuraInputComponent->BindAbilityActions(InputConfig, this, &ThisClass::AbilityInputTagPressed, &ThisClass::AbilityInputTagReleased, &ThisClass::AbilityInputTagHeld);
	AuraInputComponent->BindMagicActions(InputConfig,this,&ThisClass::MagicInputPressed
 );
}

void AAuraPlayerController::Move(const FInputActionValue& InputActionValue)
{
	if (bMagicDrawingMode)
	{
		return;
	}

	const FVector2D InputAxisVector =InputActionValue.Get<FVector2D>();
	const FRotator Rotation = GetControlRotation();
	const FRotator YawRotation(0.f, Rotation.Yaw, 0.f);
	const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
	const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

	if (APawn* ControlledPawn = GetPawn<APawn>())
	{
		ControlledPawn->AddMovementInput(ForwardDirection, InputAxisVector.Y);
		ControlledPawn->AddMovementInput(RightDirection, InputAxisVector.X);
	}
}