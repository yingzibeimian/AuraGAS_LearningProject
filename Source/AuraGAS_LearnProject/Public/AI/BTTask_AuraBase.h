// Copyright David He

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BTTask_AuraBase.generated.h"

/**
 * 
 */
UCLASS()
class AURAGAS_LEARNPROJECT_API UBTTask_AuraBase : public UBTTaskNode
{
	GENERATED_UCLASS_BODY()
	
public:
	virtual FString GetStaticDescription() const override;
	
protected:
	
	/** show detailed information about properties */
	UPROPERTY(EditInstanceOnly, Category=Description)
	uint32 bShowPropertyDetails : 1;
	
	/** properties that should be copied */
	TArray<FProperty*> PropertyData;
};
