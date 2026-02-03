// Copyright David He

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "EnemyInterface.generated.h"

// This class does not need to be modified.
UINTERFACE(MinimalAPI)
class UEnemyInterface : public UInterface
{
	GENERATED_BODY()

	// UObject 系列不能有纯虚函数, 而 UE 需要一个 UObject 来对接口做 RTTI、反射、序列化
	// UEnemyInterface用于 UE 的反射系统, 使接口能在 Blueprint 中使用
};

/**
 * 
 */
class AURAGAS_LEARNPROJECT_API IEnemyInterface
{
	GENERATED_BODY()
	
	// IEnemyInterface则是是真正 写函数、写逻辑、类来继承的接口本体

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:
	// the class IEnemyInterface is considered abstract
	// Any class that implements this interface will be required to override these pure virtual function
	
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	void SetCombatTarget(AActor* InCombatTarget);
	
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
    AActor* GetCombatTarget() const;
};
