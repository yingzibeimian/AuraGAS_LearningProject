// Copyright David He


#include "AbilitySystem/Abilities/AuraBeamSpell.h"
#include "GameFramework/Character.h"
#include "Kismet/KismetSystemLibrary.h"

void UAuraBeamSpell::StoreMouseDataInfo(const FHitResult& HitResult)
{
	if (HitResult.bBlockingHit)
	{
		MouseHitLocation = HitResult.Location;
		MouseHitActor = HitResult.GetActor();
	}
	else
	{
		CancelAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true);
	}
}

void UAuraBeamSpell::StoreOwnerVariables()
{
	if (CurrentActorInfo)
	{
		OwnerPlayerController = CurrentActorInfo->PlayerController.Get();
		OwnerCharacter = Cast<ACharacter>(CurrentActorInfo->AvatarActor);
	}
}

void UAuraBeamSpell::TraceFirstTarget(const FVector& BeamTargetLocation, const FGameplayTag& SocketTag)
{
	check(OwnerCharacter);
	if (OwnerCharacter->Implements<UCombatInterface>())
	{
		const FVector SocketLocation = ICombatInterface::Execute_GetCombatSocketLocation(OwnerCharacter, SocketTag);
		
		TArray<AActor*> ActorsToIgnore;
		ActorsToIgnore.Add(OwnerCharacter);
		
		FHitResult HitResult;
		UKismetSystemLibrary::SphereTraceSingle(
			OwnerCharacter, 
			SocketLocation, 
			BeamTargetLocation,
			10.f,
			TraceTypeQuery1,
			false,
			ActorsToIgnore,
			EDrawDebugTrace::None,
			HitResult,
			true);
		
		if (HitResult.bBlockingHit)
		{
			MouseHitLocation = HitResult.ImpactPoint;
			MouseHitActor = HitResult.GetActor();
		}
	}
}
