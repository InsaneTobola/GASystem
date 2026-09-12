// Copyright Druid Mechanics


#include "Character/AuraEnemy.h"
#include "Character/AuraCharacter.h"
#include "Aura/Aura.h"
#include "AbilitySystem/AuraAbilitySystemComponent.h"
#include "AbilitySystem/AuraAbilitySystemLibrary.h"
#include "Components/WidgetComponent.h"
#include "AbilitySystem/AuraAttributeSet.h"
#include "UI/Widget/AuraUserWidget.h"
#include "AuraGameplayTags.h"
#include "Components/StateTreeComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "AIController.h"
#include "AI/AIAuraThreatSubsystem.h"
#include "Navigation/PathFollowingComponent.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISenseConfig_Sight.h"
#include "Perception/AIPerceptionTypes.h"




AAuraEnemy::AAuraEnemy()
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = true;
	
	AIPerception = CreateDefaultSubobject<UAIPerceptionComponent>("AIPerception");
	SightConfig = CreateDefaultSubobject<UAISenseConfig_Sight>("SightConfig");
	
	SightConfig->SightRadius = SightRadius;
	SightConfig->LoseSightRadius = LoseSightRadius;
	SightConfig->PeripheralVisionAngleDegrees = PeripheralVisionAngle;

	SightConfig->DetectionByAffiliation.bDetectEnemies = true;
	SightConfig->DetectionByAffiliation.bDetectNeutrals = true;
	SightConfig->DetectionByAffiliation.bDetectFriendlies = true;

	AIPerception->ConfigureSense(*SightConfig);
	AIPerception->SetDominantSense(UAISense_Sight::StaticClass());
	
	GetMesh()->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);
	
	AbilitySystemComponent = CreateDefaultSubobject<UAuraAbilitySystemComponent>("AbilitySystemComponent");
	AbilitySystemComponent->SetIsReplicated(true);
	AbilitySystemComponent->SetReplicationMode(EGameplayEffectReplicationMode::Minimal);
	
	AttributeSet = CreateDefaultSubobject<UAuraAttributeSet>("AttributeSet");
	
	HealthBar = CreateDefaultSubobject<UWidgetComponent>("HealthBar");
	HealthBar->SetupAttachment(GetRootComponent());
}

void AAuraEnemy::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	
	// =========================
	// SEARCH - LOOK AROUND
	// =========================

	if (bIsLookingAround && bIsTurningDuringSearch)
	{
		const FRotator NewRotation = FMath::RInterpTo(
			GetActorRotation(),
			SearchLookTargetRotation,
			DeltaTime,
			LookAroundRotationSpeed
		);

		SetActorRotation(NewRotation);

		const float DeltaYaw = FMath::Abs(
			FMath::FindDeltaAngleDegrees(
				GetActorRotation().Yaw,
				SearchLookTargetRotation.Yaw
			)
		);

		if (DeltaYaw <= LookAroundTolerance)
		{
			SetActorRotation(SearchLookTargetRotation);

			bIsTurningDuringSearch = false;

			GetWorldTimerManager().SetTimer(
				LookAroundTimerHandle,
				this,
				&AAuraEnemy::FinishLookingAround,
				LookAroundPause,
				false
			);
		}
	}

	// =========================
	// ALERT - FACE THREAT LOCATION
	// =========================

	if (bIsFacingThreatLocation)
	{
		FVector Direction =
			PendingThreatLocation - GetActorLocation();

		Direction.Z = 0.f;

		if (Direction.IsNearlyZero())
		{
			bIsFacingThreatLocation = false;
			OnFinishedFacingThreatLocation();
			return;
		}

		const FRotator ThreatRotation = Direction.Rotation();

		const FRotator NewRotation = FMath::RInterpTo(
			GetActorRotation(),
			FRotator(0.f, ThreatRotation.Yaw, 0.f),
			DeltaTime,
			RotationInterpSpeed
		);

		SetActorRotation(NewRotation);

		const float DeltaYaw = FMath::Abs(
			FMath::FindDeltaAngleDegrees(
				GetActorRotation().Yaw,
				ThreatRotation.Yaw
			)
		);

		if (DeltaYaw <= FacingTolerance)
		{
			SetActorRotation(FRotator(0.f, ThreatRotation.Yaw, 0.f));
			bIsFacingThreatLocation = false;
			OnFinishedFacingThreatLocation();
		}

		return;
	}
	// =========================
	// COMBAT - FACE TARGET
	// =========================

	if (!bIsFacingCurrentTarget || !CurrentTarget)
	{
		return;
	}

	FVector Direction =
		CurrentTarget->GetActorLocation() - GetActorLocation();

	Direction.Z = 0.f;

	if (Direction.IsNearlyZero())
	{
		bIsFacingCurrentTarget = false;
		OnFinishedFacingTarget();
		return;
	}

	const FRotator TargetRotation = Direction.Rotation();

	const FRotator NewRotation = FMath::RInterpTo(
		GetActorRotation(),
		FRotator(0.f, TargetRotation.Yaw, 0.f),
		DeltaTime,
		RotationInterpSpeed
	);

	SetActorRotation(NewRotation);

	if (IsFacingCurrentTarget())
	{
		bIsFacingCurrentTarget = false;
		OnFinishedFacingTarget();
	}
}

void AAuraEnemy::HighlightActor()
{
	
	GetMesh()->SetRenderCustomDepth(true);
	GetMesh()->SetCustomDepthStencilValue(CUSTOM_DEPTH_RED);
	Weapon->SetRenderCustomDepth(true);
	Weapon->SetCustomDepthStencilValue(CUSTOM_DEPTH_RED);
}
void AAuraEnemy::UnHighlightActor()
{
	
	GetMesh()->SetRenderCustomDepth(false);
	Weapon->SetRenderCustomDepth(false);
}
int32 AAuraEnemy::GetPlayerLevel()
{
	return Level;
}

void AAuraEnemy::Die()
{
	SetLifeSpan(LifeSpan);
	Super::Die();
}

bool AAuraEnemy::IsTargetInAttackRange() const
{
	if (!CurrentTarget)
	{
		return false;
	}

	const float Distance = FVector::Dist(
		GetActorLocation(),
		CurrentTarget->GetActorLocation()
	);

	return Distance <= AttackRange;
}

void AAuraEnemy::HandleDamageReceived(AActor* SourceActor)
{
	if (!HasAuthority() || !IsValid(SourceActor))
	{
		return;
	}
	
	if (UAuraThreatSubsystem* ThreatSubsystem =
	   GetWorld()->GetSubsystem<UAuraThreatSubsystem>())
	{
		ThreatSubsystem->RegisterEnemyInCombat(this);
	}

	HandlePlayerDetected(SourceActor);
}

AActor* AAuraEnemy::GetCurrentTarget() const
{
	return CurrentTarget;
}

void AAuraEnemy::HandlePlayerDetectedFromGroup(AActor* Player)
{
	if (!HasAuthority() || !IsValid(Player))
	{
		return;
	}

	HandlePlayerDetected(Player);
}

void AAuraEnemy::FaceCurrentTarget()
{
	if (!CurrentTarget)
	{
		return;
	}
	bIsFacingCurrentTarget = true;
}

bool AAuraEnemy::IsFacingCurrentTarget() const
{
	if (!CurrentTarget)
	{
		return false;
	}
	FVector Direction = CurrentTarget->GetActorLocation() - GetActorLocation();
	Direction.Z = 0.f;
	
	if (Direction.IsNearlyZero())
	{
		return true;
	}
	const FRotator TargetRotation = Direction.Rotation();
	const float DeltaYaw = FMath::Abs(FMath::FindDeltaAngleDegrees(GetActorRotation().Yaw,TargetRotation.Yaw));

	return DeltaYaw <= FacingTolerance;
}

void AAuraEnemy::ReceiveThreatAlert(const FThreatAlert& Alert)
{
	if (!HasAuthority())
	{
		return;
	}
	
	PendingThreatSource = Alert.SourceActor;
	PendingThreatWitness = Alert.WitnessEnemy;
	PendingThreatLocation = Alert.ThreatLocation;

	LastKnownTargetLocation = Alert.ThreatLocation;
	bHasLastKnownTargetLocation = true;

	if (UAuraThreatSubsystem* ThreatSubsystem =
	GetWorld()->GetSubsystem<UAuraThreatSubsystem>())
	{
		ThreatSubsystem->RegisterEnemyInCombat(this);
	}

	OnThreatAlertReceived();
}

void AAuraEnemy::ResolvePendingThreat()
{
	if (IsValid(PendingThreatWitness))
	{
		StartChasing(PendingThreatWitness);
		PendingThreatWitness = nullptr;
		PendingThreatSource = nullptr;
		return;
	}
	AAIController* AIController = Cast<AAIController>(GetController());
	const bool bHasValidSource =IsValid(PendingThreatSource) && AIController != nullptr;
	bool bCanSeeThreatSource = false;

	if (bHasValidSource)
	{
		const float DistanceSquared = FVector::DistSquared(GetActorLocation(),PendingThreatSource->GetActorLocation());
		bCanSeeThreatSource =DistanceSquared <= FMath::Square(SightRadius) &&AIController->LineOfSightTo(PendingThreatSource);
	}

	if (bCanSeeThreatSource)
	{
		HandlePlayerDetected(PendingThreatSource);
	}
	else
	{
		CurrentTarget = nullptr;
		TargetPawn = nullptr;
		OnThreatSearch();
	}

	PendingThreatSource = nullptr;
}

void AAuraEnemy::StartChasing(AActor* TargetActor)
{
	if (!TargetActor)
	{
		return;
	}

	AAIController* AIController =
		Cast<AAIController>(GetController());

	if (!AIController)
	{
		return;
	}

	FAIMoveRequest MoveRequest;

	MoveRequest.SetGoalActor(TargetActor);

	MoveRequest.SetAcceptanceRadius(150.f);

	MoveRequest.SetUsePathfinding(true);

	MoveRequest.SetAllowPartialPath(true);

	MoveRequest.SetReachTestIncludesAgentRadius(false);

	MoveRequest.SetReachTestIncludesGoalRadius(false);

	AIController->MoveTo(MoveRequest);
}

void AAuraEnemy::StartSearching()
{
	
	
	if (!bHasLastKnownTargetLocation)
	{
		return;
	}

	AAIController* AIController = Cast<AAIController>(GetController());

	if (!AIController)
	{
		return;
	}

	UPathFollowingComponent* PathFollowingComponent =
		AIController->GetPathFollowingComponent();

	if (!PathFollowingComponent)
	{
		return;
	}
	
	PathFollowingComponent->OnRequestFinished.RemoveAll(this);
	
	PathFollowingComponent->OnRequestFinished.AddUObject(
		this,
		&AAuraEnemy::OnSearchMoveCompleted
	);

	bIsSearching = true;

	FAIMoveRequest MoveRequest;

	MoveRequest.SetGoalLocation(LastKnownTargetLocation);
	MoveRequest.SetAcceptanceRadius(50.f);
	MoveRequest.SetUsePathfinding(true);
	MoveRequest.SetAllowPartialPath(false);
	MoveRequest.SetReachTestIncludesAgentRadius(false);

	const FPathFollowingRequestResult MoveResult =
		AIController->MoveTo(MoveRequest);

	SearchMoveRequestID = MoveResult.MoveId;
	
}

void AAuraEnemy::StopSearching()
{
	const bool bWasSearching = bIsSearching || bIsLookingAround;

	bIsSearching = false;
	bIsLookingAround = false;
	bIsTurningDuringSearch = false;

	GetWorldTimerManager().ClearTimer(LookAroundTimerHandle);
	
	if (!bWasSearching)
	{
		return;
	}

	AAIController* AIController = Cast<AAIController>(GetController());

	if (AIController)
	{
		AIController->StopMovement();
	}
}

void AAuraEnemy::UpdateLastKnownTargetLocation(AActor* TargetActor)
{
	if (!IsValid(TargetActor))
	{
		return;
	}

	LastKnownTargetLocation = TargetActor->GetActorLocation();

	bHasLastKnownTargetLocation = true;
}

void AAuraEnemy::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (HasAuthority() && !AlertGroupId.IsNone())
	{
		if (UAuraThreatSubsystem* ThreatSubsystem =
			GetWorld()->GetSubsystem<UAuraThreatSubsystem>())
		{
			ThreatSubsystem->UnregisterEnemy(this);
		}
	}

	Super::EndPlay(EndPlayReason);
}

void AAuraEnemy::FaceThreatLocation()
{
	bIsFacingThreatLocation = true;
}

void AAuraEnemy::BeginPlay()
{
	Super::BeginPlay();
	
	StateTreeComponent = FindComponentByClass<UStateTreeComponent>();
	
	if (HasAuthority() && !AlertGroupId.IsNone())
	{
		if (UAuraThreatSubsystem* ThreatSubsystem =
			GetWorld()->GetSubsystem<UAuraThreatSubsystem>())
		{
			ThreatSubsystem->RegisterEnemy(this);
		}
	}
	
	
	GetCharacterMovement()->MaxWalkSpeed = BaseWalkSpeed;
	InitAbilityActorInfo();

	if (AIPerception)
	{
		AIPerception->OnTargetPerceptionUpdated.AddDynamic(this,&AAuraEnemy::OnTargetPerceptionUpdated);
	}
	
	if (HasAuthority())
	{
		InitializeDefaultAttributes();

		UAuraAbilitySystemLibrary::GiveStartupAbilities(
			this,
			AbilitySystemComponent
		);

		UAuraAbilitySystemLibrary::GiveCombatAbilities(this, AbilitySystemComponent, CombatProfile);
	}
	
	if (UAuraUserWidget* AuraUserWidget = Cast<UAuraUserWidget>(HealthBar->GetUserWidgetObject()))
	{
		AuraUserWidget->SetWidgetController(this);
	}
	
	if (const UAuraAttributeSet* AuraAS = Cast<UAuraAttributeSet>(AttributeSet))
	{
		AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(AuraAS->GetHealthAttribute()).AddLambda(
		[this](const FOnAttributeChangeData& Data)
			{
				OnHealthChanged.Broadcast(Data.NewValue);
			}	
		);
		AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(AuraAS->GetMaxHealthAttribute()).AddLambda(
		[this](const FOnAttributeChangeData& Data)
			{
				OnMaxHealthChanged.Broadcast(Data.NewValue);
			}	
		);
		
		AbilitySystemComponent->RegisterGameplayTagEvent(FAuraGameplayTags::Get().HitReact, EGameplayTagEventType::NewOrRemoved).AddUObject(
		this, 
		&AAuraEnemy::HitReactTagChanged
		);
		
		OnHealthChanged.Broadcast(AuraAS->GetHealth());
		OnMaxHealthChanged.Broadcast(AuraAS->GetMaxHealth());
	}
	
	
}
void AAuraEnemy::HitReactTagChanged(const FGameplayTag CallbackTag, int32 NewCount)
{
	bHitReacting = NewCount > 0;
	GetCharacterMovement()->MaxWalkSpeed = bHitReacting ? 0.f : BaseWalkSpeed;
}
void AAuraEnemy::InitAbilityActorInfo()
{
	AbilitySystemComponent->InitAbilityActorInfo(this, this);
	
	CastChecked<UAuraAbilitySystemComponent>(AbilitySystemComponent)
	   ->AbilityActorInfoSet();
}
void AAuraEnemy::InitializeDefaultAttributes() const
{
	UAuraAbilitySystemLibrary::InitializeDefaultAttributes(this, CharacterClass, Level, AbilitySystemComponent);
}

void AAuraEnemy::OnTargetPerceptionUpdated(AActor* Actor,FAIStimulus Stimulus)
{
	if (!Actor)
	{
		return;
	}
	
	AAuraCharacter* Player = Cast<AAuraCharacter>(Actor);

	if (!Player)
	{
		return;
	}

	if (Stimulus.WasSuccessfullySensed())
	{
		TargetPawn = Player;

		UpdateLastKnownTargetLocation(Player);
		
		HandlePlayerDetected(Player);
		if (HasAuthority() && !AlertGroupId.IsNone())
		{
			if (UAuraThreatSubsystem* ThreatSubsystem =
				GetWorld()->GetSubsystem<UAuraThreatSubsystem>())
			{
				ThreatSubsystem->ReportThreatForEnemy(
					this,
					Player,
					Player->GetActorLocation(),
					EThreatType::Sight,
					false
				);
			}
		}
	}
	else
	{
		HandlePlayerLost();
	}
}

void AAuraEnemy::HandlePlayerDetected(AActor* Player)
{
	if (!Player)
	{
		return;
	}

	if (bIsSearching || bIsLookingAround)
	{
		StopSearching();
	}

	CurrentTarget = Player;

	GetWorldTimerManager().ClearTimer(LoseSightTimerHandle);

	OnPlayerDetected(Player);
}


void AAuraEnemy::HandlePlayerLost()
{
	GetWorldTimerManager().SetTimer(LoseSightTimerHandle,this,&AAuraEnemy::ExecuteStopChase,LoseSightDelay,false);
}

void AAuraEnemy::ExecuteStopChase()
{
	OnPlayerLost();
}

void AAuraEnemy::OnSearchMoveCompleted(FAIRequestID RequestID, const FPathFollowingResult& Result)
{
	if (RequestID != SearchMoveRequestID)
	{
		return;
	}

	if (!bIsSearching)
	{
		return;
	}

	if (Result.Code == EPathFollowingResult::Success)
	{
		StartLookingAround();
	}
}

// ==========================================
// AI - Search / Look Around
// ==========================================

void AAuraEnemy::StartLookingAround()
{
	if (!bIsSearching)
	{
		return;
	}

	bIsLookingAround = true;

	bIsTurningDuringSearch = false;

	CurrentLookAroundCount = 0;

	MaxLookAroundCount = FMath::RandRange(
		MinLookAroundCount,
		MaxLookAroundCountSetting
	);

	StartNextLookAround();
}

void AAuraEnemy::StartNextLookAround()
{
	if (!bIsSearching || !bIsLookingAround)
	{
		return;
	}

	if (CurrentLookAroundCount >= MaxLookAroundCount)
	{
		FinishSearch();
		return;
	}

	const bool bLookRight = FMath::RandBool();

	const float DirectionMultiplier =bLookRight ? 1.f : -1.f;

	const float TargetYaw =GetActorRotation().Yaw +(LookAroundAngle * DirectionMultiplier);

	SearchLookTargetRotation = FRotator(0.f,TargetYaw,0.f);

	CurrentLookAroundCount++;

	bIsTurningDuringSearch = true;
}

void AAuraEnemy::FinishLookingAround()
{
	if (!bIsSearching)
	{
		return;
	}

	StartNextLookAround();
}

void AAuraEnemy::FinishSearch()
{
	bIsLookingAround = false;

	bIsTurningDuringSearch = false;

	bIsSearching = false;

	GetWorldTimerManager().ClearTimer(LookAroundTimerHandle);
	
	if (UAuraThreatSubsystem* ThreatSubsystem =
	GetWorld()->GetSubsystem<UAuraThreatSubsystem>())
	{
		ThreatSubsystem->UnregisterEnemyFromCombat(this);
	}

	OnSearchFinished();
}
