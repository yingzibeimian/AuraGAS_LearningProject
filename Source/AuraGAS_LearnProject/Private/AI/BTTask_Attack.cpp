// Copyright David He


#include "AI/BTTask_Attack.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "AIController.h"

EBTNodeResult::Type UBTTask_Attack::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	AAIController* Controller = OwnerComp.GetAIOwner();
	APawn* ControlledPawn = Controller->GetPawn();
	
	FGameplayTagContainer TagContainer = FGameplayTagContainer(AttackTag);
	if (UAbilitySystemComponent* ASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(ControlledPawn))
	{
		ASC->TryActivateAbilitiesByTag(TagContainer);
		
		return EBTNodeResult::Succeeded;
	}
	
	return EBTNodeResult::Failed;
}
