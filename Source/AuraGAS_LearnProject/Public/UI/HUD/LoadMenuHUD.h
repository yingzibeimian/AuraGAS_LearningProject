// Copyright David He

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "LoadMenuHUD.generated.h"

class UMVVM_LoadMenu;
class ULoadMenuWidget;
/**
 * 
 */
UCLASS()
class AURAGAS_LEARNPROJECT_API ALoadMenuHUD : public AHUD
{
	GENERATED_BODY()
public:
		
	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UUserWidget> LoadMenuWidgetClass;
	
	UPROPERTY(BlueprintReadOnly)
	TObjectPtr<ULoadMenuWidget> LoadMenuWidget;
	
	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UMVVM_LoadMenu> LoadMenuViewModelClass;

	UPROPERTY(BlueprintReadOnly)
	TObjectPtr<UMVVM_LoadMenu> LoadMenuViewModel;
	
protected:
	virtual void BeginPlay() override;	
};
