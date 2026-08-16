// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/Abilities/HPGA_Fire.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "GenericTeamAgentInterface.h"
#include "HPGameplayTags.h"
#include "Abilities/Tasks/AbilityTask_WaitInputRelease.h"
#include "Abilities/Tasks/AbilityTask_WaitDelay.h"
#include "AbilitySystem/HPAttributeSet.h"

#include "Characters/HPCharacterBase.h"
#include "Characters/Player/HPPlayerCharacter.h"
#include "Components/HPCombatComponent.h"
#include "Components/LagCompensationComponent.h"
#include "Controller/HPPlayerController.h"
#include "Interfaces/CombatInterface.h"
#include "Kismet/GameplayStatics.h"

UHPGA_Fire::UHPGA_Fire()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::LocalPredicted;
}

void UHPGA_Fire::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
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
		
		TargetDataDelegateHandle = ASC->AbilityTargetDataSetDelegate(
			Handle,
			ActivationInfo.GetActivationPredictionKey()).
		AddUObject(this, &UHPGA_Fire::OnServerReceiveTargetData);

		//데이터가 먼저 도착했을 경우를 대비해 한 번 call
		ASC->CallReplicatedTargetDataDelegatesIfSet(
			Handle,
			ActivationInfo.GetActivationPredictionKey()
			);
	}

	UAbilityTask_WaitInputRelease* ReleaseTask = UAbilityTask_WaitInputRelease::WaitInputRelease(this,true);
	ReleaseTask->OnRelease.AddDynamic(this, &UHPGA_Fire::OnInputReleased);

	ReleaseTask->ReadyForActivation();

	if (IsLocallyControlled()) //로컬에서 우선적인 판단
	{
		FireOneShot();
	}
}

void UHPGA_Fire::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	UAbilitySystemComponent* ASC = ActorInfo? ActorInfo->AbilitySystemComponent.Get():nullptr;

	if (ASC && TargetDataDelegateHandle.IsValid())
	{
		ASC->AbilityTargetDataSetDelegate(Handle, ActivationInfo.GetActivationPredictionKey()).Remove(TargetDataDelegateHandle);
		TargetDataDelegateHandle.Reset();
	}

	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

void UHPGA_Fire::OnInputReleased(float TimeHeld)
{
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}

bool UHPGA_Fire::MakeTargetData(FGameplayAbilityTargetDataHandle& OutTargetDataHandle) 
{
	FVector MuzzleLocation;
	FVector EndLocation;
	bool bUseServerSideRewind = false;
	if (AActor* PlayerActor = GetAvatarActorFromActorInfo())
	{
		if (PlayerActor->Implements<UCombatInterface>())
		{
			MuzzleLocation = ICombatInterface::Execute_GetWeaponSocketLocation(PlayerActor);
			EndLocation = ICombatInterface::Execute_GetHitImpactPoint(PlayerActor);
			bUseServerSideRewind=ICombatInterface::Execute_IsUsingServerRewind(PlayerActor);
		}
		else
		{
			return false;
		}
	}

	FVector End = MuzzleLocation + (EndLocation - MuzzleLocation)*1.25f;

	FHitResult HitResult;
	GetWorld()->LineTraceSingleByChannel(
		HitResult,
		MuzzleLocation,
		End,
		ECollisionChannel::ECC_Visibility
	);
	
	if (!HitResult.bBlockingHit)
	{
		HitResult.ImpactPoint = End;
	}
	
	float HitTime = 0.f;
	
	if (bUseServerSideRewind)
	{
		if (AHPPlayerController* HPOwnerController = Cast<AHPPlayerController>(GetCurrentActorInfo()->PlayerController.Get()))
		{
			HitTime = HPOwnerController->GetServerTime() - HPOwnerController->SingleTripTime;
		}
		else
		{
			return false;
		}
	}

	AHPPlayerCharacter* HPPlayerCharacter= GetHPPlayerCharacterFromActorInfo();
	
	if (!HPPlayerCharacter)
	{
		return false;
	}
	
	UHPCombatComponent* CombatComponent = HPPlayerCharacter->GetCombatComponent();
	
		
	float AimingDuration = 1.f;

	if (CombatComponent)
	{
		CombatComponent->CaptureAimEndTime();
		AimingDuration = CombatComponent->GetDurationBetweenAim();
		CombatComponent->CaptureAimStartTime();
	}
	
	FGameplayCueParameters CueParams;
	CueParams.Location = MuzzleLocation;
	HitResult.Location  = MuzzleLocation;

	FGameplayEffectContextHandle ContextHandle =  GetAbilitySystemComponentFromActorInfo()->MakeEffectContext();
	ContextHandle.AddHitResult(HitResult);
	CueParams.EffectContext = ContextHandle;

	GetAbilitySystemComponentFromActorInfo()->ExecuteGameplayCue(BeamGameplayCueTag, CueParams);
	
	if (HitResult.ImpactPoint.ContainsNaN())
	{
		return false;
	}
	FHPGameplayTags GameplayTags = FHPGameplayTags::Get();
	bool bIsAiming = GetAbilitySystemComponentFromActorInfo()->HasMatchingGameplayTag(GameplayTags.State_Combat_Aiming);
	bool bIsNanoBoosted = GetAbilitySystemComponentFromActorInfo()->HasMatchingGameplayTag(GameplayTags.State_Combat_NanoBoosted);

	DrawDebugSphere(GetWorld(),HitResult.ImpactPoint, 10,10, FColor::Yellow, false, 10);
	OutTargetDataHandle.Add(new FGameplayAbilityTargetData_HPCustom(HitResult,MuzzleLocation,HitTime, AimingDuration, bIsNanoBoosted, bIsAiming));
	return true;
}

void UHPGA_Fire::FireOneShot()
{
	if (!IsLocallyControlled()) //로컬에서 쏘므로
	{
		return;
	}

	UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo();

	if (!ASC)
		return;
	
	FGameplayAbilityTargetDataHandle TargetDataHandle;

	if (!MakeTargetData(TargetDataHandle))
	{
		EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
		return;
	}

	bool bShouldReload = false; 
	{
		FScopedPredictionWindow PredictionWindow(ASC,true);//현 Prediction Key에 묶기

		ASC->PlayMontage(this, CurrentActivationInfo, FireMontage,2.0f);

		if (!CommitAbilityCost(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo))
		{
			EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
			return;
		}

		if (IsPredictingClient()) //현재 Prediction Window로 서버에 전달
		{
			SendTargetDataToServer(TargetDataHandle);
		}
	
	}
	const float CurrentBullet = ASC->GetNumericAttribute(UHPAttributeSet::GetBulletAttribute());
	bShouldReload = CurrentBullet<=0.f;
	
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

void UHPGA_Fire::OnServerReceiveTargetData(const FGameplayAbilityTargetDataHandle& TargetDataHandle,
                                           FGameplayTag ApplicationTag)
{
	if (!HasAuthority(&CurrentActivationInfo))
	{
		return;
	}

	UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo();

	if (!ASC)
	{
		return;
	}

	if (!IsLocallyControlled())
	{
		ASC->PlayMontage(this, CurrentActivationInfo, FireMontage,2.0f);
	}

	const FGameplayAbilityTargetData* Data = TargetDataHandle.Get(0);
	
	FGameplayEffectContextHandle ContextHandle =  ASC->MakeEffectContext();
	ContextHandle.AddHitResult(*Data->GetHitResult());

	FGameplayCueParameters CueParams;
	CueParams.Location = Data->GetHitResult()->Location;
	CueParams.EffectContext = ContextHandle;
	
	GetAbilitySystemComponentFromActorInfo()->ExecuteGameplayCue(BeamGameplayCueTag, CueParams);
	
	//ConsumeClientReplicatedTargetData이후 ASC캐시가 지워지는 것을 대비
	const FGameplayAbilityTargetDataHandle CopiedTargetDataHandle = TargetDataHandle;

	ASC->ConsumeClientReplicatedTargetData(
		CurrentSpecHandle,
		CurrentActivationInfo.GetActivationPredictionKey()
		);
	if (!CommitAbilityCost(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo))
	{
		EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
		return;
	}
	
	ApplyHitGameplayEffect(CopiedTargetDataHandle);
}

void UHPGA_Fire::SendTargetDataToServer(const FGameplayAbilityTargetDataHandle& TargetDataHandle) const
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

void UHPGA_Fire::ApplyHitGameplayEffect(const FGameplayAbilityTargetDataHandle& TargetDataHandle)
{
	if (!HasAuthority(&CurrentActivationInfo) || !DamageEffectClass||!TargetDataHandle.Num())
	{
		return;
	}

	FHPGameplayTags GameplayTags = FHPGameplayTags::Get();
	UAbilitySystemComponent* SourceASC = GetAbilitySystemComponentFromActorInfo();
	FGameplayEffectContextHandle Context = SourceASC->MakeEffectContext();

	AHPPlayerCharacter* HPCharacter = GetHPPlayerCharacterFromActorInfo();
	float AimingDuration = 0.f;
	
	float FinalDamage = Damage.GetValueAtLevel(1);
	
	const FGameplayAbilityTargetData* BaseData = TargetDataHandle.Get(0);
	const FGameplayAbilityTargetData_HPCustom* TargetData = nullptr ;
	
	if (BaseData && BaseData->GetScriptStruct() == FGameplayAbilityTargetData_HPCustom::StaticStruct())
	{
		TargetData = static_cast<const FGameplayAbilityTargetData_HPCustom*>(BaseData);
	}
	
	if (TargetData)
	{
		const FHitResult* HitResult = TargetData->GetHitResult();
		bool bIsHeadShot = false;
		bool bIsShotConfirmed = false;
		bool bIsNanoBoosted = TargetData->GetNanoBoosted();
		bool bIsAiming = TargetData->IsAiming();
		
		AHPPlayerCharacter* HitCharacter = Cast<AHPPlayerCharacter>(HitResult->GetActor());
		
		if (HitCharacter)
		{
			FServerSideRewindResult SSRResult = HPCharacter->GetLagCompensationComponent()->ServerSideRewind(HitCharacter,TargetData->GetStartPoint(),HitResult->ImpactPoint, TargetData->GetHitTime());

			bIsHeadShot = SSRResult.bHeadShot;
			bIsShotConfirmed = SSRResult.bHitConfirmed;
		}
	
		if (bIsShotConfirmed)
		{
			if (!HitResult->bBlockingHit)
				return;

			
			if (bIsHeadShot)
			{
				//NEXTTHINGTODO: 헤드샷 위젯 표시
				UE_LOG(LogTemp,Warning,TEXT("HEADSHOT"));
				FinalDamage*=2;
			}

			if (bIsAiming)
			{
				FinalDamage*=FMath::Clamp<float>(1+AimingDuration, 1.f, 3.f);	
			}

			if (bIsNanoBoosted)
			{
				FinalDamage*=1.5;
			}
			
			FGameplayEffectSpecHandle EffectSpecHandle = SourceASC->MakeOutgoingSpec(DamageEffectClass,1,Context);
			UAbilitySystemBlueprintLibrary::AssignTagSetByCallerMagnitude(EffectSpecHandle, GameplayTags.SetByCaller_IncomingDamage, FinalDamage);
			
			if (!EffectSpecHandle.IsValid())
			{
				return;
			}
			EffectSpecHandle.Data->GetContext().AddHitResult(*HitResult, true);
			DrawDebugSphere(GetWorld(),HitResult->ImpactPoint, 20,10, FColor::Red, false, 10);

			
			if (IGenericTeamAgentInterface* OwnerTeamInterface = Cast<IGenericTeamAgentInterface>(HPCharacter))
			{
				ETeamAttitude::Type OtherActorTeamAttitude = OwnerTeamInterface->GetTeamAttitudeTowards(*HitCharacter);
				if (OtherActorTeamAttitude == ETeamAttitude::Hostile)
				{
					if (HitCharacter->GetAbilitySystemComponent())
					{
						HitCharacter->GetAbilitySystemComponent()->ApplyGameplayEffectSpecToSelf(*EffectSpecHandle.Data.Get());
	
						FGameplayCueParameters GameplayCueParams;
						GameplayCueParams.Location = HitResult->ImpactPoint;

						HitCharacter->GetAbilitySystemComponent()->ExecuteGameplayCue(HitVFXCueTag, GameplayCueParams);
						HitCharacter->GetAbilitySystemComponent()->ExecuteGameplayCue(HitSoundCueTag,GameplayCueParams);
					}

					//ApplyGameplayEffectSpecToTarget(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, EffectSpecHandle, TargetDataHandle);
				}
			}
		
		}
	}
}


void UHPGA_Fire::StartFireDelay()
{
	//UE_LOG(LogTemp, Warning, TEXT("StartFireDelay"));
	UAbilityTask_WaitDelay* DelayTask = UAbilityTask_WaitDelay::WaitDelay(this, FireInterval);

	DelayTask->OnFinish.AddDynamic(this, &UHPGA_Fire::OnFireDelayFinished);

	DelayTask->ReadyForActivation();
}

void UHPGA_Fire::OnFireDelayFinished()
{
	FireOneShot();
}
