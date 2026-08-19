// Copyright Druid Mechanics


#include "AI/AICombatComponent.h"

UAICombatComponent::UAICombatComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UAICombatComponent::BeginPlay()
{
	Super::BeginPlay();
}