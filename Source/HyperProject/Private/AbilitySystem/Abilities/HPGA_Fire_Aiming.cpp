// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/Abilities/HPGA_Fire_Aiming.h"

#include "HPGameplayTags.h"
#include "Characters/Player/HPPlayerCharacter.h"
#include "Components/HPCombatComponent.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Abilities/Tasks/AbilityTask_WaitInputRelease.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"
#include "UI/Widget/ScopeWidget.h"

void UHPGA_Fire_Aiming::OnInputReleased(float TimeHeld)
{
	//if (HasAuthority(&CurrentActivationInfo))
	//{
	AHPPlayerCharacter* HPPlayerCharacter = GetHPPlayerCharacterFromActorInfo();
	if (HPPlayerCharacter)
	{
		if (UHPCombatComponent* CombatComponent = HPPlayerCharacter->GetCombatComponent())
		{
			CombatComponent->ClearAimTimes();
		}
	}
	//}
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}

void UHPGA_Fire_Aiming::ZoomIn(FGameplayEventData Payload)
{
	AHPPlayerCharacter* HPPlayerCharacter = Cast<AHPPlayerCharacter>(GetAvatarActorFromActorInfo());
	if (HPPlayerCharacter)
	{
		if (UHPCombatComponent* CombatComponent = HPPlayerCharacter->GetCombatComponent())
		{
			CombatComponent->SetAiming(true);
		}
	}
	APlayerController* PlayerController = GetCurrentActorInfo()->PlayerController.Get();

	if (PlayerController)
	{
		ScopeWidget = CreateWidget<UScopeWidget>(PlayerController,ScopeWidgetClass, TEXT("ScopeWidget"));
		ScopeWidget->AddToViewport();
		ScopeWidget->ZoomIn();
	}
}

void UHPGA_Fire_Aiming::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
                                        const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
                                        const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	if (IsLocallyControlled())
	{
		UAbilityTask_WaitGameplayEvent* ZoomEventTask = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(
			this,
			FHPGameplayTags::Get().Event_ZoomIn,
			nullptr,
			true,
			true
		);

		ZoomEventTask->EventReceived.AddDynamic(this, &UHPGA_Fire_Aiming::ZoomIn);
		
		ZoomEventTask->ReadyForActivation();
	}
	
	if (HasAuthorityOrPredictionKey(ActorInfo, &ActivationInfo))
	{
		
		UAbilityTask_PlayMontageAndWait* AimStartPlayMontageAndWait = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(this, NAME_None, AimStartMontage);
		//PlayTornadoMontage->OnBlendOut.AddDynamic(this, &UHPGA_Fire_Aiming::K2_EndAbility);
		AimStartPlayMontageAndWait->OnCancelled.AddDynamic(this, &UHPGA_Fire_Aiming::K2_EndAbility);
		AimStartPlayMontageAndWait->OnInterrupted.AddDynamic(this, &UHPGA_Fire_Aiming::K2_EndAbility);
		AimStartPlayMontageAndWait->OnCompleted.AddDynamic(this, &UHPGA_Fire_Aiming::CaptureAimStartTime);
		AimStartPlayMontageAndWait->ReadyForActivation();
	}
	
	UAbilityTask_WaitInputRelease* ReleaseTask = UAbilityTask_WaitInputRelease::WaitInputRelease(this,true);
	ReleaseTask->OnRelease.AddDynamic(this, &UHPGA_Fire_Aiming::OnInputReleased);
	ReleaseTask->ReadyForActivation();
}

void UHPGA_Fire_Aiming::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{

	if (HasAuthority(&CurrentActivationInfo))
	{
		AHPPlayerCharacter* HPPlayerCharacter = GetHPPlayerCharacterFromActorInfo();
		if (HPPlayerCharacter)
		{
			if (UHPCombatComponent* CombatComponent = HPPlayerCharacter->GetCombatComponent())
			{
				CombatComponent->ClearAimTimes();
			}
		}
	}
	if (IsLocallyControlled())
	{
		AHPPlayerCharacter* HPPlayerCharacter = GetHPPlayerCharacterFromActorInfo();
		if (HPPlayerCharacter)
		{
			if (UHPCombatComponent* CombatComponent = HPPlayerCharacter->GetCombatComponent())
			{
				CombatComponent->SetAiming(false);
			}
		}
		if (ScopeWidget)
		{
			ScopeWidget->ZoomOut();
		}
	}
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

void UHPGA_Fire_Aiming::CaptureAimStartTime()
{
	if (HasAuthority(&CurrentActivationInfo))
	{
		AHPPlayerCharacter* HPPlayerCharacter = GetHPPlayerCharacterFromActorInfo();
		if (HPPlayerCharacter)
		{
			if (UHPCombatComponent* CombatComponent = HPPlayerCharacter->GetCombatComponent())
			{
				CombatComponent->CaptureAimStartTime();
			}
		}
	}
}
