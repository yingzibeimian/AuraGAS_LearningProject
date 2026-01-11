// Copyright David He


#include "UI/WidgetController/OverlayWidgetController.h"

#include "AbilitySystem/AuraAbilitySystemComponent.h"
#include "AbilitySystem/AuraAttributeSet.h"
#include "AbilitySystem/Data/AbilityInfo.h"

void UOverlayWidgetController::BroadcastInitialValues()
{
	const UAuraAttributeSet* AuraAttributeSet = CastChecked<UAuraAttributeSet>(AttributeSet);
	
	OnHealthChanged.Broadcast(AuraAttributeSet->GetHealth());
	OnMaxHealthChanged.Broadcast(AuraAttributeSet->GetMaxHealth());
	OnManaChanged.Broadcast(AuraAttributeSet->GetMana());
	OnMaxManaChanged.Broadcast(AuraAttributeSet->GetMaxMana());
}

/// 监听数据变化
void UOverlayWidgetController::BindCallbacksToDependencies()
{
	const UAuraAttributeSet* AuraAttributeSet = CastChecked<UAuraAttributeSet>(AttributeSet);
	AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(AuraAttributeSet->GetHealthAttribute()).AddLambda(
			[this](const FOnAttributeChangeData& Data) 
			{
				OnHealthChanged.Broadcast(Data.NewValue);
			}
		);
	
	AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(AuraAttributeSet->GetMaxHealthAttribute()).AddLambda(
			[this](const FOnAttributeChangeData& Data) 
			{
				OnMaxHealthChanged.Broadcast(Data.NewValue);
			}
		);
	
	AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(AuraAttributeSet->GetManaAttribute()).AddLambda(
			[this](const FOnAttributeChangeData& Data) 
			{
				OnManaChanged.Broadcast(Data.NewValue);
			}
		);
	
	AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(AuraAttributeSet->GetMaxManaAttribute()).AddLambda(
			[this](const FOnAttributeChangeData& Data) 
			{
				OnMaxManaChanged.Broadcast(Data.NewValue);
			}
		);
	
	if (UAuraAbilitySystemComponent* AuraASC = Cast<UAuraAbilitySystemComponent>(AbilitySystemComponent))
	{
		// 绑定回调, 角色Ability数据添加完毕时, 向UI传输数据
		if (AuraASC->bStartupAbilitiesGiven)
		{
			// ASC先添加完角色Ability, OverlayWidgetController后执行BindCallbacksToDependencies, 此时绑定回调无意义, 直接调用
			OnInitializeStartupAbilities(AuraASC);
		}
		else
		{
			// OverlayWidgetController先绑定回调, ASC后添加完角色Ability
			AuraASC->AbilitiesGivenDelegate.AddUObject(this, &UOverlayWidgetController::OnInitializeStartupAbilities);
		}
	
		// 使用Lambda订阅多播委托EffectAssetTags
		AuraASC->EffectAssetTags.AddLambda(
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

void UOverlayWidgetController::OnInitializeStartupAbilities(UAuraAbilitySystemComponent* AuraAbilitySystemComponent)
{
	//TODO Get information about all given abilities, look up their Ability Info, and broadcast it to widgets.
	if (!AuraAbilitySystemComponent->bStartupAbilitiesGiven) return;
	
	FForEachAbility BroadcastDelegate;
	BroadcastDelegate.BindLambda([this, AuraAbilitySystemComponent](const FGameplayAbilitySpec& AbilitySpec)
	{
		//TODO need a way to figure out the ability tag for given ability spec.
		FAuraAbilityInfo Info = AbilityInfo->FindAbilityInfoForTag(UAuraAbilitySystemComponent::GetAbilityTagFromSpec(AbilitySpec));
		Info.InputTag = UAuraAbilitySystemComponent::GetInputTagFromSpec(AbilitySpec);
		AbilityInfoDelegate.Broadcast(Info);
	});
	AuraAbilitySystemComponent->ForEachAbility(BroadcastDelegate);
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
