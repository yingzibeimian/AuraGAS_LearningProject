// Copyright David He

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemInterface.h"
#include "GameFramework/PlayerState.h"
#include "AuraPlayerState.generated.h"

class UAbilitySystemComponent;
class UAttributeSet;

/**
 * 
 */
UCLASS()
class AURAGAS_LEARNPROJECT_API AAuraPlayerState : public APlayerState, public IAbilitySystemInterface
{
	GENERATED_BODY()
public:
	AAuraPlayerState();
	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;
	// 实现接口IAbilitySystemInterface内方法 AbilitySystemComponent Getter
	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;
	// AttributeSet Getter
	UAttributeSet* GetAttributeSet() const { return AttributeSet; }
	
	// FORCEINLINE 关键优化：强制编译器在调用点直接展开函数体代码,避免函数调用开销（压栈/跳转/返回）,特别适用于高频调用的小函数
	FORCEINLINE int32 GetPlayerLevel() const { return Level; }
	
protected:
	
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UAbilitySystemComponent> AbilitySystemComponent;

	UPROPERTY()
	TObjectPtr<UAttributeSet> AttributeSet;
	
private:

	UPROPERTY(VisibleAnywhere, ReplicatedUsing = OnRep_Level)
	int32 Level = 1;
	UFUNCTION()
	void OnRep_Level(int32 OldLevel);
};
