// Copyright David He

#pragma once

#include "CoreMinimal.h"
#include "BTService_AuraBase.h"
#include "BTService_FindNearestPlayer.generated.h"

/**
 * 
 */
UCLASS(Blueprintable)
class AURAGAS_LEARNPROJECT_API UBTService_FindNearestPlayer : public UBTService_AuraBase
{
	GENERATED_BODY()
	
protected:
	
	virtual void TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;
	
	UPROPERTY(BlueprintReadOnly, EditAnywhere) 
	FBlackboardKeySelector TargetToFollowSelector; 
 
	UPROPERTY(BlueprintReadOnly, EditAnywhere) 
	FBlackboardKeySelector DistanceToTargetSelector;
};
