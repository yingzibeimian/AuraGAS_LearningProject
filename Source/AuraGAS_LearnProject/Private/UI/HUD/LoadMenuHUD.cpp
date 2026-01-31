// Copyright David He


#include "UI/HUD/LoadMenuHUD.h"
#include "UI/ViewModel/MVVM_LoadMenu.h"
#include "UI/Widget/LoadMenuWidget.h"

void ALoadMenuHUD::BeginPlay()
{
	Super::BeginPlay();
	
	LoadMenuViewModel = NewObject<UMVVM_LoadMenu>(this, LoadMenuViewModelClass);
	LoadMenuViewModel->InitializeLoadSlots();
	
	LoadMenuWidget = CreateWidget<ULoadMenuWidget>(GetWorld(), LoadMenuWidgetClass);
	LoadMenuWidget->AddToViewport();
	LoadMenuWidget->BlueprintInitializeWidget();
	
	LoadMenuViewModel->LoadData();
}
