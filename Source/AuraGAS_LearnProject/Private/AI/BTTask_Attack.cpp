// Copyright David He


#include "AI/BTTask_Attack.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Interaction/EnemyInterface.h"

EBTNodeResult::Type UBTTask_Attack::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	AAIController* Controller = OwnerComp.GetAIOwner();
	APawn* ControlledPawn = Controller->GetPawn();
	
	UBlackboardComponent* BlackboardComp = OwnerComp.GetBlackboardComponent();
	if (!BlackboardComp) return EBTNodeResult::Failed;
	
	AActor* CombatTarget = Cast<AActor>(BlackboardComp->GetValueAsObject(CombatTargetSelector.SelectedKeyName));
	if (!CombatTarget) return EBTNodeResult::Failed;
	
	IEnemyInterface::Execute_SetCombatTarget(ControlledPawn, CombatTarget);
	
	UAbilitySystemComponent* ASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(ControlledPawn);
	if (!ASC) return EBTNodeResult::Failed;
	
	FGameplayTagContainer TagContainer = FGameplayTagContainer(AttackTag);
	ASC->TryActivateAbilitiesByTag(TagContainer);
    		
	return EBTNodeResult::Succeeded;
}
