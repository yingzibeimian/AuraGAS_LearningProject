// Copyright David He


#include "UI/WidgetController/SpellMenuWidgetController.h"
#include "AuraGameplayTags.h"
#include "AbilitySystem/AuraAbilitySystemComponent.h"
#include "AbilitySystem/Data/AbilityInfo.h"
#include "Player/AuraPlayerState.h"

void USpellMenuWidgetController::BroadcastInitialValues()
{
	BroadcastAbilityInfo();
	SpellPointsChanged.Broadcast(GetAuraPS()->GetSpellPoints());
}

void USpellMenuWidgetController::BindCallbacksToDependencies()
{
	GetAuraASC()->AbilityStatusChanged.AddLambda([this](const FGameplayTag& AbilityTag, const FGameplayTag& StatusTag, int32 NewLevel)
	{
		// 由于升级时, AbilityStatusChanged和OnSpellPointsChangedDelegate都会广播, 不确定这里两个Lambda在服务器和客户端的执行顺序
		// 如果从ASC获取Status, 以及从PS获取SpellPoints不一定正确
		// 因此用本地变量SelectedAbility和CurrentSpellPoints跟踪所选择SpellGlobe对应的技能Status和正确的SpellPoints
		if (SelectedAbility.Ability.MatchesTagExact(AbilityTag))
		{
			SelectedAbility.Status = StatusTag;
			bool bEnableSpellPointsButton = false;
			bool bEnableEquipButton = false;
			ShouldEnableButtons(StatusTag, CurrentSpellPoints, bEnableSpellPointsButton, bEnableEquipButton);
			SpellGlobeSelectedDelegate.Broadcast(bEnableSpellPointsButton, bEnableEquipButton);
		}
		
		if (AbilityInfo)
		{
			FAuraAbilityInfo Info = AbilityInfo->FindAbilityInfoForTag(AbilityTag);
			Info.StatusTag = StatusTag;
			AbilityInfoDelegate.Broadcast(Info);
		}
	});
	
	GetAuraPS()->OnSpellPointsChangedDelegate.AddLambda([this](int32 NewSpellPoints)
	{
		SpellPointsChanged.Broadcast(NewSpellPoints);
		CurrentSpellPoints = NewSpellPoints;
		
		bool bEnableSpellPointsButton = false;
        bool bEnableEquipButton = false;
        ShouldEnableButtons(SelectedAbility.Status, CurrentSpellPoints, bEnableSpellPointsButton, bEnableEquipButton);
        SpellGlobeSelectedDelegate.Broadcast(bEnableSpellPointsButton, bEnableEquipButton);
	});
}

void USpellMenuWidgetController::SpellGlobeSelected(const FGameplayTag& AbilityTag)
{
	const FAuraGameplayTags GameplayTags = FAuraGameplayTags::Get();
	const int32 SpellPoints = GetAuraPS()->GetSpellPoints();
	FGameplayTag AbilityStatus;
	
	const bool bTagValid = AbilityTag.IsValid();
	const bool bTagNone = AbilityTag.MatchesTag(GameplayTags.Abilities_None);
	const FGameplayAbilitySpec* AbilitySpec = GetAuraASC()->GetSpecFromAbilityTag(AbilityTag);
	const bool bSpecValid = AbilitySpec != nullptr;
	if (!bTagValid || bTagNone || !bSpecValid)
	{
		AbilityStatus = GameplayTags.Abilities_Status_Locked; // Locked Status
	}
	else
	{
		AbilityStatus = GetAuraASC()->GetStatusFromSpec(*AbilitySpec); // Other Status
	}
	
	SelectedAbility.Ability = AbilityTag;
	SelectedAbility.Status = AbilityStatus;
	
	bool bEnableSpellPointsButton = false;
	bool bEnableEquipButton = false;
	ShouldEnableButtons(AbilityStatus, SpellPoints, bEnableSpellPointsButton, bEnableEquipButton);
	SpellGlobeSelectedDelegate.Broadcast(bEnableSpellPointsButton, bEnableEquipButton);
}

void USpellMenuWidgetController::SpendPontButtonPressed()
{
	if (GetAuraASC())
	{
		GetAuraASC()->ServerSpendSpellPoint(SelectedAbility.Ability);
	}
}

void USpellMenuWidgetController::ShouldEnableButtons(const FGameplayTag& AbilityStatus, int32 SpellPoints, bool& bShouldEnableSpellPointsButton, bool& bShouldEnableEquipButton)
{
	const FAuraGameplayTags GameplayTags = FAuraGameplayTags::Get();
	
	bShouldEnableSpellPointsButton = false;
	bShouldEnableEquipButton = false;
	if (AbilityStatus.MatchesTagExact(GameplayTags.Abilities_Status_Equipped))
	{
		bShouldEnableEquipButton = true;
		if (SpellPoints > 0)
		{
			bShouldEnableSpellPointsButton = true;
		}
	}
	else if (AbilityStatus.MatchesTagExact(GameplayTags.Abilities_Status_Eligible))
	{
		if (SpellPoints > 0)
		{
			bShouldEnableSpellPointsButton = true;
		}
	}
	else if (AbilityStatus.MatchesTagExact(GameplayTags.Abilities_Status_Unlocked))
	{
		bShouldEnableEquipButton = true;
		if (SpellPoints > 0)
		{
			bShouldEnableSpellPointsButton = true;
		}
	}
}
