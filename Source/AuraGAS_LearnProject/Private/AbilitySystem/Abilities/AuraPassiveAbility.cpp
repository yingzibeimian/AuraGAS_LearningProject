// Copyright David He


#include "AbilitySystem/Abilities/AuraPassiveAbility.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystem/AuraAbilitySystemComponent.h"

void UAuraPassiveAbility::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
                                          const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
                                          const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);
	
	if (UAuraAbilitySystemComponent* AuraASC = Cast<UAuraAbilitySystemComponent>(UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(GetAvatarActorFromActorInfo())))
	{
		AuraASC->DeactivatePassiveAbility.AddUObject(this, &UAuraPassiveAbility::ReceiveDeactivate);
		AuraASC->PassiveAbilityLevelUp.AddUObject(this, &UAuraPassiveAbility::ReceiveUpgrade);
	}
	ApplyPassiveEffectToSelf();
}

void UAuraPassiveAbility::EndAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
	bool bReplicateEndAbility, bool bWasCancelled)
{
	if (UAuraAbilitySystemComponent* AuraASC = Cast<UAuraAbilitySystemComponent>(GetAbilitySystemComponentFromActorInfo()))
    {
    	AuraASC->DeactivatePassiveAbility.RemoveAll(this);
    	AuraASC->PassiveAbilityLevelUp.RemoveAll(this);
    }
	RemovePassiveEffectHandleFromSelf();
	
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

void UAuraPassiveAbility::ApplyPassiveEffectToSelf()
{
	UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo();
	if (!ASC || !PassiveEffectClass) return;
	
	FGameplayEffectContextHandle EffectContext = ASC->MakeEffectContext();
	EffectContext.AddSourceObject(GetAvatarActorFromActorInfo());
	
	FGameplayEffectSpecHandle SpecHandle = ASC->MakeOutgoingSpec(PassiveEffectClass, 1.f, EffectContext);
	if (!SpecHandle.IsValid()) return;
	for (const TPair<FGameplayTag, FScalableFloat>& Pair : SetByCallerMagnitudes)
	{
		UAbilitySystemBlueprintLibrary::AssignTagSetByCallerMagnitude(SpecHandle, Pair.Key, Pair.Value.GetValueAtLevel(GetAbilityLevel()));
	}
	ActiveEffectHandle = ASC->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data);
}

void UAuraPassiveAbility::RemovePassiveEffectHandleFromSelf()
{
	UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo();
	if (!ASC || !ActiveEffectHandle.IsValid()) return;
	
	ASC->RemoveActiveGameplayEffect(ActiveEffectHandle);
}

void UAuraPassiveAbility::ReceiveUpgrade(const FGameplayTag& AbilityTag)
{
	if (AbilityTags.HasTagExact(AbilityTag))
	{
		RemovePassiveEffectHandleFromSelf();
		ApplyPassiveEffectToSelf();
	}
}

void UAuraPassiveAbility::ReceiveDeactivate(const FGameplayTag& AbilityTag)
{
	if (AbilityTags.HasTagExact(AbilityTag))
	{
		EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
	}
}
