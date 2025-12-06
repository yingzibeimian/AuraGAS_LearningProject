// Copyright David He


#include "Player/AuraPlayerController.h"
#include "EnhancedInputSubsystems.h"

AAuraPlayerController::AAuraPlayerController()
{
	// AuraPlayerController 必须开启复制replication，因为它是 GAS 激活流程、输入同步、UI 更新、目标选择等所有客户端→服务器通信的核心对象
	// 不开复制会导致 RPC、属性同步、HUD 更新全部失效
	// Replication is responding to data updating on the server and sending that down to clients
	bReplicates = true;
}

void AAuraPlayerController::BeginPlay()
{
	Super::BeginPlay();
	check(AuraContext); // AuraContext若不存在, 则让游戏崩溃
	
	// Subsystem is singletons, there exists only one for the duration of the program
	UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer());
	check(Subsystem);
	Subsystem->AddMappingContext(AuraContext, 0);
	
	bShowMouseCursor = true;
	DefaultMouseCursor = EMouseCursor::Default;
	
	FInputModeGameAndUI InputModeData;
	// 设置鼠标锁定行为（DoNotLock = 鼠标不锁到屏幕中心，适合俯视角等游戏）
	InputModeData.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
	// 设置在按下鼠标时是否隐藏指针（false = 不隐藏）, Aura 是点击地面移动，需要一直看到鼠标，所以不隐藏
	InputModeData.SetHideCursorDuringCapture(false);
	// 应用输入模式，使游戏同时接收“游戏输入 + UI 输入”
	SetInputMode(InputModeData);
}
