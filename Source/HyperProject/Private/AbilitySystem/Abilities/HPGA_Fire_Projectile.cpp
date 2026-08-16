// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/Abilities/HPGA_Fire_Projectile.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "HPGameplayTags.h"
#include "Abilities/Tasks/AbilityTask_WaitDelay.h"
#include "Abilities/Tasks/AbilityTask_WaitInputRelease.h"
#include "AbilitySystem/HPAttributeSet.h"
#include "Characters/Player/HPPlayerCharacter.h"
#include "Components/HPCombatComponent.h"
#include "Interfaces/CombatInterface.h"
#include "Weapons/HPProjectileBase.h"
#include "Weapons/HPVisualProjectile.h"

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
	FVector ProjectileSpawnPoint = FVector::ZeroVector;
	FVector EndLocation = FVector::ZeroVector;
	
	if (AActor* PlayerActor = GetAvatarActorFromActorInfo())
	{
		if (PlayerActor->Implements<UCombatInterface>())
		{
			ProjectileSpawnPoint = SpawnSocketType==EProjectileSpawnSocketType::Weapon ? ICombatInterface::Execute_GetWeaponSocketLocation(GetAvatarActorFromActorInfo()): ICombatInterface::Execute_GetThrowingHandSocketLocation(GetAvatarActorFromActorInfo());
			EndLocation = ICombatInterface::Execute_GetHitImpactPoint(PlayerActor);
		}
		else
		{
			return false;
		}
	}
	
	FGameplayAbilityTargetData_LocationInfo* LocationData = new FGameplayAbilityTargetData_LocationInfo();

	LocationData->SourceLocation.LocationType = EGameplayAbilityTargetingLocationType::LiteralTransform;
	LocationData->SourceLocation.LiteralTransform = FTransform(FRotator::ZeroRotator, ProjectileSpawnPoint);

	LocationData->TargetLocation.LocationType = EGameplayAbilityTargetingLocationType::LiteralTransform;
	LocationData->TargetLocation.LiteralTransform = FTransform(FRotator::ZeroRotator, EndLocation);


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
	
	FVector ProjectileSpawnPoint = FVector::ZeroVector;
	FVector EndLocation = FVector::ZeroVector;
	bool bUseServerSideRewind = false;
	
	if (AActor* PlayerActor = GetAvatarActorFromActorInfo())
	{
		if (PlayerActor->Implements<UCombatInterface>())
		{
			ProjectileSpawnPoint = SpawnSocketType==EProjectileSpawnSocketType::Weapon ? ICombatInterface::Execute_GetWeaponSocketLocation(GetAvatarActorFromActorInfo()): ICombatInterface::Execute_GetThrowingHandSocketLocation(GetAvatarActorFromActorInfo());
			EndLocation = ICombatInterface::Execute_GetHitImpactPoint(PlayerActor);
			bUseServerSideRewind=ICombatInterface::Execute_IsUsingServerRewind(PlayerActor);
		}
	}

	if (bUseServerSideRewind)
	{
		APawn* InstigatorPawn = Cast<APawn>(GetAvatarActorFromActorInfo());
		UWorld* World = GetWorld();

		if (World && InstigatorPawn && !ProjectileSpawnPoint.IsZero())
		{
			
			FVector ToTarget = (EndLocation - ProjectileSpawnPoint);
			FRotator TargetRotation = ToTarget.Rotation();

			FActorSpawnParameters SpawnParams;
			SpawnParams.Owner = GetAvatarActorFromActorInfo();
			SpawnParams.Instigator = InstigatorPawn;

			/*AHPProjectileBase* SpawnedProjectile = nullptr;

			if (IGenericTeamAgentInterface* TeamAgentInterface= Cast<IGenericTeamAgentInterface>(InstigatorPawn))
			{
				SpawnedProjectile = World->SpawnActor<AHPProjectileBase>(SpawnParams);
				SpawnedProjectile->ShouldUseServerSideRewind(true);
				SpawnedProjectile->SetTraceStart(ProjectileSpawnPoint);
				SpawnedProjectile->SetInitialVelocity(SpawnedProjectile->GetInitialSpeed()*SpawnedProjectile->GetActorForwardVector());
				SpawnedProjectile->SetGenericTeamId(TeamAgentInterface->GetGenericTeamId());
			}*/
			
			//NEXTTHINGTODO: TargetData보내서 시각화 전용, replicates=true인 시각화전용 투사체 만드는 코드

			FTransform SpawnTransform;
			SpawnTransform.SetLocation(ProjectileSpawnPoint);
			SpawnTransform.SetRotation(TargetRotation.Quaternion());

			AHPProjectileBase* ServerSideRewindProjectile = GetWorld()->SpawnActorDeferred<AHPProjectileBase>(
			ServerSideRewindProjectileClass,
			SpawnTransform,
		GetOwningActorFromActorInfo(),
			InstigatorPawn,
			ESpawnActorCollisionHandlingMethod::AlwaysSpawn);
			
			ServerSideRewindProjectile->SetProjectileParams(MakeProjectileParams());
			if (IGenericTeamAgentInterface* TeamAgentInterface= Cast<IGenericTeamAgentInterface>(InstigatorPawn))
			{
				ServerSideRewindProjectile->SetGenericTeamId(TeamAgentInterface->GetGenericTeamId());
				
			}
			ServerSideRewindProjectile->SetTraceStart(ProjectileSpawnPoint);
			ServerSideRewindProjectile->SetInitialVelocity(ServerSideRewindProjectile->GetInitialSpeed()*ServerSideRewindProjectile->GetActorForwardVector());
				
			ServerSideRewindProjectile->FinishSpawning(SpawnTransform);
		}
		FGameplayAbilityTargetDataHandle TargetDataHandle;

		if (!MakeTargetData(TargetDataHandle))
		{
			EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
			return;
		}
		
		{
			FScopedPredictionWindow PredictionWindow(ASC,true);//현 Prediction Key에 묶기

			if (!CommitAbilityCost(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo))
			{
				EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
				return;
			}


			ASC->PlayMontage(this, CurrentActivationInfo, FireMontage, 1.0f);
		
			if (IsPredictingClient()) //현재 Prediction Window로 서버에 전달
			{
				SendTargetDataToServer(TargetDataHandle);
			}
	
		}
	}

	else
	{
		FGameplayAbilityTargetDataHandle TargetDataHandle;

		if (!MakeTargetData(TargetDataHandle))
		{
			EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
			return;
		}
		
		{
			FScopedPredictionWindow PredictionWindow(ASC,true);//현 Prediction Key에 묶기


			if (!CommitAbilityCost(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo))
			{
				EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
				return;
			}
		
			ASC->PlayMontage(this, CurrentActivationInfo, FireMontage, 1.0f);
		
			

			if (IsPredictingClient()) //현재 Prediction Window로 서버에 전달
			{
				SendTargetDataToServer(TargetDataHandle);
			}
	
		}
	}
	const float CurrentBullet = ASC->GetNumericAttribute(UHPAttributeSet::GetBulletAttribute());
	bool bShouldReload = CurrentBullet<=0.f;
		
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
	if (!CommitAbilityCost(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo))
	{
		EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
		return;
	}
	const FGameplayAbilityTargetData* Data = GameplayAbilityTargetDataHandle.Get(0);

	if (!Data)
	{
		EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
		return;
	}
	if (Data && Data->GetScriptStruct() == FGameplayAbilityTargetData_LocationInfo::StaticStruct())
	{
		if (!CommitAbilityCost(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo))
		{
			EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
			return;
		}
		//
		// if (bHasBoundedSpawnedActor)
		// {
		// 	GetHPPlayerCharacterFromActorInfo()->GetCombatComponent()->TryDestroyBoundedSpawnedActor();
		// }
		//
		FGameplayTagContainer Tags;
		Tags.AddTag(FHPGameplayTags::Get().State_Ult_Full);

		GetAbilitySystemComponentFromActorInfo()->RemoveActiveEffectsWithGrantedTags(Tags);
		
		const FGameplayAbilityTargetData_LocationInfo* LocationData =
			static_cast<const FGameplayAbilityTargetData_LocationInfo*>(Data);

		FVector SourceLocation = LocationData->SourceLocation.GetTargetingTransform().GetLocation();
		FVector TargetLocation = LocationData->TargetLocation.GetTargetingTransform().GetLocation();

		FRotator SpawnRotation = (TargetLocation - SourceLocation).Rotation();

		FTransform SpawnTransform;
		SpawnTransform.SetLocation(SourceLocation);
		SpawnTransform.SetRotation(SpawnRotation.Quaternion());

		FActorSpawnParameters SpawnParams;
		SpawnParams.Owner = GetAvatarActorFromActorInfo();
		SpawnParams.Instigator = Cast<APawn>(GetAvatarActorFromActorInfo());
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

		AHPVisualProjectile* VisualProjectile = GetWorld()->SpawnActor<AHPVisualProjectile>(
		VisualProjectileClass,
		SourceLocation,
		SpawnRotation,
		SpawnParams);

		if (bIsUlt)
		{
			if (UltTagEffect)
			{
				FGameplayEffectContextHandle UltTagContextHandle = GetAbilitySystemComponentFromActorInfo()->MakeEffectContext();

				FGameplayEffectSpecHandle UltTagSpecHandle = GetAbilitySystemComponentFromActorInfo()->MakeOutgoingSpec(UltTagEffect, 1, UltTagContextHandle);

				if (UltTagSpecHandle.IsValid())
				{
					GetAbilitySystemComponentFromActorInfo()->ApplyGameplayEffectSpecToSelf(*UltTagSpecHandle.Data.Get());
				}
			}
		}
		//NEXTTHINGTODO: 비주얼용 투사체 꾸미기
		
	}
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}

