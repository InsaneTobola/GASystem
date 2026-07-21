// Copyright Druid Mechanics


#include "Interaction/CombatInterface.h"

class UMotionWarpingComponent;
// Add default functionality here for any ICombatInterface functions that are not pure virtual.
int32 ICombatInterface::GetPlayerLevel()
{
	return 0;
}

FVector ICombatInterface::GetCombatSocketLocation()
{
	return FVector();
}



