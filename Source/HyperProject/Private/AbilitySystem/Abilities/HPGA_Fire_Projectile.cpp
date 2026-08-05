// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/Abilities/HPGA_Fire_Projectile.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "HPGameplayTags.h"
#include "Abilities/Tasks/AbilityTask_WaitDelay.h"
#include "Abilities/Tasks/AbilityTask_WaitInputRelease.h"
#include "AbilitySystem/HPAttributeSet.h"
#include "Characters/HPCharacterBase.h"
#include "Components/HPCombatComponent.h"
#include "Interfaces/CombatInterface.h"
#include "Weapons/HPProjectileBase.h"

UHPGA_Fire_Projectile::UHPGA_Fire_Projectile()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::LocalPredicted;
}

void UHPGA_Fire_Projectile::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
                                            const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
                                            const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	UE_LOG(LogTemp,Warning,TEXT("Throwing Activated"));
	UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo();

	if (!ASC)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	
	if (IsForRemoteClient()) //서버에 있는 OwningClient 
	{
		//데이터가 먼저 도착했을 경우를 대비해 한 번 call
		TargetDataDelegateHandle = ASC->AbilityTargetDataSetDelegate(
			Handle,
			ActivationInfo.GetActivationPredictionKey()).
		AddUObject(this, &UHPGA_Fire_Projectile::OnServerReceiveTargetData);

		ASC->CallReplicatedTargetDataDelegatesIfSet(
			Handle,
			ActivationInfo.GetActivationPredictionKey()
			);
	}
	if (bNeedCooldown)
	{
		if (!CommitAbilityCooldown(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo,false))
		{
			EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
			return;
		}
	}
	UAbilityTask_WaitInputRelease* ReleaseTask = UAbilityTask_WaitInputRelease::WaitInputRelease(this,true);
	ReleaseTask->OnRelease.AddDynamic(this, &UHPGA_Fire_Projectile::OnInputReleased);

	ReleaseTask->ReadyForActivation();

	if (IsLocallyControlled()) //로컬에서 우선적인 판단
	{
		FireOneShot();
	}
}

void UHPGA_Fire_Projectile::EndAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
	bool bReplicateEndAbility, bool bWasCancelled)
{

	UAbilitySystemComponent* ASC = ActorInfo? ActorInfo->AbilitySystemComponent.Get():nullptr;

	if (ASC && TargetDataDelegateHandle.IsValid())
	{
		ASC->AbilityTargetDataSetDelegate(Handle, ActivationInfo.GetActivationPredictionKey()).Remove(TargetDataDelegateHandle);
		TargetDataDelegateHandle.Reset();
	}
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

void UHPGA_Fire_Projectile::OnInputReleased(float TimeHeld)
{
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}

bool UHPGA_Fire_Projectile::MakeTargetData(FGameplayAbilityTargetDataHandle& OutTargetDataHandle)
{

	if (!GetAvatarActorFromActorInfo()->Implements<UCombatInterface>())
		return false;

	UE_LOG(LogTemp,Warning,TEXT("MakeTargetData"));
	FVector HitImpactPoint = ICombatInterface::Execute_GetHitImpactPoint(GetAvatarActorFromActorInfo());
	
	FGameplayEffectContextHandle ContextHandle =  GetAbilitySystemComponentFromActorInfo()->MakeEffectContext();
	

	//GetAbilitySystemComponentFromActorInfo()->ExecuteGameplayCue(BeamGameplayCueTag, CueParams);
	
	FGameplayAbilityTargetData_LocationInfo* LocationData = new FGameplayAbilityTargetData_LocationInfo();

	LocationData->TargetLocation.LocationType =
		EGameplayAbilityTargetingLocationType::LiteralTransform;

	LocationData->TargetLocation.LiteralTransform = FTransform(FQuat::Identity, HitImpactPoint);

	OutTargetDataHandle.Add(LocationData);
	return true;
}

void UHPGA_Fire_Projectile::SendTargetDataToServer(const FGameplayAbilityTargetDataHandle& TargetDataHandle)
{
	
	UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo();
	if (!ASC)
	{
		return;
	}
	UE_LOG(LogTemp,Warning,TEXT("SendTargetDataToServer"));
	ASC->CallServerSetReplicatedTargetData
	(
		CurrentSpecHandle,
		CurrentActivationInfo.GetActivationPredictionKey(),
		TargetDataHandle,
		FGameplayTag(),
		ASC->ScopedPredictionKey
	);
}

void UHPGA_Fire_Projectile::OnFireDelayFinished()
{
	FireOneShot();
}

void UHPGA_Fire_Projectile::StartFireDelay()
{
	UAbilityTask_WaitDelay* DelayTask = UAbilityTask_WaitDelay::WaitDelay(this, FireInterval);

	DelayTask->OnFinish.AddDynamic(this, &UHPGA_Fire_Projectile::OnFireDelayFinished);

	DelayTask->ReadyForActivation();
}

void UHPGA_Fire_Projectile::FireOneShot()
{
	if (!IsLocallyControlled()) //로컬에서 쏘므로
	{
		return;
	}
	UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo();

	if (!ASC)
		return;
	UE_LOG(LogTemp,Warning,TEXT("FireOneShot"));
	
	FGameplayAbilityTargetDataHandle TargetDataHandle;

	if (!MakeTargetData(TargetDataHandle))
	{
		EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
		return;
	}
	UE_LOG(LogTemp,Warning,TEXT("AfterMakeTargetData"));
	bool bShouldReload = false; //윈도우 내부의 값을 저장하기 위한 변수

	{
		FScopedPredictionWindow PredictionWindow(ASC,true);//현 Prediction Key에 묶기


		if (!CommitAbilityCost(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo))
		{
			EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
			return;
		}
		
		ASC->PlayMontage(this, CurrentActivationInfo, FireMontage, 1.0f);
		
		const float CurrentBullet = ASC->GetNumericAttribute(UHPAttributeSet::GetBulletAttribute());
		bShouldReload = CurrentBullet<=0.f;

		if (IsPredictingClient()) //현재 Prediction Window로 서버에 전달
		{
			UE_LOG(LogTemp,Warning,TEXT("Call SendTargetDataToServer"));
			SendTargetDataToServer(TargetDataHandle);
		}
	
	}

	if (bShouldReload)
	{
		//리로드 어빌리티
		const FHPGameplayTags& GameplayTags = FHPGameplayTags::Get();

		FGameplayEventData Payload;
		Payload.EventTag = GameplayTags.Event_Reload;
		
		UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(GetAvatarActorFromActorInfo(), GameplayTags.Event_Reload,Payload);
		
		EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
		return;
	}
	
	StartFireDelay();
}

void UHPGA_Fire_Projectile::OnServerReceiveTargetData(
	const FGameplayAbilityTargetDataHandle& GameplayAbilityTargetDataHandle, FGameplayTag GameplayTag)
{
	const FGameplayAbilityTargetData* Data = GameplayAbilityTargetDataHandle.Get(0);
	UE_LOG(LogTemp, Warning, TEXT("OnServerReceiveTargetData"));
	if (!Data)
	{
		return;
	}

	FVector TargetPoint = Data->GetEndPoint();
	FVector ProjectileSpawnPoint;
	if (GetAvatarActorFromActorInfo()->Implements<UCombatInterface>())
	{
		ProjectileSpawnPoint = SpawnSocketType==EProjectileSpawnSocketType::Weapon ? ICombatInterface::Execute_GetWeaponSocketLocation(GetAvatarActorFromActorInfo()): ICombatInterface::Execute_GetThrowingHandSocketLocation(GetAvatarActorFromActorInfo());
	}
	UE_LOG(LogTemp, Warning, TEXT("OnServerReceiveTargetData Rotation"));
	FRotator SpawnRotation = (TargetPoint - ProjectileSpawnPoint).Rotation();
	
	FTransform SpawnTransform;
	SpawnTransform.SetLocation(ProjectileSpawnPoint);
	SpawnTransform.SetRotation(SpawnRotation.Quaternion());

	AHPProjectileBase* Projectile = GetWorld()->SpawnActorDeferred<AHPProjectileBase>(
		ProjectileClass,
		SpawnTransform,
		GetOwningActorFromActorInfo(),
		Cast<APawn>(GetOwningActorFromActorInfo()),
		ESpawnActorCollisionHandlingMethod::AlwaysSpawn);

	if (Projectile->IsMine())
	{
		//NEXTTHINGTODO: 바인딩
	}
	Projectile->SetProjectileEffectParams(MakeProjectileParams());
	
	Projectile->FinishSpawning(SpawnTransform);
}

