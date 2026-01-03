// Copyright David He


#include "AI/BTTask_Attack.h"

#include "AIController.h"

EBTNodeResult::Type UBTTask_Attack::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	AAIController* Controller = OwnerComp.GetAIOwner();
	APawn* ControlledPawn = Controller->GetPawn();
	
	DrawDebugSphere(GetWorld(), ControlledPawn->GetActorLocation(), 40.f, 12, FColor::Red, false, 3.f);
	return Super::ExecuteTask(OwnerComp, NodeMemory);
}
