// Copyright David He


#include "Actor/AuraProjectile.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "AbilitySystem/AuraAbilitySystemLibrary.h"
#include "AuraGAS_LearnProject/AuraGAS_LearnProject.h"
#include "Components/AudioComponent.h"
#include "Components/SphereComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Kismet/GameplayStatics.h"

AAuraProjectile::AAuraProjectile()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;

	Sphere = CreateDefaultSubobject<USphereComponent>("Sphere");
	SetRootComponent(Sphere);
	Sphere->SetCollisionObjectType(ECC_Projectile);
	Sphere->SetCollisionEnabled(ECollisionEnabled::NoCollision); // 初始化阶段，禁止碰撞; FinishSpawning之后再开启碰撞
	Sphere->SetCollisionResponseToChannels(ECR_Ignore);
	Sphere->SetCollisionResponseToChannel(ECC_WorldDynamic, ECR_Overlap);
	Sphere->SetCollisionResponseToChannel(ECC_WorldStatic, ECR_Overlap);
	Sphere->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	
	ProjectileMovement = CreateDefaultSubobject<UProjectileMovementComponent>("ProjectileMovement");
	ProjectileMovement->InitialSpeed = 550.f;
	ProjectileMovement->MaxSpeed = 550.f;
	ProjectileMovement->ProjectileGravityScale = 0.f;
}

void AAuraProjectile::EnableCollision() const
{
	Sphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
}

void AAuraProjectile::BeginPlay()
{
	Super::BeginPlay();
	SetLifeSpan(LifeSpan);
	Sphere->OnComponentBeginOverlap.AddDynamic(this, &AAuraProjectile::OnSphereOverlap);
	
	LoopingSoundComponent = UGameplayStatics::SpawnSoundAttached(LoopingSound, GetRootComponent());
}

void AAuraProjectile::OnHit()
{
	UGameplayStatics::PlaySoundAtLocation(this, ImpactSound, GetActorLocation(), FRotator::ZeroRotator);
	UNiagaraFunctionLibrary::SpawnSystemAtLocation(this, ImpactEffect, GetActorLocation(), FRotator::ZeroRotator);
	if (LoopingSoundComponent) LoopingSoundComponent->Stop();
	bHit = true;
}

void AAuraProjectile::Destroyed()
{
	// 防止碰撞后, 服务器已经Destroy, 且该权威行为现在Replicate到客户端时, 但客户端尚未发生碰撞： 此时直接播放碰撞音效和特效并Destroy
	if (!bHit && !HasAuthority()) OnHit();
	
	Super::Destroyed();
}

void AAuraProjectile::OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
                                      UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	/*
	 * Design Notes:
	 * Projectile 使用 Deferred Spawn，在 FinishSpawning 之前其 GameplayEffect 数据尚未准备完成。
	 * 在网络环境（尤其是 Client）下，OnSphereOverlap 可能早于 DamageEffectSpecHandle 的初始化触发。
	 * 为避免无效 Overlap 参与逻辑：
	 * 1. 碰撞在 SpecHandle 完成初始化后才启用（EnableCollision），从源头减少无意义 Overlap。
	 * 2. OnSphereOverlap 内仍对 DamageEffectSpecHandle 做 IsValid 判定，作为网络时序与异常情况的兜底防御。
	 * 伤害应用与 Destroy 行为仅由 Server 执行，Client 只负责表现（特效/音效）。
	 * bHit 用于解决 OnSphereOverlap 与 Destroy 在 Client 上可能产生的竞态，防止特效与音效重复播放。
	 */
	
	// Overlap只在“数据完整”的情况下才响应, 并防止火球碰撞到自己
	// if (!DamageEffectSpecHandle.Data.IsValid() || DamageEffectSpecHandle.Data.Get()->GetContext().GetEffectCauser() == OtherActor)
	// {
	// 	return;
	// }
	AActor* SourceAvatarActor = DamageEffectParams.SourceAbilitySystemComponent->GetAvatarActor();
	// 防止Projectile碰撞到发射者自身
	if (SourceAvatarActor == OtherActor) return;
	// 防止伤害友方
	if (UAuraAbilitySystemLibrary::IsFriend(SourceAvatarActor, OtherActor)) return;
	
	// 防止客户端上OnSphereOverlap和Destroyed产生竞态时的两种情况
	// 1. 先执行Destroyed, 则在Destroyed中播放碰撞音效和特效, 后续物体已经被销毁, OnSphereOverlap不会执行
	// 2. 先执行OnSphereOverlap, 并标记为已碰撞, 后续Destroyed执行时就不会重复播放
	if (!bHit) OnHit();
	
	if (HasAuthority())
	{
		// Destroy 和 应用GameplayEffect都是服务器权威行为
		if (UAbilitySystemComponent* TargetASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(OtherActor))
		{
			const FVector DeathImpulse = GetActorForwardVector() * DamageEffectParams.DeathImpulseMagnitude;
			DamageEffectParams.DeathImpulse = DeathImpulse;
			DamageEffectParams.TargetAbilitySystemComponent = TargetASC;
			UAuraAbilitySystemLibrary::ApplyDamageEffect(DamageEffectParams);
		}
		
		Destroy();
	}
	else bHit = true;
}

