// Copyright David He

#pragma once

#include "CoreMinimal.h"
#include "BTTask_AuraBase.h"
#include "BTTask_GoAroundTarget.generated.h"

/**
 * 
 */
UCLASS(Blueprintable)
class AURAGAS_LEARNPROJECT_API UBTTask_GoAroundTarget : public UBTTask_AuraBase
{
	GENERATED_BODY()
	
protected:
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Default")
    FBlackboardKeySelector NewLocationSelector; 
 
    UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Default")
    FBlackboardKeySelector TargetSelector;

	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Default")
	float Radius;
	
private:
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
};
