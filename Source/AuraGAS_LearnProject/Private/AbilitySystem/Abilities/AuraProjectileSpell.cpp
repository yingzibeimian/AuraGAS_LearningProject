// Copyright David He


#include "AbilitySystem/Abilities/AuraProjectileSpell.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "Actor/AuraProjectile.h"
#include "Interaction/CombatInterface.h"
#include "AuraGameplayTags.h"

void UAuraProjectileSpell::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
                                           const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
                                           const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);
	
}

void UAuraProjectileSpell::SpawnProjectile(const FVector& ProjectileTargetLocation, const FGameplayTag& SocketTag, bool bOverridePitch, float PitchOverride)
{
	const bool bIsServer = GetAvatarActorFromActorInfo()->HasAuthority();;
	if (!bIsServer) return;
	
	const FVector SocketLocation = ICombatInterface::Execute_GetCombatSocketLocation(GetAvatarActorFromActorInfo(), SocketTag);
    FRotator Rotation = (ProjectileTargetLocation - SocketLocation).Rotation();
    //Rotation.Pitch = 0.f; // 让Projectile与地面保持平行状态飞行 (Play as Client时客户端的火球位置会过高导致攻击不到Goblin, 因此注释掉)
	if (bOverridePitch)
	{
		Rotation.Pitch = PitchOverride;
	}
	
    FTransform SpawnTransform;
    SpawnTransform.SetLocation(SocketLocation);
    SpawnTransform.SetRotation(Rotation.Quaternion());
    
    AAuraProjectile* Projectile = GetWorld()->SpawnActorDeferred<AAuraProjectile>(
    	ProjectileClass,
    	SpawnTransform,
    	GetOwningActorFromActorInfo(),
    	Cast<APawn>(GetAvatarActorFromActorInfo()),
    	ESpawnActorCollisionHandlingMethod::AlwaysSpawn);
    
	/*
    const UAbilitySystemComponent* SourceASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(GetAvatarActorFromActorInfo());
    // 设置 EffectContextHandle
    FGameplayEffectContextHandle EffectContextHandle = SourceASC->MakeEffectContext(); // 自动设置Instigator为AuraPlayerState, EffectCauser为AuraCharacter
    EffectContextHandle.SetAbility(this); // 设置 AbilityInstanceNotReplicated, AbilityCDO, AbilityLevel
    EffectContextHandle.AddSourceObject(Projectile);
    TArray<TWeakObjectPtr<AActor>> Actors;
    Actors.Add(Projectile);
    EffectContextHandle.AddActors(Actors);
    FHitResult HitResult;
    HitResult.Location = ProjectileTargetLocation;
    EffectContextHandle.AddHitResult(HitResult);
    
    const FGameplayEffectSpecHandle SpecHandle = SourceASC->MakeOutgoingSpec(DamageEffectClass, GetAbilityLevel(), EffectContextHandle);
    
    const FAuraGameplayTags GameplayTags = FAuraGameplayTags::Get();
    
	const float ScaledDamage = Damage.GetValueAtLevel(GetAbilityLevel());
	UAbilitySystemBlueprintLibrary::AssignTagSetByCallerMagnitude(SpecHandle, DamageType, ScaledDamage);
    
    Projectile->DamageEffectSpecHandle = SpecHandle;
    */
	
	// 重构: 把 Damage 计算相关参数的设置 放进 FDamageEffectParams
    Projectile->DamageEffectParams = MakeDamageEffectParamsFromClassDefaults();
	
    Projectile->FinishSpawning(SpawnTransform);
}
