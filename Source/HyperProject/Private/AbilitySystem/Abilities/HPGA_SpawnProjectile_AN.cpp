// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/Abilities/HPGA_SpawnProjectile_AN.h"

#include "AbilitySystemComponent.h"
#include "HPGameplayTags.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"
#include "Interfaces/CombatInterface.h"
#include "Weapons/HPProjectileBase.h"

UHPGA_SpawnProjectile_AN::UHPGA_SpawnProjectile_AN()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::LocalPredicted;
}

void UHPGA_SpawnProjectile_AN::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
                                               const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
                                               const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo();

	if (!ASC)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	if (!HasAuthorityOrPredictionKey(ActorInfo, &ActivationInfo))
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}
	
	if (IsForRemoteClient()) //서버에 있는 OwningClient 
	{
		
		TargetDataDelegateHandle = ASC->AbilityTargetDataSetDelegate(
			Handle,
			ActivationInfo.GetActivationPredictionKey()).
		AddUObject(this, &UHPGA_SpawnProjectile_AN::OnServerReceiveTargetData);

		//데이터가 먼저 도착했을 경우를 대비해 한 번 call
		ASC->CallReplicatedTargetDataDelegatesIfSet(
			Handle,
			ActivationInfo.GetActivationPredictionKey()
			);
	}
	
	if (!CommitAbilityCooldown(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo,false))
	{
		EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
		return;
	}

	if (IsLocallyControlled()) //로컬에서 우선적인 판단
	{
		FireOneShot();
	}
}

bool UHPGA_SpawnProjectile_AN::MakeTargetData(FGameplayAbilityTargetDataHandle& OutTargetDataHandle) const
{
	if (!GetAvatarActorFromActorInfo()->Implements<UCombatInterface>())
		return false;

	UE_LOG(LogTemp,Warning,TEXT("MakeTargetData"));
	FVector HitImpactPoint = ICombatInterface::Execute_GetHitImpactPoint(GetAvatarActorFromActorInfo());
	
	FGameplayEffectContextHandle ContextHandle =  GetAbilitySystemComponentFromActorInfo()->MakeEffectContext();
	
	FGameplayAbilityTargetData_LocationInfo* LocationData = new FGameplayAbilityTargetData_LocationInfo();

	LocationData->TargetLocation.LocationType = EGameplayAbilityTargetingLocationType::LiteralTransform;

	LocationData->TargetLocation.LiteralTransform = FTransform(FQuat::Identity, HitImpactPoint);

	OutTargetDataHandle.Add(LocationData);
	return true;
}

void UHPGA_SpawnProjectile_AN::FireOneShot()
{
	if (!IsLocallyControlled()) //로컬에서 쏘므로
	{
		return;
	}
	UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo();

	if (!ASC)
		return;
	
	UE_LOG(LogTemp,Warning,TEXT("UHPGA_SpawnProjectile_AN FireOneShot"));
	
	FGameplayAbilityTargetDataHandle TargetDataHandle;

	if (!MakeTargetData(TargetDataHandle))
	{
		EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
		return;
	}
	UE_LOG(LogTemp,Warning,TEXT("AfterMakeTargetData"));

	if (IsPredictingClient()) //현재 Prediction Window로 서버에 전달
	{
		UE_LOG(LogTemp,Warning,TEXT("Call SendTargetDataToServer"));
		SendTargetDataToServer(TargetDataHandle);
	}
}


void UHPGA_SpawnProjectile_AN::OnServerReceiveTargetData(const FGameplayAbilityTargetDataHandle& TargetDataHandle,
                                                         FGameplayTag ApplicationTag)
{

	UAbilityTask_WaitGameplayEvent* WaitANEventTask = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(
		this,
		FHPGameplayTags::Get().Event_SpawnProjectile);
	WaitANEventTask->EventReceived.AddDynamic(this, &UHPGA_SpawnProjectile_AN::SpawnProjectile);
	WaitANEventTask->ReadyForActivation();
	
	PlayFireMontageTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(this, NAME_None, FireMontage);
	//PlayFireMontageTask->OnBlendOut.AddDynamic(this, &UHPGA_SpawnProjectile_AN::K2_EndAbility);
	PlayFireMontageTask->OnCancelled.AddDynamic(this, &UHPGA_SpawnProjectile_AN::K2_EndAbility);
	PlayFireMontageTask->OnInterrupted.AddDynamic(this, &UHPGA_SpawnProjectile_AN::K2_EndAbility);
	PlayFireMontageTask->OnCompleted.AddDynamic(this, &UHPGA_SpawnProjectile_AN::K2_EndAbility);
	PlayFireMontageTask->ReadyForActivation();

	UE_LOG(LogTemp, Warning,TEXT("Montage"));

	TargetData = TargetDataHandle.Get(0);
}

void UHPGA_SpawnProjectile_AN::SendTargetDataToServer(const FGameplayAbilityTargetDataHandle& TargetDataHandle) const
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

void UHPGA_SpawnProjectile_AN::SpawnProjectile(FGameplayEventData Payload)
{
	if (!TargetData)
	{
		return;
	}

	FVector TargetPoint = TargetData->GetEndPoint();
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


	Projectile->SetProjectileParams(MakeProjectileParams());
	Projectile->FinishSpawning(SpawnTransform);
}

