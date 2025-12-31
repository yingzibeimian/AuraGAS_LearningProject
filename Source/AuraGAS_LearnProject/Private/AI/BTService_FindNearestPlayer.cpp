// Copyright David He


#include "AI/BTService_FindNearestPlayer.h"

#include "AIController.h"

UBTService_FindNearestPlayer::UBTService_FindNearestPlayer()
{
	INIT_SERVICE_NODE_NOTIFY_FLAGS();
}

void UBTService_FindNearestPlayer::TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	Super::TickNode(OwnerComp, NodeMemory, DeltaSeconds);
	
	AAIController* Controller = OwnerComp.GetAIOwner();
	APawn* OwningPawn = Controller->GetPawn();
	
	GEngine->AddOnScreenDebugMessage(1, 1.f, FColor::Red, *Controller->GetName());
	GEngine->AddOnScreenDebugMessage(2, 1.f, FColor::Green, *OwningPawn->GetName());
}
