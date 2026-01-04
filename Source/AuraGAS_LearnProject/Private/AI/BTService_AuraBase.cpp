// Copyright David He


#include "AI/BTService_AuraBase.h"

#include "BlueprintNodeHelpers.h"

UBTService_AuraBase::UBTService_AuraBase(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	const UClass* StopAtClass = GetClass();
	
	// Aura services always tick
	bNotifyTick = true;
	bNotifyOnSearch = true;

	// Match Blueprint service behavior
	bCreateNodeInstance = true;
	
	bShowPropertyDetails = true;
	if (HasAnyFlags(RF_ClassDefaultObject))
	{
		BlueprintNodeHelpers::CollectPropertyData(this, StopAtClass, PropertyData);
	}
}

FString UBTService_AuraBase::GetStaticServiceDescription() const
{
	FString ReturnDesc = Super::GetStaticServiceDescription();

	UBTService_AuraBase* CDO = (UBTService_AuraBase*)(GetClass()->GetDefaultObject());
	if (CDO)
	{
		if (bShowPropertyDetails)
		{
			UClass* StopAtClass = UBTService_AuraBase::StaticClass();
			FString PropertyDesc = BlueprintNodeHelpers::CollectPropertyDescription(this, StopAtClass, CDO->PropertyData);
			if (PropertyDesc.Len())
			{
				ReturnDesc += TEXT("\n");
				ReturnDesc += PropertyDesc;
			}
		}
	}

	return ReturnDesc;
}
