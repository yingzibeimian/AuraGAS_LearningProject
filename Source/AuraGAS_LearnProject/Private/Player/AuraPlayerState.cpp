// Copyright David He


#include "Player/AuraPlayerState.h"

AAuraPlayerState::AAuraPlayerState()
{
	// NetUpdateFrequency: how often (per second) the server will try to update clients
	// as changes occur on the server for the player state, the server will be sending updates out to all clients 
	// so that clients can sync up with server version(同步)
	NetUpdateFrequency = 100.f;
}
