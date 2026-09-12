#include "AI/AIAuraThreatSubsystem.h"

#include "Character/AuraEnemy.h"
#include "Engine/World.h"

void UAuraThreatSubsystem::RegisterEnemy(AAuraEnemy* Enemy)
{
	if (!IsValid(Enemy) || Enemy->AlertGroupId.IsNone())
	{
		return;
	}

	EnemiesByGroup.FindOrAdd(Enemy->AlertGroupId).Add(Enemy);
}

void UAuraThreatSubsystem::UnregisterEnemy(AAuraEnemy* Enemy)
{
	if (!IsValid(Enemy) || Enemy->AlertGroupId.IsNone())
	{
		return;
	}

	TSet<TWeakObjectPtr<AAuraEnemy>>* GroupMembers =
		EnemiesByGroup.Find(Enemy->AlertGroupId);

	if (!GroupMembers)
	{
		return;
	}

	GroupMembers->Remove(Enemy);

	if (GroupMembers->IsEmpty())
	{
		EnemiesByGroup.Remove(Enemy->AlertGroupId);
		LastGroupAlertTime.Remove(Enemy->AlertGroupId);
	}
}

void UAuraThreatSubsystem::ReportThreatForEnemy(AAuraEnemy* Witness,AActor* SourceActor,const FVector& ThreatLocation,EThreatType ThreatType,bool bNotifyWitness)
{
	if (!IsValid(Witness))
	{
		return;
	}

	FThreatAlert Alert;
	Alert.SourceActor = SourceActor;
	Alert.WitnessEnemy = Witness;
	Alert.ThreatLocation = ThreatLocation;
	Alert.AlertGroupId = Witness->AlertGroupId;
	Alert.ThreatType = ThreatType;

	if (bNotifyWitness)
	{
		Witness->ReceiveThreatAlert(Alert);
	}

	if (!Witness->AlertGroupId.IsNone())
	{
		BroadcastToGroup(Witness->AlertGroupId, Alert, Witness);
	}
}

void UAuraThreatSubsystem::ReportThreatAtLocation(
	AActor* SourceActor,
	const FVector& ThreatLocation,
	float SeedRadius,
	EThreatType ThreatType)
{
	AAuraEnemy* ClosestEnemy = nullptr;
	float ClosestDistanceSquared = FMath::Square(SeedRadius);

	for (const TPair<FName, TSet<TWeakObjectPtr<AAuraEnemy>>>& Pair : EnemiesByGroup)
	{
		for (const TWeakObjectPtr<AAuraEnemy>& WeakEnemy : Pair.Value)
		{
			AAuraEnemy* Enemy = WeakEnemy.Get();

			if (!IsValid(Enemy))
			{
				continue;
			}

			const float DistanceSquared =
				FVector::DistSquared(Enemy->GetActorLocation(), ThreatLocation);

			if (DistanceSquared < ClosestDistanceSquared)
			{
				ClosestDistanceSquared = DistanceSquared;
				ClosestEnemy = Enemy;
			}
		}
	}

	if (ClosestEnemy)
	{
		ReportThreatForEnemy(
			ClosestEnemy,
			SourceActor,
			ThreatLocation,
			ThreatType,
			true
		);
	}
}

void UAuraThreatSubsystem::EndGroupCombat(FName AlertGroupId)
{
	if (AlertGroupId.IsNone())
	{
		return;
	}

	ActiveCombatGroups.Remove(AlertGroupId);
}

void UAuraThreatSubsystem::RegisterEnemyInCombat(AAuraEnemy* Enemy)
{
	if (!IsValid(Enemy) || Enemy->AlertGroupId.IsNone())
	{
		return;
	}

	ActiveCombatEnemies.Add(Enemy);
}

void UAuraThreatSubsystem::UnregisterEnemyFromCombat(AAuraEnemy* Enemy)
{  
	if (!IsValid(Enemy))
	{
		return;
	}

	ActiveCombatEnemies.Remove(Enemy);

	if (Enemy->AlertGroupId.IsNone())
	{
		return;
	}

	for (const TWeakObjectPtr<AAuraEnemy>& WeakEnemy : ActiveCombatEnemies)
	{
		AAuraEnemy* ActiveEnemy = WeakEnemy.Get();

		if (!IsValid(ActiveEnemy))
		{
			continue;
		}

		if (ActiveEnemy->AlertGroupId == Enemy->AlertGroupId)
		{
			return;
		}
	}

	ActiveCombatGroups.Remove(Enemy->AlertGroupId);
}

void UAuraThreatSubsystem::BroadcastToGroup(
	FName AlertGroupId,
	const FThreatAlert& Alert,
	AAuraEnemy* ExcludedEnemy)
{
	if (AlertGroupId.IsNone() || !GetWorld())
	{
		return;
	}

	if (ActiveCombatGroups.Contains(AlertGroupId))
	{
		return;
	}

	ActiveCombatGroups.Add(AlertGroupId);

	TSet<TWeakObjectPtr<AAuraEnemy>>* GroupMembers =
		EnemiesByGroup.Find(AlertGroupId);

	if (!GroupMembers)
	{
		return;
	}

	for (TSet<TWeakObjectPtr<AAuraEnemy>>::TIterator It(*GroupMembers); It; ++It)
	{
		AAuraEnemy* Enemy = It->Get();

		if (!IsValid(Enemy))
		{
			It.RemoveCurrent();
			continue;
		}

		if (Enemy == ExcludedEnemy)
		{
			continue;
		}

		Enemy->ReceiveThreatAlert(Alert);
	}
}
