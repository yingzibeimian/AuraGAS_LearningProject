// Copyright David He


#include "Character/AuraCharacter.h"

#include "AbilitySystemComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Player/AuraPlayerState.h"

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
	// 设置AuraCharacterBase中的ASC和AS指针
	AbilitySystemComponent = AuraPlayerState->GetAbilitySystemComponent();
	AttributeSet = AuraPlayerState->GetAttributeSet();
}
