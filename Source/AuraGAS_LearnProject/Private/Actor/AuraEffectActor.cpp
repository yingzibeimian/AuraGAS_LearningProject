// Copyright David He


#include "Actor/AuraEffectActor.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"

AAuraEffectActor::AAuraEffectActor()
{
	PrimaryActorTick.bCanEverTick = false;
	
	SetRootComponent(CreateDefaultSubobject<USceneComponent>("SceneRoot"));
}

/// 开始 Overlap 回调
/// @param OverlappedComponent 自己的触发器
/// @param OtherActor 触发Overlap 的对方物体
/// @param OtherComp 对方触发 Overlap 的具体组件
/// @param OtherBodyIndex 物理系统用的索引
/// @param bFromSweep 该 Overlap 是否是因为物体“运动过程中”产生的
/// @param SweepResult 命中结果
/**
void AAuraEffectActor::OnOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
                                 UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (IAbilitySystemInterface* ASCInterface = Cast<IAbilitySystemInterface>(OtherActor))
	{
		//TODO: Change this to apply a Gameplay Effect. For now, using const_cast as a hack!
		const UAuraAttributeSet* AuraAttributeSet = Cast<UAuraAttributeSet>(ASCInterface->GetAbilitySystemComponent()->GetAttributeSet(UAuraAttributeSet::StaticClass()));
		// Should not set the variables directly on the AttributeSet(or its subclass), 
		// AttributeSet should either set its own attribute values or change values in respond to GameplayEffect
		// For learning purpose, we cast away the constantness of "AuraAttributeSet" and will fix this up to have a better design later
		//AuraAttributeSet->SetHealth(AuraAttributeSet->GetHealth() + 25.f);
		UAuraAttributeSet* MutableAuraAttributeSet = const_cast<UAuraAttributeSet*>(AuraAttributeSet);
		MutableAuraAttributeSet->SetHealth(AuraAttributeSet->GetHealth() + 25.f);
		MutableAuraAttributeSet->SetMana(AuraAttributeSet->GetMana() - 25.f);
		Destroy();
	}
}
*/

void AAuraEffectActor::BeginPlay()
{
	Super::BeginPlay();
	
}

/*
void AAuraEffectActor::ApplyEffectToTarget(AActor* TargetActor, TSubclassOf<UGameplayEffect> GameplayEffectClass)
{
	// if (IAbilitySystemInterface* ASCInterface = Cast<IAbilitySystemInterface>(Target))
	// {
	// 	ASCInterface->GetAbilitySystemComponent();
	// }
	check(GameplayEffectClass);
	if (UAbilitySystemComponent* TargetASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(TargetActor))
	{
		FGameplayEffectContextHandle EffectContextHandle = TargetASC->MakeEffectContext();
		EffectContextHandle.AddSourceObject(this);
		const FGameplayEffectSpecHandle EffectSpecHandle = TargetASC->MakeOutgoingSpec(GameplayEffectClass, 1.f, EffectContextHandle);
		//ApplyGameplayEffectSpecToSelf: apply GE to TargetASC; ApplyGameplayEffectSpecToTarget: TargetASC apply GE to other ASC
		FActiveGameplayEffectHandle ActiveEffectHandle = TargetASC->ApplyGameplayEffectSpecToSelf(*EffectSpecHandle.Data.Get());
		
		// 如果DurationType为Infinite, 则存储到Map中, 方便后续Remove
		const bool bIsInfinite = EffectSpecHandle.Data.Get()->Def.Get()->DurationPolicy == EGameplayEffectDurationType::Infinite;
		if (bIsInfinite && InfiniteEffectRemovePolicy == EEffectRemovePolicy::RemoveOnEndOverlap)
		{
			ActiveEffectHandles.Add(ActiveEffectHandle, TargetASC);
		}
	}
}
*/

void AAuraEffectActor::ApplyEffectToTarget(AActor* TargetActor, const FEffectConfig& Config)
{
	TSubclassOf<UGameplayEffect> GameplayEffectClass = Config.GameplayEffectClass;
	check(GameplayEffectClass);
	if (UAbilitySystemComponent* TargetASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(TargetActor))
	{
		FGameplayEffectContextHandle EffectContextHandle = TargetASC->MakeEffectContext();
		EffectContextHandle.AddSourceObject(this);
		const FGameplayEffectSpecHandle EffectSpecHandle = TargetASC->MakeOutgoingSpec(GameplayEffectClass, ActorLevel, EffectContextHandle);
		//ApplyGameplayEffectSpecToSelf: apply GE to TargetASC; ApplyGameplayEffectSpecToTarget: TargetASC apply GE to other ASC
		FActiveGameplayEffectHandle ActiveEffectHandle = TargetASC->ApplyGameplayEffectSpecToSelf(*EffectSpecHandle.Data.Get());
		
		// 如果DurationType为Infinite, 则存储到Map中, 方便后续Remove
		const bool bIsInfinite = EffectSpecHandle.Data.Get()->Def.Get()->DurationPolicy == EGameplayEffectDurationType::Infinite;
		if (bIsInfinite && Config.RemovePolicy == EEffectRemovePolicy::RemoveOnEndOverlap)
		{
			ActiveEffectHandles.Add(ActiveEffectHandle, TargetASC);
		}
	}
}

void AAuraEffectActor::ApplyEffectToTargetFromArray(AActor* TargetActor, const TArray<FEffectConfig>& GameplayEffectClasses,
	EEffectApplicationPolicy EffectApplicationPolicy)
{
	if (TargetActor->ActorHasTag(FName("Enemy")) && !bApplyEffectsToEnemies) return;
	
	for (const FEffectConfig& Config : GameplayEffectClasses)
	{
		if (Config.ApplicationPolicy ==  EffectApplicationPolicy)
		{
			ApplyEffectToTarget(TargetActor, Config);
		}
	}
}

void AAuraEffectActor::OnOverlap(AActor* TargetActor)
{
	if (TargetActor->ActorHasTag(FName("Enemy")) && !bApplyEffectsToEnemies) return;
	
	// if (InstantEffectApplicationPolicy == EEffectApplicationPolicy::ApplyOnOverlap)
	// {
	// 	ApplyEffectToTarget(TargetActor, InstantGameplayEffectClass);
	// }
	// if (DurationEffectApplicationPolicy == EEffectApplicationPolicy::ApplyOnOverlap)
	// {
	// 	ApplyEffectToTarget(TargetActor, DurationGameplayEffectClass);
	// }
	// if (InfiniteEffectApplicationPolicy == EEffectApplicationPolicy::ApplyOnOverlap)
	// {
	// 	ApplyEffectToTarget(TargetActor, InfiniteGameplayEffectClass);
	// }
	ApplyEffectToTargetFromArray(TargetActor, InstantGameplayEffectClasses, EEffectApplicationPolicy::ApplyOnOverlap);
	ApplyEffectToTargetFromArray(TargetActor, DurationGameplayEffectClasses, EEffectApplicationPolicy::ApplyOnOverlap);
	ApplyEffectToTargetFromArray(TargetActor, InfiniteGameplayEffectClasses, EEffectApplicationPolicy::ApplyOnOverlap);
	
	if (DestroyPolicy == EEffectActorDestroyPolicy::AfterApplyOnOverlap) Destroy();
}

void AAuraEffectActor::OnEndOverlap(AActor* TargetActor)
{
	if (TargetActor->ActorHasTag(FName("Enemy")) && !bApplyEffectsToEnemies) return;
	
	// Apply Effect
	// if (InstantEffectApplicationPolicy == EEffectApplicationPolicy::ApplyOnEndOverlap)
	// {
	// 	ApplyEffectToTarget(TargetActor, InstantGameplayEffectClass);
	// }
	// if (DurationEffectApplicationPolicy == EEffectApplicationPolicy::ApplyOnEndOverlap)
	// {
	// 	ApplyEffectToTarget(TargetActor, DurationGameplayEffectClass);
	// }
	// if (InfiniteEffectApplicationPolicy == EEffectApplicationPolicy::ApplyOnEndOverlap)
	// {
	// 	ApplyEffectToTarget(TargetActor, InfiniteGameplayEffectClass);
	// }
	ApplyEffectToTargetFromArray(TargetActor, InstantGameplayEffectClasses, EEffectApplicationPolicy::ApplyOnEndOverlap);
	ApplyEffectToTargetFromArray(TargetActor, DurationGameplayEffectClasses, EEffectApplicationPolicy::ApplyOnEndOverlap);
	ApplyEffectToTargetFromArray(TargetActor, InfiniteGameplayEffectClasses, EEffectApplicationPolicy::ApplyOnEndOverlap);
	// Remove Effect
	// if (InfiniteEffectRemovePolicy == EEffectRemovePolicy::RemoveOnEndOverlap)
	// {
	// 	UAbilitySystemComponent* TargetASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(TargetActor);
	// 	if (!IsValid(TargetASC)) return;
	// 	
	// 	TArray<FActiveGameplayEffectHandle> HandlesToRemove;
	// 	for (const TPair<FActiveGameplayEffectHandle, UAbilitySystemComponent*>& HandlePair : ActiveEffectHandles)
	// 	{
	// 		if (TargetASC == HandlePair.Value)
	// 		{
	// 			TargetASC->RemoveActiveGameplayEffect(HandlePair.Key, 1);
	// 			HandlesToRemove.Add(HandlePair.Key);
	// 		}
	// 	}
	// 	for (const FActiveGameplayEffectHandle& Handle : HandlesToRemove)
	// 	{
	// 		ActiveEffectHandles.Remove(Handle);
	// 	}
	// }
	
	UAbilitySystemComponent* TargetASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(TargetActor);
	if (!IsValid(TargetASC)) return;
	TArray<FActiveGameplayEffectHandle> HandlesToRemove;
	for (const TPair<FActiveGameplayEffectHandle, UAbilitySystemComponent*>& HandlePair : ActiveEffectHandles)
	{
		if (TargetASC == HandlePair.Value)
		{
			TargetASC->RemoveActiveGameplayEffect(HandlePair.Key, 1);
			HandlesToRemove.Add(HandlePair.Key);
		}
	}
	for (const FActiveGameplayEffectHandle& Handle : HandlesToRemove)
	{
		ActiveEffectHandles.Remove(Handle);
	}
	
	if (DestroyPolicy == EEffectActorDestroyPolicy::AfterApplyOnEndOverlap) Destroy();
	
	// TODO: Multiple Effects (Arrays of each Duration Type, and how to apply them and remove the Infinite Effect)
}

#if WITH_EDITOR
void AAuraEffectActor::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);
	
	switch (DestroyPolicy)
	{
	case EEffectActorDestroyPolicy::AfterApplyOnOverlap:
		{
			for (const FEffectConfig& Config : InstantGameplayEffectClasses)
			{
				if (Config.ApplicationPolicy == EEffectApplicationPolicy::ApplyOnEndOverlap)
				{
					UE_LOG(LogTemp, Warning,
						TEXT("EffectActor %s: DestroyPolicy=AfterApplyOnOverlap but has Instant GE with ApplyOnEndOverlap"),
						*GetName());
				}
			}
			for (const FEffectConfig& Config : DurationGameplayEffectClasses)
			{
				if (Config.ApplicationPolicy == EEffectApplicationPolicy::ApplyOnEndOverlap)
				{
					UE_LOG(LogTemp, Warning,
						TEXT("EffectActor %s: DestroyPolicy=AfterApplyOnOverlap but has Duration GE with ApplyOnEndOverlap"),
						*GetName());
				}
			}
			break;
		}
	case EEffectActorDestroyPolicy::AfterApplyOnEndOverlap:
		{
			for (const FEffectConfig& Config : InstantGameplayEffectClasses)
			{
				if (Config.ApplicationPolicy == EEffectApplicationPolicy::ApplyOnOverlap)
				{
					UE_LOG(LogTemp, Warning,
						TEXT("EffectActor %s: DestroyPolicy=AfterApplyOnEndOverlap but has Instant GE with ApplyOnOverlap"),
						*GetName());
				}
			}
			for (const FEffectConfig& Config : DurationGameplayEffectClasses)
			{
				if (Config.ApplicationPolicy == EEffectApplicationPolicy::ApplyOnOverlap)
				{
					UE_LOG(LogTemp, Warning,
						TEXT("EffectActor %s: DestroyPolicy=AfterApplyOnEndOverlap but has Duration GE with ApplyOnOverlap"),
						*GetName());
				}
			}
			break;
		}
	default:
		break;
	}
}
#endif
