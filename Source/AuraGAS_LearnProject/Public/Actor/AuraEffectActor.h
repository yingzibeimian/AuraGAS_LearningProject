// Copyright David He

#pragma once

#include "CoreMinimal.h"
#include "ActiveGameplayEffectHandle.h"
#include "GameFramework/Actor.h"
#include "AuraEffectActor.generated.h"


class UGameplayEffect;

UENUM(BlueprintType)
enum class EEffectActorDestroyPolicy
{
	Never UMETA(
		DisplayName = "Never",
		ToolTip = 
		"EffectActor will never be automatically destroyed.\nUse this if the actor applies Infinite Gameplay Effects."
	),

	AfterApplyOnOverlap UMETA(
		DisplayName = "After Apply On Overlap",
		ToolTip =
		"Destroy the EffectActor immediately after applying effects with ApplyOnOverlap.\n\nRequirements:\n- All applied effects must be Instant or Duration.\n\nNotes:\n- Effects using ApplyOnEndOverlap will not be applied."
	),

	AfterApplyOnEndOverlap UMETA(
		DisplayName = "After Apply On End Overlap",
		ToolTip =
		"Destroy the EffectActor after ApplyOnEndOverlap is processed.\n\nRequirements:\n- All applied effects must be Instant or Duration.\n\nNotes:\n- Even effects ApplyOnOverlap will only destroy the actor at EndOverlap."
	)
};

UENUM(BlueprintType)
enum class EEffectApplicationPolicy
{
	ApplyOnOverlap,
	ApplyOnEndOverlap,
	DoNotApply,
};

UENUM(BlueprintType)
enum class EEffectRemovePolicy
{
	RemoveOnEndOverlap,
	DoNotRemove,
};

USTRUCT(BlueprintType)
struct FEffectConfig
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TSubclassOf<UGameplayEffect> GameplayEffectClass;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	EEffectApplicationPolicy ApplicationPolicy = EEffectApplicationPolicy::DoNotApply;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	EEffectRemovePolicy RemovePolicy = EEffectRemovePolicy::DoNotRemove;
};

UCLASS()
class AURAGAS_LEARNPROJECT_API AAuraEffectActor : public AActor
{
	GENERATED_BODY()
	
public:	
	AAuraEffectActor();
	
protected:
	virtual void BeginPlay() override;
	
	// UFUNCTION(BlueprintCallable)
	// void ApplyEffectToTarget(AActor* TargetActor, TSubclassOf<UGameplayEffect> GameplayEffectClass);
	
	void ApplyEffectToTarget(AActor* TargetActor, const FEffectConfig& Config);
	void ApplyEffectToTargetFromArray(AActor* TargetActor, const TArray<FEffectConfig>& GameplayEffectClasses, EEffectApplicationPolicy EffectApplicationPolicy);

	UFUNCTION(BlueprintCallable)
	void OnOverlap(AActor* TargetActor);
	
	UFUNCTION(BlueprintCallable)
	void OnEndOverlap(AActor* TargetActor);
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Applied Effects")
	EEffectActorDestroyPolicy DestroyPolicy = EEffectActorDestroyPolicy::Never; //bool bDestroyOnEffectApplication = false;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Applied Effects")
	bool bApplyEffectsToEnemies = false;
	
	// UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Applied Effects")
	// TSubclassOf<UGameplayEffect> InstantGameplayEffectClass;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Applied Effects")
	TArray<FEffectConfig> InstantGameplayEffectClasses;
	
	// UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Applied Effects")
	// EEffectApplicationPolicy InstantEffectApplicationPolicy = EEffectApplicationPolicy::DoNotApply;
	
	// UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Applied Effects")
	// TSubclassOf<UGameplayEffect> DurationGameplayEffectClass;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Applied Effects")
	TArray<FEffectConfig> DurationGameplayEffectClasses;
	
	// UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Applied Effects")
	// EEffectApplicationPolicy DurationEffectApplicationPolicy = EEffectApplicationPolicy::DoNotApply;
	
	// UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Applied Effects")
	// TSubclassOf<UGameplayEffect> InfiniteGameplayEffectClass;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Applied Effects")
	TArray<FEffectConfig> InfiniteGameplayEffectClasses;
	
	// UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Applied Effects")
	// EEffectApplicationPolicy InfiniteEffectApplicationPolicy = EEffectApplicationPolicy::DoNotApply;
	//
	// UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Applied Effects")
	// EEffectRemovePolicy InfiniteEffectRemovePolicy = EEffectRemovePolicy::RemoveOnEndOverlap;
	
	TMap<FActiveGameplayEffectHandle, UAbilitySystemComponent*> ActiveEffectHandles;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Applied Effects")
	float ActorLevel = 1.f;
	
#if WITH_EDITOR
	// 声明编辑器专用的属性修改回调
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif
};
