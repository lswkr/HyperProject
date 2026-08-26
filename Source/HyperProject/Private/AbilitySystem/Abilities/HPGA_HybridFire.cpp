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
#include "Characters/Player/HPPlayerCharacter.h"
#include "Components/HPCombatComponent.h"
#include "Components/LagCompensationComponent.h"
#include "Controller/HPPlayerController.h"
#include "HyperProject/HyperProject.h"
#include "Interfaces/CombatInterface.h"
#include "Kismet/GameplayStatics.h"
#include "Weapons/HPProjectileBase.h"
#include "Weapons/HPVisualProjectile.h"

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
	
	if (IsForRemoteClient())
	{
		TargetDataDelegateHandle_HitScan = ASC->AbilityTargetDataSetDelegate(
			Handle,
			ActivationInfo.GetActivationPredictionKey()).
		AddUObject(this, &UHPGA_HybridFire::OnServerReceiveTargetData_HitScan);

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

	if (IsLocallyControlled())
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
		ASC->AbilityTargetDataSetDelegate(Handle, ActivationInfo.GetActivationPredictionKey()).Remove(TargetDataDelegateHandle_Projectile);
		TargetDataDelegateHandle_Projectile.Reset();
	}
	
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

void UHPGA_HybridFire::OnInputReleased(float TimeHeld)
{
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}

bool UHPGA_HybridFire::MakeTargetData_HitScan(FGameplayAbilityTargetDataHandle& OutTargetDataHandle) 
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
	else
	{
		//여기에서는 자신의 로컬만 효과가 보이는 곳
		//그래서 Muzzle에서 나오는 이펙트나 총 소리는 모든 클라에서 재생되는 Montage에 넣음
		UGameplayStatics::SpawnEmitterAtLocation(
			GetAvatarActorFromActorInfo(),
			LocalHitParticle,
			HitResult.ImpactPoint,
			HitResult.ImpactNormal.Rotation()
		);
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

	//Beam이펙트는 판정과 큰 상관이 없어 여기서 쏜다.
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

bool UHPGA_HybridFire::MakeTargetData_Projectile(FGameplayAbilityTargetDataHandle& OutTargetDataHandle) const
{
	// if (!GetAvatarActorFromActorInfo()->Implements<UCombatInterface>())
	// return false;
	//
	// UE_LOG(LogTemp,Warning,TEXT("MakeTargetData"));
	// FVector HitImpactPoint = ICombatInterface::Execute_GetHitImpactPoint(GetAvatarActorFromActorInfo());
	//
	// FGameplayEffectContextHandle ContextHandle =  GetAbilitySystemComponentFromActorInfo()->MakeEffectContext();
	//
	//
	// //GetAbilitySystemComponentFromActorInfo()->ExecuteGameplayCue(BeamGameplayCueTag, CueParams);
	//
	// FGameplayAbilityTargetData_LocationInfo* LocationData = new FGameplayAbilityTargetData_LocationInfo();
	//
	// LocationData->TargetLocation.LocationType =
	// 	EGameplayAbilityTargetingLocationType::LiteralTransform;
	//
	// LocationData->TargetLocation.LiteralTransform = FTransform(FQuat::Identity, HitImpactPoint);
	//
	// OutTargetDataHandle.Add(LocationData);
	// return true;

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
	
	if (!IsLocallyControlled() || !IsAiming()) //로컬에서 쏘므로
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
			SendTargetDataToServer_HitScan(TargetDataHandle);
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

void UHPGA_HybridFire::Fire_Projectile()
{
	if (!IsLocallyControlled() || IsAiming()) //로컬에서 쏘므로
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

		if (!MakeTargetData_Projectile(TargetDataHandle))
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
		
			ASC->PlayMontage(this, CurrentActivationInfo, FireMontage, 2.0f);

			if (IsPredictingClient()) //현재 Prediction Window로 서버에 전달
			{
				SendTargetDataToServer_Projectile(TargetDataHandle);
			}
	
		}
	}

	else
	{
		FGameplayAbilityTargetDataHandle TargetDataHandle;

		if (!MakeTargetData_Projectile(TargetDataHandle))
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
		
			ASC->PlayMontage(this, CurrentActivationInfo, FireMontage, 2.0f);
		
			

			if (IsPredictingClient()) //현재 Prediction Window로 서버에 전달
			{
				SendTargetDataToServer_Projectile(TargetDataHandle);
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
		if (!DoesContainAiming)
		{
			ASC->PlayMontage(this, CurrentActivationInfo, FireMontage,2.0f);
		}
		else
		{
			const FGameplayAbilityTargetData* BaseData = TargetDataHandle.Get(0);
			const FGameplayAbilityTargetData_HPCustom* TargetData = nullptr ;
			
			if (BaseData && BaseData->GetScriptStruct() == FGameplayAbilityTargetData_HPCustom::StaticStruct())
			{
				TargetData = static_cast<const FGameplayAbilityTargetData_HPCustom*>(BaseData);
				if (TargetData->IsAiming())
				{
					if (AimingFireMontage) //Belica는 Aiming상태에서의 FireMontage가 따로 없음
					{
						ASC->PlayMontage(this, CurrentActivationInfo, AimingFireMontage,1.0f);
					}
				}
			}
		}
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
	// if (!ApplicationTag.MatchesTagExact(FHPGameplayTags::Get().ApplicationTag_Normal))
	// {
	// 	return;
	// }
	// const FGameplayAbilityTargetData* Data = TargetDataHandle.Get(0);
	// UE_LOG(LogTemp, Warning, TEXT("OnServerReceiveTargetData"));
	// if (!Data)
	// {
	// 	return;
	// }
	//
	// FVector TargetPoint = Data->GetEndPoint();
	// FVector ProjectileSpawnPoint;
	// if (GetAvatarActorFromActorInfo()->Implements<UCombatInterface>())
	// {
	// 	ProjectileSpawnPoint = SpawnSocketType==EProjectileSpawnSocketType::Weapon ? ICombatInterface::Execute_GetWeaponSocketLocation(GetAvatarActorFromActorInfo()): ICombatInterface::Execute_GetThrowingHandSocketLocation(GetAvatarActorFromActorInfo());
	// }
	// UE_LOG(LogTemp, Warning, TEXT("OnServerReceiveTargetData Rotation"));
	// FRotator SpawnRotation = (TargetPoint - ProjectileSpawnPoint).Rotation();
	//
	// FTransform SpawnTransform;
	// SpawnTransform.SetLocation(ProjectileSpawnPoint);
	// SpawnTransform.SetRotation(SpawnRotation.Quaternion());
	//
	// AHPProjectileBase* Projectile = GetWorld()->SpawnActorDeferred<AHPProjectileBase>(
	// 	ProjectileClass,
	// 	SpawnTransform,
	// 	GetOwningActorFromActorInfo(),
	// 	Cast<APawn>(GetOwningActorFromActorInfo()),
	// 	ESpawnActorCollisionHandlingMethod::AlwaysSpawn);
	//
	// Projectile->SetProjectileEffectParams(MakeProjectileParams());
	//
	// Projectile->FinishSpawning(SpawnTransform);

	if (!ApplicationTag.MatchesTagExact(FHPGameplayTags::Get().ApplicationTag_Normal))
	{
		return;
	}

	if (!CommitAbilityCost(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo))
	{
		EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
		return;
	}
	
	const FGameplayAbilityTargetData* Data = TargetDataHandle.Get(0);

	if (!Data)
	{
		return;
	}
	
	if (Data && Data->GetScriptStruct() == FGameplayAbilityTargetData_LocationInfo::StaticStruct())
	{
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
		//NEXTTHINGTODO: 비주얼 용 액터 생성

	}
}

void UHPGA_HybridFire::SendTargetDataToServer_Projectile(const FGameplayAbilityTargetDataHandle& TargetDataHandle) const
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
		FHPGameplayTags::Get().ApplicationTag_Normal,
		ASC->ScopedPredictionKey
	);
}

void UHPGA_HybridFire::ApplyHitGameplayEffect(const FGameplayAbilityTargetDataHandle& TargetDataHandle)
{
	// //저격 시에는 SecondaryEffect
	// if (!HasAuthority(&CurrentActivationInfo) || !SecondaryEffect||!TargetDataHandle.Num())
	// {
	// 	return;
	// }
	//
	// FHPGameplayTags GameplayTags = FHPGameplayTags::Get();
	// UAbilitySystemComponent* SourceASC = GetAbilitySystemComponentFromActorInfo();
	// FGameplayEffectContextHandle Context = SourceASC->MakeEffectContext();
	//
	// AHPCharacterBase* HPCharacter = Cast<AHPCharacterBase>(GetAvatarActorFromActorInfo());
	//
	//
	// float AimingDuration = 0.f;
	//
	// UHPCombatComponent* CombatComponent = HPCharacter->GetCombatComponent();
	//
	// if (CombatComponent)
	// {
	// 	CombatComponent->CaptureAimEndTime();
	// 	AimingDuration = CombatComponent->GetDurationBetweenAim();
	// 	CombatComponent->CaptureAimStartTime();
	// }
	//
	// float FinalValue= AppliedValue.GetValueAtLevel(1);
	//
	// if (SourceASC->HasMatchingGameplayTag(FGameplayTag::RequestGameplayTag(FName("State.Combat.Aiming"))) && AimingDuration>0.f )
	// {
	// 	FinalValue*=FMath::Clamp<float>(1+AimingDuration, 1.f, 3.f);	
	// }
	//
	// const FHitResult* HitResult = TargetDataHandle.Get(0)->GetHitResult();
	// if (HitResult)
	// {
	// 	if (!HitResult->bBlockingHit)
	// 		return;
	// 	if (HitResult->BoneName.ToString()==FString("head"))
	// 	{
	// 		//NEXTTHINGTODO: 헤드샷 위젯 표시
	// 		UE_LOG(LogTemp,Warning,TEXT("HEADSHOT"));
	// 		FinalValue*=2;
	// 	}
	//
	// 	FGameplayEffectSpecHandle EffectSpecHandle = SourceASC->MakeOutgoingSpec(SecondaryEffect,1,Context);
	// 	UAbilitySystemBlueprintLibrary::AssignTagSetByCallerMagnitude(EffectSpecHandle, GameplayTags.SetByCaller_IncomingDamage, FinalValue);
	//
	// 	if (!EffectSpecHandle.IsValid())
	// 	{
	// 		return;
	// 	}
	// 	EffectSpecHandle.Data->GetContext().AddHitResult(*HitResult, true);
	// 	DrawDebugSphere(GetWorld(),HitResult->ImpactPoint, 20,10, FColor::Red, false, 10);
	//
	// 		//
	// 		// if (IGenericTeamAgentInterface* OwnerTeamInterface = Cast<IGenericTeamAgentInterface>(HPCharacter))
	// 		// {
	// 		// 	ETeamAttitude::Type OtherActorTeamAttitude = OwnerTeamInterface->GetTeamAttitudeTowards(*HitResult->GetActor());
	// 		// 	if (OtherActorTeamAttitude == ETeamAttitude::Hostile)
	// 		// 	{
	// 				ApplyGameplayEffectSpecToTarget(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, EffectSpecHandle, TargetDataHandle);
	// 		// 	}
	// 		// }
	// 	
	// }

	if (!HasAuthority(&CurrentActivationInfo) || !PrimaryEffectClass||!TargetDataHandle.Num())
	{
		return;
	}

	FHPGameplayTags GameplayTags = FHPGameplayTags::Get();
	UAbilitySystemComponent* SourceASC = GetAbilitySystemComponentFromActorInfo();
	FGameplayEffectContextHandle Context = SourceASC->MakeEffectContext();

	AHPPlayerCharacter* HPCharacter = GetHPPlayerCharacterFromActorInfo();

	
	float Damage = PrimaryValue.GetValueAtLevel(1);
	
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
		float AimingDuration = TargetData->GetAimingDuration();
		AHPPlayerCharacter* HitCharacter = Cast<AHPPlayerCharacter>(HitResult->GetActor());
		
		if (HitCharacter)
		{
			FServerSideRewindResult SSRResult = HPCharacter->GetLagCompensationComponent()->ServerSideRewind(HitCharacter,TargetData->GetStartPoint(),HitResult->ImpactPoint, TargetData->GetHitTime());

			bIsHeadShot = SSRResult.bHeadShot;
			bIsShotConfirmed = SSRResult.bHitConfirmed;
		}
		else
		{
			FGameplayCueParameters GameplayCueParams;
			GameplayCueParams.Location = HitResult->ImpactPoint;
			GetAbilitySystemComponentFromActorInfo()->ExecuteGameplayCue(HitVFXCueTag, GameplayCueParams);
			return;
		}
		
		float Multiplier = 1.f;
		
		if (bIsShotConfirmed)
		{
			if (!HitResult->bBlockingHit)
				return;

			
			if (bIsHeadShot)
			{
				Multiplier*=2;
			}

			if (bIsAiming)
			{
				Multiplier*=FMath::Clamp<float>(1+AimingDuration, 1.f, 3.f);	
			}

			if (bIsNanoBoosted)
			{
				Multiplier*=1.5;
			}
			
			FGameplayEffectSpecHandle EffectSpecHandle = SourceASC->MakeOutgoingSpec(PrimaryEffectClass,1,Context);
			UAbilitySystemBlueprintLibrary::AssignTagSetByCallerMagnitude(EffectSpecHandle, GameplayTags.SetByCaller_IncomingDamage, Damage*Multiplier);
			
			if (!EffectSpecHandle.IsValid())
			{
				return;
			}
			DrawDebugSphere(GetWorld(),HitResult->ImpactPoint, 20,10, FColor::Red, false, 10);

			
			if (IGenericTeamAgentInterface* OwnerTeamInterface = Cast<IGenericTeamAgentInterface>(HPCharacter))
			{
				ETeamAttitude::Type OtherActorTeamAttitude = OwnerTeamInterface->GetTeamAttitudeTowards(*HitCharacter);
				if (OtherActorTeamAttitude == ETeamAttitude::Hostile)
				{
					
					UE_LOG(LogTemp ,Warning,TEXT("Deal"));
					HitCharacter->GetAbilitySystemComponent()->ApplyGameplayEffectSpecToSelf(*EffectSpecHandle.Data.Get());					
					//ApplyGameplayEffectSpecToTarget(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, EffectSpecHandle, TargetDataHandle);
				}

				else if (bIsForBoth)
				{
					float Heal = SecondaryValue.GetValueAtLevel(1);


					FGameplayEffectSpecHandle SecondSpecHandle = SourceASC->MakeOutgoingSpec(SecondaryEffectClass,1,Context);
					UAbilitySystemBlueprintLibrary::AssignTagSetByCallerMagnitude(SecondSpecHandle, GameplayTags.SetByCaller_IncomingHeal, Heal*Multiplier);
					UE_LOG(LogTemp, Warning,TEXT("HEAL: %f"),Heal*Multiplier );
					//ApplyGameplayEffectSpecToTarget(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, SecondSpecHandle, TargetDataHandle);

					HitCharacter->GetAbilitySystemComponent()->ApplyGameplayEffectSpecToSelf(*SecondSpecHandle.Data.Get());
				}
				
				if (HitCharacter->GetAbilitySystemComponent())
				{
					FGameplayCueParameters GameplayCueParams;
					GameplayCueParams.Location = HitResult->ImpactPoint;

					HitCharacter->GetAbilitySystemComponent()->ExecuteGameplayCue(HitVFXCueTag, GameplayCueParams);
					HitCharacter->GetAbilitySystemComponent()->ExecuteGameplayCue(HitSoundCueTag,GameplayCueParams);

					if (bIsHeadShot)
					{
						HitCharacter->GetAbilitySystemComponent()->ExecuteGameplayCue(LocalHeadHitSoundCueTag,GameplayCueParams);
					}
					else
					{
						HitCharacter->GetAbilitySystemComponent()->ExecuteGameplayCue(LocalBodyHitSoundCueTag,GameplayCueParams);
					}
					HPCharacter->Client_HitConfirm(bIsHeadShot);
				}
			}
			
		
		}
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

