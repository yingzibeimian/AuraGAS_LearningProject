// Copyright David He

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/Services/BTService_BlueprintBase.h"
#include "BTService_FindNearestPlayer.generated.h"

/**
 * 
 */
UCLASS(Blueprintable)
class AURAGAS_LEARNPROJECT_API UBTService_FindNearestPlayer : public UBTService
{
	GENERATED_BODY()
public:
	UBTService_FindNearestPlayer();
	
protected:
	
	virtual void TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;
	
	UPROPERTY(BlueprintReadOnly, EditAnywhere) 
	FBlackboardKeySelector TargetToFollowSelector; 
 
	UPROPERTY(BlueprintReadOnly, EditAnywhere) 
	FBlackboardKeySelector DistanceToTargetSelector;
};
