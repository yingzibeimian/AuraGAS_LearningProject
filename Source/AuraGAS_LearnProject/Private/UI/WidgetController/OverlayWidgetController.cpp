// Copyright David He


#include "UI/WidgetController/OverlayWidgetController.h"
#include "AbilitySystem/AuraAbilitySystemComponent.h"
#include "AbilitySystem/AuraAttributeSet.h"
#include "AbilitySystem/Data/AbilityInfo.h"
#include "AbilitySystem/Data/LevelUpInfo.h"
#include "Player/AuraPlayerState.h"

void UOverlayWidgetController::BroadcastInitialValues()
{
	OnHealthChanged.Broadcast(GetAuraAS()->GetHealth());
	OnMaxHealthChanged.Broadcast(GetAuraAS()->GetMaxHealth());
	OnManaChanged.Broadcast(GetAuraAS()->GetMana());
	OnMaxManaChanged.Broadcast(GetAuraAS()->GetMaxMana());
}

/// 监听数据变化
void UOverlayWidgetController::BindCallbacksToDependencies()
{
	GetAuraPS()->OnXPChangedDelegate.AddUObject(this, &UOverlayWidgetController::OnXPChanged);
	GetAuraPS()->OnLevelChangedDelegate.AddLambda(
		[this](int32 NewLevel)
		{
			OnPlayerLevelChangedDelegate.Broadcast(NewLevel);
		}
	);
	
	AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(GetAuraAS()->GetHealthAttribute()).AddLambda(
			[this](const FOnAttributeChangeData& Data) 
			{
				OnHealthChanged.Broadcast(Data.NewValue);
			}
		);
	
	AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(GetAuraAS()->GetMaxHealthAttribute()).AddLambda(
			[this](const FOnAttributeChangeData& Data) 
			{
				OnMaxHealthChanged.Broadcast(Data.NewValue);
			}
		);
	
	AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(GetAuraAS()->GetManaAttribute()).AddLambda(
			[this](const FOnAttributeChangeData& Data) 
			{
				OnManaChanged.Broadcast(Data.NewValue);
			}
		);
	
	AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(GetAuraAS()->GetMaxManaAttribute()).AddLambda(
			[this](const FOnAttributeChangeData& Data) 
			{
				OnMaxManaChanged.Broadcast(Data.NewValue);
			}
		);
	
	if (GetAuraASC())
	{
		// 绑定回调, 角色Ability数据添加完毕时, 向UI传输数据
		if (GetAuraASC()->bStartupAbilitiesGiven)
		{
			// ASC先添加完角色Ability, OverlayWidgetController后执行BindCallbacksToDependencies, 此时绑定回调无意义, 直接调用
			BroadcastAbilityInfo();
		}
		else
		{
			// OverlayWidgetController先绑定回调, ASC后添加完角色Ability
			GetAuraASC()->AbilitiesGivenDelegate.AddUObject(this, &UOverlayWidgetController::BroadcastAbilityInfo);
		}
	
		// 使用Lambda订阅多播委托EffectAssetTags
		GetAuraASC()->EffectAssetTags.AddLambda(
			// [this]: capture this (the object that we're in)
			[this](const FGameplayTagContainer& AssetTags)
			{
				for (const FGameplayTag& Tag : AssetTags)
				{
					// For example, say that Tag = Message.HealthPotion
					// "Message.HealthPotion".MatchesTag("Message") will return True, "Message".MatchesTag("Message.HealthPotion") will return False
					FGameplayTag MessageTag = FGameplayTag::RequestGameplayTag(FName("Message"));
					if (Tag.MatchesTag(MessageTag))
					{
						const FUIWidgetRow* Row = GetDataTableRowByTag<FUIWidgetRow>(MessageWidgetDataTable, Tag);
						MessageWidgetRowDelegate.Broadcast(*Row);
					}
				}
			}
		);
	}
}

void UOverlayWidgetController::OnXPChanged(int32 NewXP)
{
	//const AAuraPlayerState* AuraPlayerState = Cast<AAuraPlayerState>(PlayerState); // static cast, no need to worry about performance
	const ULevelUpInfo* LevelUpInfo = GetAuraPS()->LevelUpInfo;
	checkf(LevelUpInfo, TEXT("Unabled to find LevelUpInfo. Please fill out AuraPlayerState Blueprint"));
	
	const int32 Level = LevelUpInfo->FindLevelForXP(NewXP);
	const int32 MaxLevel = LevelUpInfo->LevelUpInformation.Num();
	if (Level <= MaxLevel && Level > 0)
	{
		const int32 LevelUpRequirement = LevelUpInfo->LevelUpInformation[Level].LevelUpRequirement;
		const int32 PreviousLevelUpRequirement = LevelUpInfo->LevelUpInformation[Level - 1].LevelUpRequirement;
		
		const int32 DeltaLevelRequirement = LevelUpRequirement - PreviousLevelUpRequirement;
		const int32 XPForThisLevel = NewXP - PreviousLevelUpRequirement;
		
		const float XPBarPercent = static_cast<float>(XPForThisLevel) / static_cast<float>(DeltaLevelRequirement);
		OnXPPercentChangedDelegate.Broadcast(XPBarPercent);
	}
}



// TODO(GAS Ability Failure UI Feedback):
//
// Bind to AbilitySystemComponent->AbilityFailedCallbacks to handle ability activation
// failures that occur before ActivateAbility() (e.g. Cooldown, Cost).
//
// Architecture:
// - Use FailureTags as the semantic reason for ability activation failure.
// - Map FailureTag -> FailureFeedbackData (DataAsset).
// - FailureFeedbackData contains:
//     - FailureTag
//     - DisplayText (FText)
//     - TextColor
//     - UISound
//
// Flow:
// 1. AbilitySystemComponent broadcasts AbilityFailedCallbacks(Ability, FailureTags).
// 2. Controller receives the callback and looks up FailureFeedbackData by FailureTag.
// 3. Controller broadcasts a lightweight UI event with the resolved feedback data.
// 4. UI binds to the event, displays floating text above character, applies text color,
//    and plays the provided UI sound.
//
// Rules:
// - Do NOT inspect cooldown or cost GameplayEffects directly.
// - Do NOT place failure feedback logic inside GameplayAbility.
// - UI layer must remain data-driven and unaware of GAS internals.
//
// Note:
// UI failure feedback sounds are local-only and should be played directly by UI.
// GameplayCues are intentionally NOT used here, as ability activation failures
// do not represent gameplay state changes.
