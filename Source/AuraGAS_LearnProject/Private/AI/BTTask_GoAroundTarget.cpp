// Copyright David He


#include "AI/BTTask_GoAroundTarget.h"

#include "NavigationSystem.h"
#include "BehaviorTree/BlackboardComponent.h"

EBTNodeResult::Type UBTTask_GoAroundTarget::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	UBlackboardComponent* BlackboardComp = OwnerComp.GetBlackboardComponent();
	if (!BlackboardComp) return EBTNodeResult::Failed;
	
	if (AActor* Target = Cast<AActor>(BlackboardComp->GetValueAsObject(TargetSelector.SelectedKeyName)))
	{
		if (UNavigationSystemV1* NavSystem = FNavigationSystem::GetCurrent<UNavigationSystemV1>(GetWorld()))
		{
			FNavLocation TargetLocation;
            if (NavSystem->GetRandomPointInNavigableRadius(Target->GetActorLocation(), Radius, TargetLocation))
            {
            	BlackboardComp->SetValueAsVector(NewLocationSelector.SelectedKeyName, TargetLocation);
            	return EBTNodeResult::Succeeded;
            }
		}
	}
	
	return EBTNodeResult::Failed;
}
