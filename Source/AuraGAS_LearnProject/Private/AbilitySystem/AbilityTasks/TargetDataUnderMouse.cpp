// Copyright David He

/*
整体思路说明（TargetDataUnderMouse）：

这是一个 AbilityTask，用于在“客户端本地”通过鼠标射线检测生成 TargetData，
并将该 TargetData 安全地同步到服务器，同时保证：

1. 客户端可以立即使用 TargetData（本地预测，不卡手感）
2. 服务器一定能拿到 TargetData（权威逻辑）
3. 不论 Activate RPC 还是 TargetData RPC 谁先到服务器，都不会丢数据（解决竞态问题）

核心问题：
- TargetData 是客户端产生的，服务器默认不知道
- 网络中 Activate() 和 ServerSetReplicatedTargetData() 的到达顺序是不确定的

解决方案（GAS 内置）：
- 客户端：
  - 本地广播 ValidData（立即使用）
  - 通过 ServerSetReplicatedTargetData RPC 把 TargetData 发给服务器
- 服务器：
  - 如果 Ability 还没 Activate → 先缓存 TargetData
  - Activate 时 Bind 委托 + CallIfSet，补消费已到达的 TargetData
- PredictionKey：
  - 用于把“一次 Ability 激活 + 相关 TargetData”绑定在一起
*/

#include "AbilitySystem/AbilityTasks/TargetDataUnderMouse.h"

#include "AbilitySystemComponent.h"

UTargetDataUnderMouse* UTargetDataUnderMouse::CreateTargetDataUnderMouse(UGameplayAbility* OwningAbility)
{
	UTargetDataUnderMouse* MyObj = NewAbilityTask<UTargetDataUnderMouse>(OwningAbility);
	return MyObj;
}

void UTargetDataUnderMouse::Activate()
{
	const bool bIsLocallyControlled = Ability->GetCurrentActorInfo()->IsLocallyControlled();
	if (bIsLocallyControlled)
	{
		/*
		 * 本地控制 (两种情况)
		 * On Server and being Locally Controlled (如 Listen Server): broadcasting the delegate
		 * On Client and being Locally Controlled: a. 立刻生成TargetData (鼠标射线) b. 本地使用用（预测） c. RPC 发给服务器
		 */
		SendMouseCursorData();
	}
	else
	{
		/* 
		 * 非本地控制
		 * 此时服务器不知道TargetData, 只能“等客户端发过来”
		 * 但要注意：此时TargetData RPC可能已经先到了，也可能还没到
		 * 因此要做三件事：
		 */
		const FGameplayAbilitySpecHandle SpecHandle = GetAbilitySpecHandle();
		FPredictionKey ActivationPredictionKey = GetActivationPredictionKey();
		// 1. 绑定当 TargetData 到达时要调用的回调
		AbilitySystemComponent.Get()->AbilityTargetDataSetDelegate(SpecHandle, ActivationPredictionKey).AddUObject(this, &UTargetDataUnderMouse::OnTargetDataReplicatedCallback);
		// 2. 立即检查：TargetData 是否已经提前通过 RPC 到达服务器; 如果 TargetData 已经提前到了，现在立刻补一次 Broadcast
		const bool bCalledDelegate = AbilitySystemComponent.Get()->CallReplicatedTargetDataDelegatesIfSet(SpecHandle, ActivationPredictionKey);
		// 3. 如果TargetData 还没到：
		if (!bCalledDelegate)
		{
			// AbilityTask 进入“等待远端玩家数据”的状态, GAS 内部会阻止 Ability 继续往下执行, 直到 OnTargetDataReplicatedCallback 被触发
			SetWaitingOnRemotePlayerData();
		}
	}
}

void UTargetDataUnderMouse::SendMouseCursorData()
{
	/*
	 * 打开一个预测窗口（Prediction Window）
	 * 作用： 为这一次 Ability 行为分配一个 PredictionKey, 让接下来发给服务器的 RPC（TargetData）和这次 Ability Activate 绑定在一起
	 */
	FScopedPredictionWindow ScopedPrediction(AbilitySystemComponent.Get());
	
	APlayerController* PC = Ability->GetCurrentActorInfo()->PlayerController.Get();
	FHitResult CursorHit;
	PC->GetHitResultUnderCursor(ECC_Visibility, false, CursorHit);

	/*
	 * 构造 TargetData
	 * 注意：FGameplayAbilityTargetData 是多态基类, SingleTargetHit 是其中一种具体实现
	 * 这里使用 new 是 GAS 的设计要求, Data 的内存生命周期由 DataHandle 管理
	 */
	FGameplayAbilityTargetDataHandle DataHandle;
	FGameplayAbilityTargetData_SingleTargetHit* Data = new FGameplayAbilityTargetData_SingleTargetHit();
	Data->HitResult = CursorHit;
	DataHandle.Add(Data);
	
	/*
	 * 关键 RPC：把 TargetData 发送给服务器
	 * 服务器会：根据 SpecHandle + PredictionKey 缓存 TargetData (添加到 AbilityTargetDataMap, 如果服务器补救一次 Broadcast 时正是从这里查询 TargetData 是否已经提前到达)
	 */
	AbilitySystemComponent->ServerSetReplicatedTargetData(
		GetAbilitySpecHandle(), 
		GetActivationPredictionKey(), 
		DataHandle, 
		FGameplayTag(), 
		AbilitySystemComponent->ScopedPredictionKey);
	
	/*
	 * 客户端本地立刻广播 ValidData：
	 * 目的：本地 Ability / Task 立即响应（预测）, 比如：立刻播放动画、特效、UI 反馈. 不用等服务器确认, 保证操作手感
	 */
	if (ShouldBroadcastAbilityTaskDelegates())
	{
		ValidData.Broadcast(DataHandle);
	}
}

// 服务器拿到 TargetData 时绑定的回调
void UTargetDataUnderMouse::OnTargetDataReplicatedCallback(const FGameplayAbilityTargetDataHandle& DataHandle,
	FGameplayTag ActivationTag)
{
	/*
	 * Tell ASC that target data has been received, so don't need to keep it stored and cached
	 * 告诉 ASC：这次 Ability 的 TargetData 已经被成功消费, 可以从 AbilityTargetDataMap 中清掉缓存. 防止重复使用 / 内存浪费
	 */
	AbilitySystemComponent->ConsumeClientReplicatedTargetData(GetAbilitySpecHandle(), GetActivationPredictionKey());
	
	/*
	 * 服务器侧广播 ValidData：
	 * 此时：服务器终于拿到了客户端发来的 TargetData, 可以继续执行权威逻辑（ApplyGE、计算伤害等）
	 */
	if (ShouldBroadcastAbilityTaskDelegates())
	{
		ValidData.Broadcast(DataHandle);
	}
}
