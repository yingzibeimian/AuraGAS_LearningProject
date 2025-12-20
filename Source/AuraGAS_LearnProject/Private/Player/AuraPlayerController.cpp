// Copyright David He


#include "Player/AuraPlayerController.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AuraGameplayTags.h"
#include "EnhancedInputSubsystems.h"
#include "AbilitySystem/AuraAbilitySystemComponent.h"
#include "Components/SplineComponent.h"
#include "Input/AuraInputComponent.h"
#include "Interaction/EnemyInterface.h"

AAuraPlayerController::AAuraPlayerController()
{
	// AuraPlayerController 必须开启复制replication，因为它是 GAS 激活流程、输入同步、UI 更新、目标选择等所有客户端→服务器通信的核心对象
	// 不开复制会导致 RPC、属性同步、HUD 更新全部失效
	// Replication is responding to data updating on the server and sending that down to clients
	bReplicates = true;
	
	Spline = CreateDefaultSubobject<USplineComponent>("Spline");
}

void AAuraPlayerController::PlayerTick(float DeltaTime)
{
	Super::PlayerTick(DeltaTime);
	
	CursorTrace();
}

// 追踪光标, 检测Enemy并高亮
void AAuraPlayerController::CursorTrace()
{
	FHitResult CursorHit;
	GetHitResultUnderCursor(ECollisionChannel::ECC_Visibility, false, CursorHit);
	if (!CursorHit.bBlockingHit) return;
	
	LastActor = ThisActor;
	ThisActor = Cast<IEnemyInterface>(CursorHit.GetActor());
	
	/** 
	 * Line trace from cursor. There are several scenarios:
	 *  A. LastActor is null && ThisActor is null
	 *		- Do nothing
	 *	B. LastActor is null && ThisActor is valid
	 *		- Highlight ThisActor
	 *	C. LastActor is valid && ThisActor is null
	 *		- UnHighlight LastActor
	 *	D. Both actors are valid, but LastActor != ThisActor
	 *		- UnHighlight LastActor, and Highlight ThisActor
	 *	E. Both actors are valid, and are the same actor
	 *		- Do nothing
	 */
	
	if (LastActor == nullptr && ThisActor != nullptr)
	{
		// Case B
		ThisActor->HighlightActor();
	}
	else if (LastActor != nullptr && ThisActor == nullptr)
	{
		// Case C
		LastActor->UnHighlightActor();
	}
	else if (LastActor != nullptr && ThisActor != nullptr && LastActor != ThisActor)
	{
		// Case D
		LastActor->HighlightActor();
		ThisActor->UnHighlightActor();
	}
	
}

void AAuraPlayerController::AbilityInputTagPressed(FGameplayTag InputTag)
{
	if (InputTag.MatchesTagExact(FAuraGameplayTags::Get().InputTag_LMB))
	{
		bTargeting = ThisActor ? true : false;
		bAutoRunning = false;	
	}
}

void AAuraPlayerController::AbilityInputTagReleased(FGameplayTag InputTag)
{
	if (GetASC() == nullptr) return;
	GetASC()->AbilityInputTagReleased(InputTag);
}

void AAuraPlayerController::AbilityInputTagHeld(FGameplayTag InputTag)
{
	if (!InputTag.MatchesTagExact(FAuraGameplayTags::Get().InputTag_LMB))
	{
		if (GetASC()) // 防止调用时机过早
		{
			GetASC()->AbilityInputTagHeld(InputTag);
		}
		return;
	}
	
	if (bTargeting)
	{
		if (GetASC())
		{
			GetASC()->AbilityInputTagHeld(InputTag);
		}
	}
	else
	{
		FollowTime += GetWorld()->GetDeltaSeconds();
		
		FHitResult Hit;
		if (GetHitResultUnderCursor(ECC_Visibility, false, Hit))
		{
			CachedDestination = Hit.ImpactPoint;
			if (APawn* ControlledPawn = GetPawn())
			{
				const FVector WorldDirection = (CachedDestination - ControlledPawn->GetActorLocation()).GetSafeNormal();
				ControlledPawn->AddMovementInput(WorldDirection);
			}
		}
	}
}

UAuraAbilitySystemComponent* AAuraPlayerController::GetASC()
{
	if (AuraAbilitySystemComponent == nullptr)
	{
		AuraAbilitySystemComponent = Cast<UAuraAbilitySystemComponent>(UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(GetPawn<APawn>()));
	}
	return AuraAbilitySystemComponent;
}

void AAuraPlayerController::BeginPlay()
{
	Super::BeginPlay();
	check(AuraContext); // AuraContext若不存在, 则让游戏崩溃
	
	// Subsystem is singletons, there exists only one for the duration of the program
	// 这里用 if 而不是 check：
	// 因为 AAuraPlayerController 的 BeginPlay 不仅会在“本地客户端”执行，
	// 还会在“服务器上的 PlayerController”上执行。
	// 服务器也有 PlayerController，但服务器永远没有 LocalPlayer，
	// 因此在服务器上：GetLocalPlayer() == nullptr 是完全正常情况。
	// 这里的逻辑本质是：
	//   “只在真正拥有本地玩家的那一台客户端上初始化输入系统”。
	// 如果这里用 check，服务器必然崩溃。
	UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer());
	if (Subsystem)
	{
		Subsystem->AddMappingContext(AuraContext, 0);
	}
	
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

void AAuraPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();
	
	// 可见Project Settings-Input-Default Classes
	UAuraInputComponent* AuraInputComponent = CastChecked<UAuraInputComponent>(InputComponent);
	AuraInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &AAuraPlayerController::Move);
	AuraInputComponent->BindAbilityActions(InputConfig, this, &ThisClass::AbilityInputTagPressed, &ThisClass::AbilityInputTagReleased, &ThisClass::AbilityInputTagHeld);
}


/// 移动函数
/// @param InputActionValue 使用 const 引用（const FInputActionValue&）避免拷贝，提高效率，同时保证函数内部不会修改传入的输入值（输入是只读的）
void AAuraPlayerController::Move(const FInputActionValue& InputActionValue)
{
	const FVector2D InputAxisVector = InputActionValue.Get<FVector2D>();
	// 控制器朝向 = 摄像机朝向（玩家视角旋转）
	const FRotator CameraRotation = GetControlRotation();
	// 只取 Yaw（水平旋转），防止上下看时移动方向跟着倾斜
	const FRotator YawRotation(0.f, CameraRotation.Yaw, 0.f);
	// 根据控制器朝向构建旋转矩阵，获取前(X)和右(Y)的世界方向单位向量
	const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
	const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);
	
	if (APawn* ControlledPawn = GetPawn<APawn>())
	{
		ControlledPawn->AddMovementInput(ForwardDirection, InputAxisVector.Y);
		ControlledPawn->AddMovementInput(RightDirection, InputAxisVector.X);
	}
}
