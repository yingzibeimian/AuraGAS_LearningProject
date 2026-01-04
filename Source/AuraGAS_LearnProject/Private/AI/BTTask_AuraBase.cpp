// Copyright David He


#include "AI/BTTask_AuraBase.h"

#include "BlueprintNodeHelpers.h"

UBTTask_AuraBase::UBTTask_AuraBase(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	const UClass* StopAtClass = GetClass();
	INIT_TASK_NODE_NOTIFY_FLAGS();
	
	bShowPropertyDetails = true;
	if (HasAnyFlags(RF_ClassDefaultObject))
	{
		BlueprintNodeHelpers::CollectPropertyData(this, StopAtClass, PropertyData);
	}
}

FString UBTTask_AuraBase::GetStaticDescription() const
{
	FString ReturnDesc = Super::GetStaticDescription();
	
	UBTTask_AuraBase* CDO = static_cast<UBTTask_AuraBase*>(GetClass()->GetDefaultObject());
	if (bShowPropertyDetails && CDO)
	{
		const UClass* StopAtClass = UBTTask_AuraBase::StaticClass();
		FString PropertyDesc = BlueprintNodeHelpers::CollectPropertyDescription(this, StopAtClass, CDO->PropertyData);
		if (PropertyDesc.Len())
		{
			ReturnDesc += TEXT(":\n\n");
			ReturnDesc += PropertyDesc;
		}
	}
	
	return ReturnDesc;
}
