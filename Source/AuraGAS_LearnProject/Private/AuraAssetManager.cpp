// Copyright David He


#include "AuraAssetManager.h"

#include "AbilitySystemGlobals.h"
#include "AuraGameplayTags.h"

UAuraAssetManager& UAuraAssetManager::Get()
{
	check(GEngine);
	
	UAuraAssetManager* AuraAssetManager = Cast<UAuraAssetManager>(GEngine->AssetManager);
	return *AuraAssetManager;
}

void UAuraAssetManager::StartInitialLoading()
{
	Super::StartInitialLoading();
	FAuraGameplayTags::Get().InitializeNativeGameplayTags();
	// 重点在于InitGlobalData()中的InitTargetDataScriptStructCache(), because this is required to ues Target Data (such as TargetDataUnderMouse.cpp)
	UAbilitySystemGlobals::Get().InitGlobalData();
}
