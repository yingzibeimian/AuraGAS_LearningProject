// Copyright David He

#pragma once

#include "CoreMinimal.h"
#include "BTTask_AuraBase.h"
#include "BTTask_Attack.generated.h"

/**
 * 
 */
UCLASS(Blueprintable)
class AURAGAS_LEARNPROJECT_API UBTTask_Attack : public UBTTask_AuraBase
{
	GENERATED_BODY()
	
protected:
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Default")
	FGameplayTag AttackTag;
	
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Default") 
	FBlackboardKeySelector CombatTargetSelector;
	
private:
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
};
