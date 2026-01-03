// Copyright David He

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTService.h"
#include "BTService_AuraBase.generated.h"

/**
 * 
 */
UCLASS()
class AURAGAS_LEARNPROJECT_API UBTService_AuraBase : public UBTService
{
	GENERATED_UCLASS_BODY()
	
protected:
	// Gets the description for our service
	virtual FString GetStaticServiceDescription() const override;

	/** properties with runtime values, stored only in class default object */
	TArray<FProperty*> PropertyData;
	
	/** show detailed information about properties */
	UPROPERTY(EditInstanceOnly, Category=Description)
	uint32 bShowPropertyDetails : 1;
};
