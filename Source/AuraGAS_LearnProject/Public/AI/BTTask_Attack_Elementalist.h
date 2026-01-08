// Copyright David He

#pragma once

#include "CoreMinimal.h"
#include "AI/BTTask_AuraBase.h"
#include "BTTask_Attack_Elementalist.generated.h"

/**
 * 
 */
UCLASS(Blueprintable)
class AURAGAS_LEARNPROJECT_API UBTTask_Attack_Elementalist : public UBTTask_AuraBase
{
	GENERATED_BODY()
protected:
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Default")
	FGameplayTag AttackTag;
	
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Default")
	FGameplayTag SummonTag;
	
	FGameplayTag AbilityTag;
	
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Default")
	FBlackboardKeySelector CombatTargetSelector;
	
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Default")
	int32 MinionSpawnThreshold = 2;
	
private:
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
};
