// Copyright David He


#include "AI/BTTask_Attack_Elementalist.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Interaction/CombatInterface.h"
#include "Interaction/EnemyInterface.h"

EBTNodeResult::Type UBTTask_Attack_Elementalist::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	AAIController* Controller = OwnerComp.GetAIOwner();
	APawn* ControlledPawn = Controller->GetPawn();
	
	UBlackboardComponent* BlackboardComp = OwnerComp.GetBlackboardComponent();
	if (!BlackboardComp) return EBTNodeResult::Failed;
	
	AActor* CombatTarget = Cast<AActor>(BlackboardComp->GetValueAsObject(CombatTargetSelector.SelectedKeyName));
	if (!CombatTarget) return EBTNodeResult::Failed;
	
	if (!Cast<IEnemyInterface>(ControlledPawn)) return EBTNodeResult::Failed;
	IEnemyInterface::Execute_SetCombatTarget(ControlledPawn, CombatTarget);
	
	if (!Cast<ICombatInterface>(ControlledPawn)) return EBTNodeResult::Failed;
	if (ICombatInterface::Execute_GetMinionCount(ControlledPawn) < MinionSpawnThreshold)
	{
		AbilityTag = SummonTag;
	}
	else
	{
		AbilityTag = AttackTag;
	}
	
	UAbilitySystemComponent* ASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(ControlledPawn);
	if (!ASC) return EBTNodeResult::Failed;
	
	FGameplayTagContainer TagContainer = FGameplayTagContainer(AbilityTag);
	ASC->TryActivateAbilitiesByTag(TagContainer);
    		
	return EBTNodeResult::Succeeded;
}
