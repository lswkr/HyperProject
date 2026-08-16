// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/Abilities/HPGA_SendEventToDetectedActor.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "Abilities/Tasks/AbilityTask_WaitInputPress.h"
#include "Characters/Player/HPPlayerCharacter_UsingDC.h"
#include "Components/DetectComponent.h"

UHPGA_SendEventToDetectedActor::UHPGA_SendEventToDetectedActor()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::LocalPredicted;
}

void UHPGA_SendEventToDetectedActor::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
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
		TargetDataDelegateHandle = ASC->AbilityTargetDataSetDelegate(
			Handle,
			ActivationInfo.GetActivationPredictionKey()).
		AddUObject(this, &UHPGA_SendEventToDetectedActor::OnServerReceiveTargetData);

		//데이터가 먼저 도착했을 경우를 대비해 한 번 call
		ASC->CallReplicatedTargetDataDelegatesIfSet(
			Handle,
			ActivationInfo.GetActivationPredictionKey()
			);
	}
	UAbilityTask_WaitInputPress* InputPressTask = UAbilityTask_WaitInputPress::WaitInputPress(this, true);

	InputPressTask->OnPress.AddDynamic(this, &UHPGA_SendEventToDetectedActor::OnInputPressed);
	InputPressTask->ReadyForActivation();
}

void UHPGA_SendEventToDetectedActor::EndAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
	bool bReplicateEndAbility, bool bWasCancelled)
{

	UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo();

	if (ASC && TargetDataDelegateHandle.IsValid())
	{
		ASC->AbilityTargetDataSetDelegate(
			Handle,
			ActivationInfo.GetActivationPredictionKey()
		).Remove(TargetDataDelegateHandle);

		TargetDataDelegateHandle.Reset();
	}

	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

void UHPGA_SendEventToDetectedActor::OnInputPressed(float TimeHeld)
{
	if (IsLocallyControlled())
	{
		UE_LOG(LogTemp, Warning, TEXT("InputPressed"));
		if (AHPPlayerCharacter_UsingDC* HPPlayerCharacter = Cast<AHPPlayerCharacter_UsingDC>(GetAvatarActorFromActorInfo()))
		{
			
			if (AActor* ConfirmedActor = HPPlayerCharacter->GetDetectComponent()->GetConfirmedActor())
			{
				UE_LOG(LogTemp, Warning, TEXT("ConfirmedActor Exist"));
				
				FGameplayAbilityTargetDataHandle TargetDataHandle;
				if (!MakeTargetData(TargetDataHandle,ConfirmedActor))
				{
					EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
					return;
				}
				
				if (!CommitAbilityCost(CurrentSpecHandle,CurrentActorInfo,CurrentActivationInfo))
				{
					EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
					return;
				}
				SendTargetDataToServer(TargetDataHandle);
				EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, false, false);
   
			}
			else
			{
				EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
				return;
			}
		}
	}
}

bool UHPGA_SendEventToDetectedActor::MakeTargetData(FGameplayAbilityTargetDataHandle& OutTargetDataHandle, AActor* ConfirmedActor) const
{
	if (!ConfirmedActor)
		return false;
	
	FGameplayAbilityTargetData_ActorArray* TargetData = new FGameplayAbilityTargetData_ActorArray();

	TargetData->TargetActorArray.Add(ConfirmedActor);

	OutTargetDataHandle.Add(TargetData);
	return true;
}

void UHPGA_SendEventToDetectedActor::OnServerReceiveTargetData(const FGameplayAbilityTargetDataHandle& TargetDataHandle,
	FGameplayTag ApplicationTag)
{
	const FGameplayAbilityTargetData* BaseData = TargetDataHandle.Get(0);
	const FGameplayAbilityTargetData_ActorArray* TargetData = nullptr ;

	UE_LOG(LogTemp, Warning, TEXT("OnServerReceiveTargetData"));
	
	if (BaseData && BaseData->GetScriptStruct() == FGameplayAbilityTargetData_ActorArray::StaticStruct())
	{
		TargetData = static_cast<const FGameplayAbilityTargetData_ActorArray*>(BaseData);
	}
	if (!TargetData || TargetData->TargetActorArray.Num() <= 0)
	{
		EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
		return;
	}

	
	AActor* ConfirmedActor = TargetData->TargetActorArray[0].Get();
	
	
	if (UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo())
	{
		ASC->ConsumeClientReplicatedTargetData(CurrentSpecHandle,CurrentActivationInfo.GetActivationPredictionKey());
	}
	if (ConfirmedActor)
	{
		if (!CommitAbilityCost(CurrentSpecHandle,CurrentActorInfo,CurrentActivationInfo))
		{
			EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
			return;
		}
		UE_LOG(LogTemp, Warning, TEXT("Confirmed Actor: %s"),*ConfirmedActor->GetName());
		FGameplayEventData Payload;
		Payload.EventTag = EventTag;
		Payload.Instigator = GetAvatarActorFromActorInfo();
		Payload.Target = ConfirmedActor;
		
		UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(ConfirmedActor, EventTag, Payload);
		EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
		return;
	}
}

void UHPGA_SendEventToDetectedActor::SendTargetDataToServer(
	const FGameplayAbilityTargetDataHandle& TargetDataHandle) const
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
