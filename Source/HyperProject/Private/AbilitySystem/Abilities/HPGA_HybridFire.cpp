// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/Abilities/HPGA_HybridFire.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "HPGameplayTags.h"
#include "Abilities/Tasks/AbilityTask_WaitDelay.h"
#include "Abilities/Tasks/AbilityTask_WaitInputRelease.h"
#include "AbilitySystem/HPAttributeSet.h"
#include "AbilitySystem/Abilities/HPGA_Fire_Projectile.h"
#include "Characters/HPCharacterBase.h"
#include "Components/HPCombatComponent.h"
#include "Interfaces/CombatInterface.h"
#include "Weapons/HPProjectileBase.h"

UHPGA_HybridFire::UHPGA_HybridFire()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::LocalPredicted;
}

void UHPGA_HybridFire::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
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
		TargetDataDelegateHandle_HitScan = ASC->AbilityTargetDataSetDelegate(
			Handle,
			ActivationInfo.GetActivationPredictionKey()).
		AddUObject(this, &UHPGA_HybridFire::OnServerReceiveTargetData_HitScan);
		//데이터가 먼저 도착했을 경우를 대비해 한 번 call
		TargetDataDelegateHandle_Projectile = ASC->AbilityTargetDataSetDelegate(
			Handle,
			ActivationInfo.GetActivationPredictionKey()).
		AddUObject(this, &UHPGA_HybridFire::OnServerReceiveTargetData_Projectile);
		
		ASC->CallReplicatedTargetDataDelegatesIfSet(
			Handle,
			ActivationInfo.GetActivationPredictionKey()
			);
	}

	UAbilityTask_WaitInputRelease* ReleaseTask = UAbilityTask_WaitInputRelease::WaitInputRelease(this,true);
	ReleaseTask->OnRelease.AddDynamic(this, &UHPGA_HybridFire::OnInputReleased);

	ReleaseTask->ReadyForActivation();

	if (IsLocallyControlled()) //로컬에서 우선적인 판단
	{
		FireOneShot();
	}
	
}

void UHPGA_HybridFire::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	UAbilitySystemComponent* ASC = ActorInfo? ActorInfo->AbilitySystemComponent.Get():nullptr;

	if (ASC && TargetDataDelegateHandle_HitScan.IsValid())
	{
		ASC->AbilityTargetDataSetDelegate(Handle, ActivationInfo.GetActivationPredictionKey()).Remove(TargetDataDelegateHandle_HitScan);
		TargetDataDelegateHandle_HitScan.Reset();
	}

	if (ASC && TargetDataDelegateHandle_Projectile.IsValid())
	{
		ASC->AbilityTargetDataSetDelegate(Handle, ActivationInfo.GetActivationPredictionKey()).Remove(TargetDataDelegateHandle_HitScan);
		TargetDataDelegateHandle_Projectile.Reset();
	}
	
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

void UHPGA_HybridFire::OnInputReleased(float TimeHeld)
{
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}

bool UHPGA_HybridFire::MakeTargetData_HitScan(FGameplayAbilityTargetDataHandle& OutTargetDataHandle) const
{
	AHPCharacterBase* HPCharacter = Cast<AHPCharacterBase>(GetAvatarActorFromActorInfo());

	if (!HPCharacter)
	{
		return false;
	}

	UHPCombatComponent* CombatComponent = HPCharacter->GetCombatComponent();

	if (!CombatComponent)
	{
		return false;
	}

	FVector MuzzleLocation;
	FHitResult HitResult;

	CombatComponent->ReturnHitTargetFromMuzzleSocket(MuzzleLocation, HitResult);

	FGameplayCueParameters CueParams;
	CueParams.Location = MuzzleLocation;
	HitResult.Location  = MuzzleLocation;


	if (!HitResult.bBlockingHit)
	{
		UE_LOG(LogTemp, Warning, TEXT("None Blocking"));
	}
	
	FGameplayEffectContextHandle ContextHandle =  GetAbilitySystemComponentFromActorInfo()->MakeEffectContext();
	ContextHandle.AddHitResult(HitResult);
	CueParams.EffectContext = ContextHandle;

	GetAbilitySystemComponentFromActorInfo()->ExecuteGameplayCue(BeamGameplayCueTag, CueParams);
	
	if (HitResult.ImpactPoint.ContainsNaN())
	{
		return false;
	}

	DrawDebugSphere(GetWorld(),HitResult.ImpactPoint, 10,10, FColor::Yellow, false, 10);
	OutTargetDataHandle.Add(new FGameplayAbilityTargetData_SingleTargetHit(HitResult));
	return true;
}

bool UHPGA_HybridFire::MakeTargetData_Projectile(FGameplayAbilityTargetDataHandle& OutTargetDataHandle) const
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

void UHPGA_HybridFire::FireOneShot()
{
	if (IsAiming())
	{
		Fire_HitScan();
	}
	else
	{
		Fire_Projectile();
	}
}

void UHPGA_HybridFire::Fire_HitScan()
{
	if (!IsLocallyControlled()) //로컬에서 쏘므로
	{
		return;
	}

	UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo();

	if (!ASC)
		return;
	
	FGameplayAbilityTargetDataHandle TargetDataHandle;

	if (!MakeTargetData_HitScan(TargetDataHandle))
	{
		EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
		return;
	}

	bool bShouldReload = false; //윈도우 내부의 값을 저장하기 위한 변수

	{
		FScopedPredictionWindow PredictionWindow(ASC,true);//현 Prediction Key에 묶기

		ASC->PlayMontage(this, CurrentActivationInfo, FireMontage,2.0f);

		if (!CommitAbilityCost(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo))
		{
			EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
			return;
		}
		
		const float CurrentBullet = ASC->GetNumericAttribute(UHPAttributeSet::GetBulletAttribute());
		bShouldReload = CurrentBullet<=0.f;

		if (IsPredictingClient()) //현재 Prediction Window로 서버에 전달
		{
			SendTargetDataToServer_HitScan(TargetDataHandle);
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

void UHPGA_HybridFire::Fire_Projectile()
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

	if (!MakeTargetData_Projectile(TargetDataHandle))
	{
		EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
		return;
	}
	UE_LOG(LogTemp,Warning,TEXT("AfterMakeTargetData"));
	bool bShouldReload = false; //윈도우 내부의 값을 저장하기 위한 변수

	{
		FScopedPredictionWindow PredictionWindow(ASC,true);//현 Prediction Key에 묶기

		ASC->PlayMontage(this, CurrentActivationInfo, FireMontage, 2.0f);

		if (!CommitAbilityCost(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo))
		{
			EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
			return;
		}
		
		const float CurrentBullet = ASC->GetNumericAttribute(UHPAttributeSet::GetBulletAttribute());
		bShouldReload = CurrentBullet<=0.f;

		if (IsPredictingClient()) //현재 Prediction Window로 서버에 전달
		{
			UE_LOG(LogTemp,Warning,TEXT("Call SendTargetDataToServer"));
			SendTargetDataToServer_Projectile(TargetDataHandle);
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

void UHPGA_HybridFire::OnServerReceiveTargetData_HitScan(const FGameplayAbilityTargetDataHandle& TargetDataHandle,
	FGameplayTag ApplicationTag)
{
	if (!HasAuthority(&CurrentActivationInfo))
	{
		return;
	}

	if (!ApplicationTag.MatchesTagExact(FHPGameplayTags::Get().ApplicationTag_Aiming))
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
	
	FGameplayCueParameters CueParams;
	CueParams.Location = Data->GetHitResult()->Location;
	
	FGameplayEffectContextHandle ContextHandle =  ASC->MakeEffectContext();
	ContextHandle.AddHitResult(*Data->GetHitResult());
	CueParams.EffectContext = ContextHandle;
	
	GetAbilitySystemComponentFromActorInfo()->ExecuteGameplayCue(BeamGameplayCueTag, CueParams);
	
	//ConsumeClientReplicatedTargetData이후 ASC캐시가 지워지는 것을 대비
	const FGameplayAbilityTargetDataHandle CopiedTargetDataHandle = TargetDataHandle;

	ASC->ConsumeClientReplicatedTargetData(
		CurrentSpecHandle,
		CurrentActivationInfo.GetActivationPredictionKey()
		);

	//UE_LOG(LogTemp, Warning, TEXT("OnServerReceiveTargetData"));
	if (!CommitAbilityCost(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo))
	{
		EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
		return;
	}

	const float ServerBullet =
	   ASC->GetNumericAttribute(UHPAttributeSet::GetBulletAttribute());

	// UE_LOG(
	// 	LogTemp,
	// 	Warning,
	// 	TEXT("SERVER: Shot committed, Bullet=%f"),
	// 	ServerBullet
	// );
	ApplyHitGameplayEffect(CopiedTargetDataHandle);
}

void UHPGA_HybridFire::SendTargetDataToServer_HitScan(const FGameplayAbilityTargetDataHandle& TargetDataHandle) const
{
	UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo();
	if (!ASC)
	{
		return;
	}
	if (!IsAiming())
		return;
	ASC->CallServerSetReplicatedTargetData
	(
		CurrentSpecHandle,
		CurrentActivationInfo.GetActivationPredictionKey(),
		TargetDataHandle,
		FHPGameplayTags::Get().ApplicationTag_Aiming,
		ASC->ScopedPredictionKey
	);
}

void UHPGA_HybridFire::OnServerReceiveTargetData_Projectile(const FGameplayAbilityTargetDataHandle& TargetDataHandle,
	FGameplayTag ApplicationTag)
{
	if (!ApplicationTag.MatchesTagExact(FHPGameplayTags::Get().ApplicationTag_Normal))
	{
		return;
	}
	const FGameplayAbilityTargetData* Data = TargetDataHandle.Get(0);
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

void UHPGA_HybridFire::SendTargetDataToServer_Projectile(const FGameplayAbilityTargetDataHandle& TargetDataHandle) const
{
	UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo();
	if (!ASC)
	{
		return;
	}
	if (IsAiming())
		return;
	UE_LOG(LogTemp,Warning,TEXT("SendTargetDataToServer"));
	ASC->CallServerSetReplicatedTargetData
	(
		CurrentSpecHandle,
		CurrentActivationInfo.GetActivationPredictionKey(),
		TargetDataHandle,
		FHPGameplayTags::Get().ApplicationTag_Normal,
		ASC->ScopedPredictionKey
	);
}

void UHPGA_HybridFire::ApplyHitGameplayEffect(const FGameplayAbilityTargetDataHandle& TargetDataHandle)
{
	//저격 시에는 SecondaryEffect
	if (!HasAuthority(&CurrentActivationInfo) || !SecondaryEffect||!TargetDataHandle.Num())
	{
		return;
	}

	FHPGameplayTags GameplayTags = FHPGameplayTags::Get();
	UAbilitySystemComponent* SourceASC = GetAbilitySystemComponentFromActorInfo();
	FGameplayEffectContextHandle Context = SourceASC->MakeEffectContext();

	AHPCharacterBase* HPCharacter = Cast<AHPCharacterBase>(GetAvatarActorFromActorInfo());


	float AimingDuration = 0.f;
	
	UHPCombatComponent* CombatComponent = HPCharacter->GetCombatComponent();

	if (CombatComponent)
	{
		CombatComponent->CaptureAimEndTime();
		AimingDuration = CombatComponent->GetDurationBetweenAim();
		CombatComponent->CaptureAimStartTime();
	}

	float FinalValue= AppliedValue.GetValueAtLevel(1);

	if (SourceASC->HasMatchingGameplayTag(FGameplayTag::RequestGameplayTag(FName("State.Combat.Aiming"))) && AimingDuration>0.f )
	{
		FinalValue*=FMath::Clamp<float>(1+AimingDuration, 1.f, 3.f);	
	}
	
	const FHitResult* HitResult = TargetDataHandle.Get(0)->GetHitResult();
	if (HitResult)
	{
		if (!HitResult->bBlockingHit)
			return;
		if (HitResult->BoneName.ToString()==FString("head"))
		{
			//NEXTTHINGTODO: 헤드샷 위젯 표시
			UE_LOG(LogTemp,Warning,TEXT("HEADSHOT"));
			FinalValue*=2;
		}

		FGameplayEffectSpecHandle EffectSpecHandle = SourceASC->MakeOutgoingSpec(SecondaryEffect,1,Context);
		UAbilitySystemBlueprintLibrary::AssignTagSetByCallerMagnitude(EffectSpecHandle, GameplayTags.SetByCaller_IncomingDamage, FinalValue);
	
		if (!EffectSpecHandle.IsValid())
		{
			return;
		}
		EffectSpecHandle.Data->GetContext().AddHitResult(*HitResult, true);
		DrawDebugSphere(GetWorld(),HitResult->ImpactPoint, 20,10, FColor::Red, false, 10);

			//
			// if (IGenericTeamAgentInterface* OwnerTeamInterface = Cast<IGenericTeamAgentInterface>(HPCharacter))
			// {
			// 	ETeamAttitude::Type OtherActorTeamAttitude = OwnerTeamInterface->GetTeamAttitudeTowards(*HitResult->GetActor());
			// 	if (OtherActorTeamAttitude == ETeamAttitude::Hostile)
			// 	{
					ApplyGameplayEffectSpecToTarget(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, EffectSpecHandle, TargetDataHandle);
			// 	}
			// }
		
	}
}

void UHPGA_HybridFire::OnFireDelayFinished()
{
	FireOneShot();
}

void UHPGA_HybridFire::StartFireDelay()
{
	float FireInterval = IsAiming() ? PrimaryFireInterval : SecondaryFireInterval;
	UAbilityTask_WaitDelay* DelayTask = UAbilityTask_WaitDelay::WaitDelay(this, FireInterval);

	DelayTask->OnFinish.AddDynamic(this, &UHPGA_HybridFire::OnFireDelayFinished);

	DelayTask->ReadyForActivation();
}

bool UHPGA_HybridFire::IsAiming() const
{
	if (!GetAbilitySystemComponentFromActorInfo())
		return false;

	return	GetAbilitySystemComponentFromActorInfo()->HasMatchingGameplayTag(FHPGameplayTags::Get().State_Combat_Aiming);
}

