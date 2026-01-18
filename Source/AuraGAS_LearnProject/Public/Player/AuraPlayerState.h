// Copyright David He

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemInterface.h"
#include "GameFramework/PlayerState.h"
#include "AuraPlayerState.generated.h"

class UAbilitySystemComponent;
class UAttributeSet;
class ULevelUpInfo;

DECLARE_MULTICAST_DELEGATE_OneParam(FOnPlayerStatChanged, int32 /*StatValue*/)

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
	
	UPROPERTY(EditDefaultsOnly)
	TObjectPtr<ULevelUpInfo> LevelUpInfo;
	
	FOnPlayerStatChanged OnXPChangedDelegate;
	FOnPlayerStatChanged OnLevelChangedDelegate;
	FOnPlayerStatChanged OnAttributePointsChangedDelegate;
	FOnPlayerStatChanged OnSpellPointsChangedDelegate;
	
	// FORCEINLINE 关键优化：强制编译器在调用点直接展开函数体代码,避免函数调用开销（压栈/跳转/返回）,特别适用于高频调用的小函数
	FORCEINLINE int32 GetPlayerLevel() const { return Level; }
	FORCEINLINE int32 GetXP() const { return XP; }
	FORCEINLINE int32 GetAttributePoints() const { return AttributePoints; }
	FORCEINLINE int32 GetSpellPoints() const { return SpellPoints; }
	
	void AddToXp(int32 InXP);
	void AddToLevel(int32 InLevel);
	void AddToAttributePoints(int32 InPoints);
	void AddToSpellPoints(int32 InPoints);
	
	void SetXp(int32 InXP);
	void SetLevel(int32 InLevel);
	
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
	
	UPROPERTY(VisibleAnywhere, ReplicatedUsing = OnRep_XP)
	int32 XP = 1;
	UFUNCTION()
	void OnRep_XP(int32 OldXP);
	
	UPROPERTY(VisibleAnywhere, ReplicatedUsing = OnRep_AttributePoints)
	int32 AttributePoints = 0;
	UFUNCTION()
	void OnRep_AttributePoints(int32 OldAttributePoints);
	
	UPROPERTY(VisibleAnywhere, ReplicatedUsing = OnRep_SpellPoints)
	int32 SpellPoints = 0;
	UFUNCTION()
	void OnRep_SpellPoints(int32 OldSellPoints);
};
