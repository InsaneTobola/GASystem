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
#include "GameFramework/CharacterMovementComponent.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISenseConfig_Sight.h"
#include "Perception/AIPerceptionTypes.h"
#include "Components/StateTreeComponent.h"



AAuraEnemy::AAuraEnemy()
{
	
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

void AAuraEnemy::BeginPlay()
{
	Super::BeginPlay();
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
		HandlePlayerDetected(Player);
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

	GetWorldTimerManager().ClearTimer(LoseSightTimerHandle);
	
}

void AAuraEnemy::HandlePlayerLost()
{
	GetWorldTimerManager().SetTimer(LoseSightTimerHandle,this,&AAuraEnemy::ExecuteStopChase,LoseSightDelay,false);
}

void AAuraEnemy::ExecuteStopChase()
{
	OnPlayerLost();
}