#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "AI/AIAuraThreatTypes.h"
#include "AIAuraThreatSubsystem.generated.h"

class AAuraEnemy;

UCLASS()
class AURA_API UAuraThreatSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	void RegisterEnemy(AAuraEnemy* Enemy);
	void UnregisterEnemy(AAuraEnemy* Enemy);
	void ReportThreatForEnemy(AAuraEnemy* Witness,AActor* SourceActor,const FVector& ThreatLocation,EThreatType ThreatType,bool bNotifyWitness);
	void ReportThreatAtLocation(AActor* SourceActor,const FVector& ThreatLocation,float SeedRadius,EThreatType ThreatType);
	void EndGroupCombat(FName AlertGroupId);
	void RegisterEnemyInCombat(AAuraEnemy* Enemy);
	void UnregisterEnemyFromCombat(AAuraEnemy* Enemy);
	
private:
	void BroadcastToGroup(FName AlertGroupId,const FThreatAlert& Alert,AAuraEnemy* ExcludedEnemy);

	TMap<FName, TSet<TWeakObjectPtr<AAuraEnemy>>> EnemiesByGroup;
	TMap<FName, double> LastGroupAlertTime;
	TSet<FName> ActiveCombatGroups;
	TSet<TWeakObjectPtr<AAuraEnemy>> ActiveCombatEnemies;

	float GroupAlertCooldown = 0.5f;
};