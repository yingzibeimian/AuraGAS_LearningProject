// Copyright David He


#include "Player/AuraPlayerState.h"

#include "AbilitySystem/AuraAbilitySystemComponent.h"
#include "AbilitySystem/AuraAttributeSet.h"

AAuraPlayerState::AAuraPlayerState()
{
	AbilitySystemComponent = CreateDefaultSubobject<UAuraAbilitySystemComponent>("AbilitySystemComponent");
	AbilitySystemComponent->SetIsReplicated(true);
	// Full: Gameplay Effects are replicated to all clients. (Case: Single Player)
	// Mixed: Gameplay Effects are replicated to the owning client only. Gameplay Cues and Gameplay Tags replicated to all clients. (Case: Multiplayer, Player-Controlled)
	// Minimal: Gameplay Effects are not replicated. Gameplay Cues and Gameplay Tags replicated to all clients. (Case: Multiplayer, AI-Controlled)
	AbilitySystemComponent->SetReplicationMode(EGameplayEffectReplicationMode::Mixed);
	
	AttributeSet = CreateDefaultSubobject<UAuraAttributeSet>("AttributeSet");
	
	// NetUpdateFrequency: how often (per second) the server will try to update clients
	// as changes occur on the server for the player state, the server will be sending updates out to all clients 
	// so that clients can sync up with server version(同步)
	NetUpdateFrequency = 100.f;
}

UAbilitySystemComponent* AAuraPlayerState::GetAbilitySystemComponent() const
{
	return AbilitySystemComponent;
}
