
#include "Player/AuraPlayerController.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystem/AuraAbilitySystemComponent.h"
#include "AbilitySystem/Abilities/AuraAreaSpell.h"
#include "AbilitySystem/AuraAttributeSet.h"
#include "AuraGameplayTags.h"

#include "Character/AuraCharacter.h"
#include "Components/SplineComponent.h"
#include "CollisionQueryParams.h"

#include "EnhancedInputSubsystems.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Engine/EngineTypes.h"

#include "GameFramework/Character.h"

#include "Input/AuraInputComponent.h"
#include "Interaction/EnemyInterface.h"

#include "Magic/UAuraMagicComponent.h"
#include "Magic/MagicPointCloudRecognizer.h"
#include "Magic/MagicPatternDefinition.h"
#include "Magic/MagicTargetingActor.h"

#include "UI/Widget/DamageTextComponent.h"
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

	//InputModeData.SetWidgetToFocus(MagicDrawingWidget->TakeWidget());

	SetInputMode(InputModeData);
}


void AAuraPlayerController::StopMagicDrawingMode()
{
	if (UAuraMagicComponent* MagicComponent = GetMagicComponent())
	{
		if (MagicComponent->IsDrawing())
		{
			MagicComponent->StopDrawing();
		}
	}
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
	UAuraMagicComponent* MagicComponent = GetMagicComponent();

	if (!IsValid(MagicComponent))
	{
		StopMagicDrawingMode();
		return;
	}

	const FMagicValidationResult Validation = MagicComponent->ConfirmGesture();

	if (!Validation.bIsValid)
	{
		StopMagicDrawingMode();
		MagicComponent->CancelMagic();
		return;
	}

	const FMagicRecognitionResult Recognition = MagicComponent->RecognizeCurrentGesture();

	if (!Recognition.bMatchFound)
	{
		StopMagicDrawingMode();
		MagicComponent->CancelMagic();
		return;
	}

	UMagicPatternDefinition* Pattern = MagicComponent->FindPatternDefinition(Recognition.PatternId);

	if (!IsValid(Pattern) || Pattern->Element != MagicComponent->GetSelectedElement() || !Pattern->SkillAbilityClass)
	{
		StopMagicDrawingMode();
		MagicComponent->CancelMagic();
		return;
	}

	const UAuraAreaSpell* SpellDefaults = Cast<UAuraAreaSpell>(Pattern->SkillAbilityClass.GetDefaultObject());

	if (!SpellDefaults)
	{
		UE_LOG(
			LogTemp,
			Error,
			TEXT("Magic pattern is not assigned to a UAuraAreaSpell.")
		);

		StopMagicDrawingMode();
		MagicComponent->CancelMagic();
		return;
	}

	StopMagicDrawingMode();

	if (!MagicComponent->BeginTargeting(Pattern))
	{
		MagicComponent->CancelMagic();
		return;
	}

	StartMagicTargeting(Pattern);
}

void AAuraPlayerController::StartMagicTargeting(UMagicPatternDefinition* Pattern)
{
	UAuraMagicComponent* MagicComponent = GetMagicComponent();
	UAbilitySystemComponent* ASC = GetASC();

	if (!IsValid(Pattern) || !IsValid(MagicComponent) || !IsValid(ASC) || !MagicTargetingActorClass)
	{
		CancelMagicTargeting();
		return;
	}

	const UAuraAreaSpell* SpellDefaults = Cast<UAuraAreaSpell>(Pattern->SkillAbilityClass.GetDefaultObject());

	if (!SpellDefaults)
	{
		CancelMagicTargeting();
		return;
	}

	if (IsValid(MagicTargetingActor))
	{
		MagicTargetingActor->Destroy();
		MagicTargetingActor = nullptr;
	}

	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = this;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	MagicTargetingActor = GetWorld()->SpawnActor<AAuraMagicTargetingActor>(MagicTargetingActorClass,FVector::ZeroVector,FRotator::ZeroRotator,SpawnParams);

	if (!IsValid(MagicTargetingActor))
	{
		CancelMagicTargeting();
		return;
	}

	MagicTargetingActor->InitializeTargeting(this,GetPawn(),SpellDefaults->GetMaxRangeCm(),SpellDefaults->CalculateAreaRadius(ASC));

	PendingMagicPatternId = Pattern->PatternId;
	PendingMagicElement = Pattern->Element;

	bMagicTargetingMode = true;
	bWaitingForMagicCastResponse = false;

	bShowMouseCursor = true;

	FInputModeGameAndUI InputModeData;
	InputModeData.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
	InputModeData.SetHideCursorDuringCapture(false);

	SetInputMode(InputModeData);
}

void AAuraPlayerController::CancelMagicTargeting()
{
	if (IsValid(MagicTargetingActor))
	{
		MagicTargetingActor->Destroy();
		MagicTargetingActor = nullptr;
	}

	bMagicTargetingMode = false;
	bWaitingForMagicCastResponse = false;

	PendingMagicPatternId = NAME_None;

	if (UAuraMagicComponent* MagicComponent = GetMagicComponent())
	{
		if (MagicComponent->IsTargeting())
		{
			MagicComponent->CancelMagic();
		}
	}

	bShowMouseCursor = true;

	FInputModeGameAndUI InputModeData;
	InputModeData.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
	InputModeData.SetHideCursorDuringCapture(false);

	SetInputMode(InputModeData);
}

void AAuraPlayerController::ConfirmMagicTargeting()
{
	if (!bMagicTargetingMode || bWaitingForMagicCastResponse || !IsValid(MagicTargetingActor))
	{
		return;
	}

	if (!MagicTargetingActor->HasValidTarget())
	{
		return;
	}
	bWaitingForMagicCastResponse = true;

	ServerConfirmMagicTarget(PendingMagicPatternId,PendingMagicElement,MagicTargetingActor->GetTargetLocation());
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
	if (bWaitingForMagicCastResponse)
	{
		return;
	}

	if (bMagicTargetingMode)
	{
		CancelMagicTargeting();
	}
	
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

void AAuraPlayerController::ServerConfirmMagicTarget_Implementation(FName PatternId, EMagicElement Element, FVector TargetLocation)
{
	 AAuraCharacter* AuraCharacter = Cast<AAuraCharacter>(GetPawn());

    if (!IsValid(AuraCharacter) || !IsValid(AuraCharacter->MagicComponent))
    {
        ClientMagicTargetRejected();
        return;
    }

    UAbilitySystemComponent* ASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(AuraCharacter);

    if (!IsValid(ASC))
    {
        ClientMagicTargetRejected();
        return;
    }

    UMagicPatternDefinition* Pattern = AuraCharacter->MagicComponent->FindPatternDefinition(PatternId);

    if (!IsValid(Pattern) || Pattern->Element != Element || !Pattern->SkillAbilityClass)
    {
        ClientMagicTargetRejected();
        return;
    }

    const UAuraAreaSpell* SpellDefaults = Cast<UAuraAreaSpell>(Pattern->SkillAbilityClass.GetDefaultObject());

    if (!SpellDefaults || !ASC->FindAbilitySpecFromClass(Pattern->SkillAbilityClass))
    {
        ClientMagicTargetRejected();
        return;
    }

    FHitResult GroundHit;
    FCollisionQueryParams QueryParams(SCENE_QUERY_STAT(MagicTargetValidation),true);
    QueryParams.AddIgnoredActor(AuraCharacter);

    const FVector TraceStart = TargetLocation + FVector(0.f, 0.f, 5000.f);
    const FVector TraceEnd = TargetLocation - FVector(0.f, 0.f, 5000.f);

    if (!GetWorld()->LineTraceSingleByChannel(GroundHit,TraceStart,TraceEnd,ECC_WorldStatic,QueryParams))
    {
        ClientMagicTargetRejected();
        return;
    }

    const FVector ValidatedLocation = GroundHit.Location;
    const float Distance2D = FVector::Dist2D(AuraCharacter->GetActorLocation(),ValidatedLocation);

    if (Distance2D > SpellDefaults->GetMaxRangeCm())
    {
        ClientMagicTargetRejected();
        return;
    }

    AuraCharacter->MagicComponent->SetPendingCast(
        PatternId,
        ValidatedLocation
    );

    if (!ASC->TryActivateAbilityByClass(Pattern->SkillAbilityClass))
    {
        AuraCharacter->MagicComponent->ClearPendingCast();
        ClientMagicTargetRejected();
        return;
    }

    ClientMagicTargetAccepted();
}

void AAuraPlayerController::ClientMagicTargetAccepted_Implementation()
{
	if (IsValid(MagicTargetingActor))
	{
		MagicTargetingActor->Destroy();
		MagicTargetingActor = nullptr;
	}

	bMagicTargetingMode = false;
	bWaitingForMagicCastResponse = false;

	PendingMagicPatternId = NAME_None;

	if (UAuraMagicComponent* MagicComponent = GetMagicComponent())
	{
		MagicComponent->CancelMagic();
	}
}

void AAuraPlayerController::ClientMagicTargetRejected_Implementation()
{
	bWaitingForMagicCastResponse = false;

	UE_LOG(
		LogTemp,
		Warning,
		TEXT("Magic target was rejected by the server.")
	);
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
	if (bMagicTargetingMode)
	{
		if (bWaitingForMagicCastResponse)
		{
			return;
		}

		const FAuraGameplayTags& MagicTags = FAuraGameplayTags::Get();

		if (InputTag.MatchesTagExact(MagicTags.InputTag_LMB))
		{
			ConfirmMagicTargeting();
			return;
		}

		if (InputTag.MatchesTagExact(MagicTags.InputTag_RMB))
		{
			CancelMagicTargeting();
			return;
		}

		return;
	}

	if (bMagicDrawingMode)
	{
		return;
	}
	const FAuraGameplayTags& MagicGameplayTags = FAuraGameplayTags::Get();

	if (InputTag.MatchesTagExact(MagicGameplayTags.InputTag_1) ||
		InputTag.MatchesTagExact(MagicGameplayTags.InputTag_2) ||
		InputTag.MatchesTagExact(MagicGameplayTags.InputTag_3) ||
		InputTag.MatchesTagExact(MagicGameplayTags.InputTag_4))
	{
		HandleMagicInput(InputTag);
		return;
	}
	
	if (bMagicDrawingMode || bMagicTargetingMode)
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
	if (bMagicDrawingMode || bMagicTargetingMode)
	{
		return;
	}
	
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
	
		FollowTime = 0.f;
		bTargeting = false;
	
}

void AAuraPlayerController::AbilityInputTagHeld(FGameplayTag InputTag)
{
	if (bMagicDrawingMode || bMagicTargetingMode)
	{
		return;
	}
	
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