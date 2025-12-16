// Copyright David He


#include "Character/AuraCharacter.h"

#include "AbilitySystemComponent.h"
#include "AbilitySystem/AuraAbilitySystemComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Player/AuraPlayerController.h"
#include "Player/AuraPlayerState.h"
#include "UI/HUD/AuraHUD.h"

AAuraCharacter::AAuraCharacter()
{
	// rotate the Character toward the direction of acceleration
	GetCharacterMovement()->bOrientRotationToMovement = true;
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 400.0f, 0.0f); 
	GetCharacterMovement()->bConstrainToPlane = true;
	GetCharacterMovement()->bSnapToPlaneAtStart = true;
	
	bUseControllerRotationPitch = false;
	bUseControllerRotationRoll = false;
	bUseControllerRotationYaw = false;
}

/// “控制权”的建立（Possession）是服务器权威行为, 仅在服务器调用
void AAuraCharacter::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);
	
	// Init ability actor for the Server
	// Call InitAbilityActorInfo here because it needs to be sure that we have a valid player controller set already, and our player state is valid and accessible
	// The PlayerState is created and assigned by the engine during the possession process, which happens later in the actor's lifecycle such as constructor
	InitAbilityActorInfo();
}

/// RepNotify回调函数
/// PlayerState指针通过RepNotify从服务器同步到客户端
void AAuraCharacter::OnRep_PlayerState()
{
	Super::OnRep_PlayerState();
	
	// Init ability actor for the Client
	InitAbilityActorInfo();
}

void AAuraCharacter::InitAbilityActorInfo()
{
	// 初始化PlayerState中ASC的OwnerActor和AvatarActor
	AAuraPlayerState* AuraPlayerState = GetPlayerState<AAuraPlayerState>();
	check(AuraPlayerState);
	AuraPlayerState->GetAbilitySystemComponent()->InitAbilityActorInfo(AuraPlayerState, this);
	// do something after InitAbilityActorInfo such as bind func to delegate
	Cast<UAuraAbilitySystemComponent>(AuraPlayerState->GetAbilitySystemComponent())->AbilityActorInfoSet();
	
	// 设置AuraCharacterBase中的ASC和AS指针
	AbilitySystemComponent = AuraPlayerState->GetAbilitySystemComponent();
	AttributeSet = AuraPlayerState->GetAttributeSet();
	
	// 这里必须用 if 而不能用 check：
	// 因为 InitAbilityActorInfo() 不仅会在“本地受控角色”上调用，
	// 还会在“其他客户端角色的副本”上通过 OnRep_PlayerState() 被调用。
	// 对于这些非本机角色：
	//   - 它们有 PlayerState（复制而来）
	//   - 但没有有效的 PlayerController（GetController() 为 nullptr）
	// 如果这里用 check，任意一个远端角色同步下来都会直接崩溃。
	// 所以这里只能“有就初始化 UI，没有就跳过”，这是联机下的必然写法。
	if (AAuraPlayerController* AuraPlayerController = Cast<AAuraPlayerController>(GetController()))
	{
		if (AAuraHUD* AuraHUD = Cast<AAuraHUD>(AuraPlayerController->GetHUD()))
		{
			// 初始化OverlayWidget和OverlayWidgetController, 并做绑定
			AuraHUD->InitOverlay(AuraPlayerController, AuraPlayerState, AbilitySystemComponent, AttributeSet);
		}
	}
	
	// 初始化AttributeSet中的属性
	InitializePrimaryAttributes();
}
