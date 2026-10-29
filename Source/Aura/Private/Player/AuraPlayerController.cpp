
#include "Player/AuraPlayerController.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystem/AuraAbilitySystemComponent.h"
#include "AbilitySystem/Abilities/AuraAreaSpell.h"
#include "AbilitySystem/Abilities/AuraSelfCastSpell.h"
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
#include "Magic/MagicDebug.h"

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
	if (bMagicDrawingMode || bMagicTargetingMode)
	{
		return;
	}

	CursorTrace();
	
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
			MagicDrawingWidget->SetIsFocusable(false);
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

	const UAuraSelfCastSpell* SelfCastDefaults = Cast<UAuraSelfCastSpell>(Pattern->SkillAbilityClass.GetDefaultObject());
	
	if (SelfCastDefaults)
	{
		// This Self-Cast implementation is for Water only.
		if (Pattern->Element != EMagicElement::Water)
		{
			UE_LOG(
				LogTemp,
				Warning,
				TEXT("Water Self-Cast requires the Water element.")
			);

			StopMagicDrawingMode();
			MagicComponent->CancelMagic();
			return;
		}

		// Close drawing mode. No world targeting is required.
		StopMagicDrawingMode();
		MagicComponent->CancelMagic();

		// Prevent another cast request before the server responds.
		bWaitingForMagicCastResponse = true;

		ServerConfirmMagicSelfCast(
			Pattern->PatternId,
			Pattern->Element
		);

		return;
	}

	// Existing Area Spell flow continues below.
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

	ServerConfirmMagicTarget(PendingMagicPatternId,PendingMagicElement,MagicTargetingActor->GetMagicTargetLocation());
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

void AAuraPlayerController::ServerConfirmMagicSelfCast_Implementation(FName PatternId,EMagicElement Element)
{
	const auto RejectCast = [this](const TCHAR* Reason)
	{
		UE_LOG(
			LogTemp,
			Warning,
			TEXT("[MagicSelfCast] Server rejected cast: %s"),
			Reason
		);

		ClientMagicSelfCastRejected();
	};

	AAuraCharacter* AuraCharacter = Cast<AAuraCharacter>(GetPawn());

	if (!IsValid(AuraCharacter) || !IsValid(AuraCharacter->MagicComponent))
	{
		RejectCast(TEXT("Invalid character or MagicComponent."));
		return;
	}

	UAbilitySystemComponent* ASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(AuraCharacter);

	if (!IsValid(ASC))
	{
		RejectCast(TEXT("Invalid AbilitySystemComponent."));
		return;
	}

	UMagicPatternDefinition* Pattern = AuraCharacter->MagicComponent->FindPatternDefinition(PatternId);

	if (!IsValid(Pattern) || Pattern->Element != Element || Element != EMagicElement::Water || !Pattern->SkillAbilityClass)
	{
		RejectCast(TEXT("Invalid pattern, element or ability class."));
		return;
	}

	const UAuraSelfCastSpell* SelfCastDefaults = Cast<UAuraSelfCastSpell>(Pattern->SkillAbilityClass.GetDefaultObject());

	if (!SelfCastDefaults || !ASC->FindAbilitySpecFromClass(Pattern->SkillAbilityClass))
	{
		RejectCast(TEXT("Self-Cast class is invalid or not granted to ASC."));
		return;
	}

	if (!ASC->TryActivateAbilityByClass(Pattern->SkillAbilityClass))
	{
		RejectCast(TEXT("TryActivateAbilityByClass failed."));
		return;
	}

	UE_LOG(
		LogTemp,
		Log,
		TEXT("[MagicSelfCast] Activation accepted for pattern: %s"),
		*PatternId.ToString()
	);

	// The ability reports final success or failure.
}

void AAuraPlayerController::ServerConfirmMagicTarget_Implementation(FName PatternId, EMagicElement Element, FVector TargetLocation)
{
	AuraMagicDebugPrint(
	FString::Printf(
		TEXT("SERVER: Cast request Pattern=%s Element=%d Location=(%.0f, %.0f, %.0f)"),
		*PatternId.ToString(),
		static_cast<int32>(Element),
		TargetLocation.X,
		TargetLocation.Y,
		TargetLocation.Z
	),
	FColor::Cyan,
	5.f
);
	
	 AAuraCharacter* AuraCharacter = Cast<AAuraCharacter>(GetPawn());

    if (!IsValid(AuraCharacter) || !IsValid(AuraCharacter->MagicComponent))
    {
    	AuraMagicDebugPrint(TEXT("SERVER REJECT: Invalid character or MagicComponent."), FColor::Red);
        ClientMagicTargetRejected();
        return;
    }

    UAbilitySystemComponent* ASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(AuraCharacter);

    if (!IsValid(ASC))
    {
    	AuraMagicDebugPrint(TEXT("SERVER REJECT: AbilitySystemComponent is invalid."), FColor::Red);
        ClientMagicTargetRejected();
        return;
    }

    UMagicPatternDefinition* Pattern = AuraCharacter->MagicComponent->FindPatternDefinition(PatternId);

    if (!IsValid(Pattern) || Pattern->Element != Element || !Pattern->SkillAbilityClass)
    {
    	AuraMagicDebugPrint(TEXT("SERVER REJECT: Pattern, element or ability class is invalid."), FColor::Red);
        ClientMagicTargetRejected();
        return;
    }

    const UAuraAreaSpell* SpellDefaults = Cast<UAuraAreaSpell>(Pattern->SkillAbilityClass.GetDefaultObject());

    if (!SpellDefaults || !ASC->FindAbilitySpecFromClass(Pattern->SkillAbilityClass))
    {
    	AuraMagicDebugPrint(TEXT("SERVER REJECT: Ability spec was not granted or class is invalid."), FColor::Red);
        ClientMagicTargetRejected();
        return;
    }

    FHitResult GroundHit;
    FCollisionQueryParams QueryParams(SCENE_QUERY_STAT(MagicTargetValidation),false);
    QueryParams.AddIgnoredActor(AuraCharacter);

    const FVector TraceStart = TargetLocation + FVector(0.f, 0.f, 5000.f);
    const FVector TraceEnd = TargetLocation - FVector(0.f, 0.f, 5000.f);

    if (!GetWorld()->LineTraceSingleByChannel(GroundHit,TraceStart,TraceEnd,ECC_WorldStatic,QueryParams))
    {
    	AuraMagicDebugPrint(TEXT("SERVER REJECT: Ground trace failed."), FColor::Red);
        ClientMagicTargetRejected();
        return;
    }

    const FVector ValidatedLocation = GroundHit.Location;
    const float Distance2D = FVector::Dist2D(AuraCharacter->GetActorLocation(),ValidatedLocation);

    if (Distance2D > SpellDefaults->GetMaxRangeCm())
    {
    	AuraMagicDebugPrint(TEXT("SERVER REJECT: Target is outside maximum range."), FColor::Red);
        ClientMagicTargetRejected();
        return;
    }

    AuraCharacter->MagicComponent->SetPendingCast(
        PatternId,
        ValidatedLocation
    );

	if (!ASC->TryActivateAbilityByClass(Pattern->SkillAbilityClass))
	{
		AuraMagicDebugPrint(
			TEXT("SERVER REJECT: TryActivateAbilityByClass returned false."),
			FColor::Red
		);

		AuraCharacter->MagicComponent->ClearPendingCast();
		ClientMagicTargetRejected();
		return;
	}

	AuraMagicDebugPrint(
		TEXT("SERVER: Ability activation accepted. Waiting for field spawn result."),
		FColor::Yellow,
		5.f
	);
}

void AAuraPlayerController::ClientMagicTargetAccepted_Implementation()
{
	AuraMagicDebugPrint(
	   TEXT("MAGIC CAST SUCCESS: Area field was spawned."),
	   FColor::Green,
	   5.f
   );
	
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
	AuraMagicDebugPrint(
	   TEXT("MAGIC CAST FAILED: Check the preceding server log."),
	   FColor::Red,
	   6.f
   );
	
	bWaitingForMagicCastResponse = false;

	UE_LOG(
		LogTemp,
		Warning,
		TEXT("Magic target was rejected by the server.")
	);
}

void AAuraPlayerController::ClientMagicSelfCastAccepted_Implementation()
{
	bWaitingForMagicCastResponse = false;

	StopMagicDrawingMode();

	if (UAuraMagicComponent* MagicComponent = GetMagicComponent())
	{
		MagicComponent->CancelMagic();
	}

	AuraMagicDebugPrint(
		TEXT("WATER SELF-CAST SUCCESS: Healing effect applied."),
		FColor::Green,
		5.f
	);
}

void AAuraPlayerController::ClientMagicSelfCastRejected_Implementation()
{
	bWaitingForMagicCastResponse = false;

	StopMagicDrawingMode();

	if (UAuraMagicComponent* MagicComponent = GetMagicComponent())
	{
		MagicComponent->CancelMagic();
	}

	AuraMagicDebugPrint(
		TEXT("WATER SELF-CAST FAILED: Check the server log."),
		FColor::Red,
		5.f
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