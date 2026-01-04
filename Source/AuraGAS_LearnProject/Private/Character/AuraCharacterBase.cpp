// Copyright David He


#include "Character/AuraCharacterBase.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystem/AuraAbilitySystemComponent.h"
#include "AbilitySystem/AuraAttributeSet.h"
#include "AuraGAS_LearnProject/AuraGAS_LearnProject.h"
#include "Components/CapsuleComponent.h"

AAuraCharacterBase::AAuraCharacterBase()
{
	PrimaryActorTick.bCanEverTick = false;

	GetCapsuleComponent()->SetCollisionResponseToChannel(ECC_Camera, ECR_Ignore);
	GetCapsuleComponent()->SetGenerateOverlapEvents(false);
	GetMesh()->SetCollisionResponseToChannel(ECC_Camera, ECR_Ignore);
	GetMesh()->SetCollisionResponseToChannel(ECC_Projectile, ECR_Overlap);
	GetMesh()->SetGenerateOverlapEvents(true);
	
	Weapon = CreateDefaultSubobject<USkeletalMeshComponent>("Weapon");
	Weapon->SetupAttachment(GetMesh(), FName("WeaponHandSocket"));
	Weapon->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

UAbilitySystemComponent* AAuraCharacterBase::GetAbilitySystemComponent() const
{
	return AbilitySystemComponent;
}

UAnimMontage* AAuraCharacterBase::GetHitReactMontage_Implementation()
{
	return HitReactMontage;
}

void AAuraCharacterBase::Die()
{
	// Detach本身就是Replication行为
	Weapon->DetachFromComponent(FDetachmentTransformRules(EDetachmentRule::KeepWorld, true));
	MulticastHandleDeath();
}

void AAuraCharacterBase::MulticastHandleDeath_Implementation()
{
	// 武器掉落
	Weapon->SetSimulatePhysics(true);
	Weapon->SetEnableGravity(true);
	Weapon->SetCollisionEnabled(ECollisionEnabled::PhysicsOnly);
	
	GetMesh()->SetSimulatePhysics(true);
	GetMesh()->SetEnableGravity(true);
	GetMesh()->SetCollisionEnabled(ECollisionEnabled::PhysicsOnly);
	GetMesh()->SetCollisionResponseToChannel(ECC_WorldStatic, ECR_Block);
	
	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	
	// 溶解特效
	Dissolve();
}

void AAuraCharacterBase::BeginPlay()
{
	Super::BeginPlay();
	
}

FVector AAuraCharacterBase::GetCombatSocketLocation_Implementation()
{
	check(Weapon);
	return Weapon->GetSocketLocation(WeaponTipSocketName);
}

void AAuraCharacterBase::InitAbilityActorInfo()
{
}

void AAuraCharacterBase::ApplyEffectToSelf(TSubclassOf<UGameplayEffect> GameplayEffectClass, float Level) const
{
	check(IsValid(GetAbilitySystemComponent()));
	check(GameplayEffectClass);
	FGameplayEffectContextHandle ContextHandle = GetAbilitySystemComponent()->MakeEffectContext();
	ContextHandle.AddSourceObject(this);
	const FGameplayEffectSpecHandle SpecHandle = GetAbilitySystemComponent()->MakeOutgoingSpec(GameplayEffectClass, Level, ContextHandle);
	GetAbilitySystemComponent()->ApplyGameplayEffectSpecToTarget(*SpecHandle.Data.Get(), GetAbilitySystemComponent());
}

void AAuraCharacterBase::InitializeDefaultAttributes() const
{
	ApplyEffectToSelf(DefaultPrimaryAttributes, 1.f);
	ApplyEffectToSelf(DefaultSecondaryAttributes, 1.f);
	ApplyEffectToSelf(DefaultVitalAttributes, 1.f);
	
	/*
	 * 监听主属性变化，用于在主属性发生改变时重新 Apply 一次派生属性 GE，确保派生属性始终与主属性保持一致
	 * (原本使用 Infinite GE 持续更新派生属性，但在PIE的服务器端/Listen Server 上不生效,
	 *	因此改为当前改为在初始化和属性变化时应用 Instant GE，确保服务器端也能正确同步派生属性。
	 *	如果未来发现问题，可考虑回退到 Infinite GE)
	 */
	BindPrimaryAttributeDelegates();
}

void AAuraCharacterBase::BindPrimaryAttributeDelegates() const
{
	if (!HasAuthority()) return;
	
	const UAuraAttributeSet* AS = Cast<UAuraAttributeSet>(AttributeSet);
	if (!AS) return;
	
	AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(AS->GetStrengthAttribute()).AddLambda(
		[this](const FOnAttributeChangeData& Data)
		{
			ApplyEffectToSelf(DefaultSecondaryAttributes, 1.f);
		});
	
	AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(AS->GetIntelligenceAttribute()).AddLambda(
		[this](const FOnAttributeChangeData& Data)
		{
			ApplyEffectToSelf(DefaultSecondaryAttributes, 1.f);
		});
	
	AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(AS->GetResilienceAttribute()).AddLambda(
		[this](const FOnAttributeChangeData& Data)
		{
			ApplyEffectToSelf(DefaultSecondaryAttributes, 1.f);
		});
	
	AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(AS->GetVigorAttribute()).AddLambda(
		[this](const FOnAttributeChangeData& Data)
		{
			ApplyEffectToSelf(DefaultSecondaryAttributes, 1.f);
		});
}

void AAuraCharacterBase::AddCharacterAbilities()
{
	//只在Server上Add Abilities
	if (!HasAuthority()) return;
	
	UAuraAbilitySystemComponent* AuraASC = CastChecked<UAuraAbilitySystemComponent>(AbilitySystemComponent);
	
	AuraASC->AddCharacterAbilities(StartupAbilities);
}

void AAuraCharacterBase::Dissolve()
{
	if (IsValid(DissolveMaterialInstance))
	{
		UMaterialInstanceDynamic* DynamicMatInst = UMaterialInstanceDynamic::Create(DissolveMaterialInstance, this);
		GetMesh()->SetMaterial(0, DynamicMatInst);
		StartDissolveTimeline(DynamicMatInst);
	}
	if (IsValid(WeaponDissolveMaterialInstance))
	{
		UMaterialInstanceDynamic* DynamicMatInst = UMaterialInstanceDynamic::Create(WeaponDissolveMaterialInstance, this);
		Weapon->SetMaterial(0, DynamicMatInst);
		StartWeaponDissolveTimeline(DynamicMatInst);
	}
}

