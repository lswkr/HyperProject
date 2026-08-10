// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/Abilities/HPGameplayAbility.h"
#include "HPGA_SendEventToDetectedActor.generated.h"

/**
 * 
 */
UCLASS()
class HYPERPROJECT_API UHPGA_SendEventToDetectedActor : public UHPGameplayAbility
{
	GENERATED_BODY()

public:
	UHPGA_SendEventToDetectedActor();

protected:
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;

private:
	UFUNCTION()
	void OnInputPressed(float TimeHeld);

	bool MakeTargetData(FGameplayAbilityTargetDataHandle& OutTargetDataHandle, AActor* ConfirmedActor) const;
	
	UPROPERTY(EditDefaultsOnly)
	UAnimMontage* FireMontage;
	
	UPROPERTY(EditDefaultsOnly)
	FGameplayTag BeamGameplayCueTag;
	
	FDelegateHandle TargetDataDelegateHandle;

	void OnServerReceiveTargetData (const FGameplayAbilityTargetDataHandle& TargetDataHandle, FGameplayTag ApplicationTag);
	void SendTargetDataToServer(const FGameplayAbilityTargetDataHandle& TargetDataHandle) const;

	UPROPERTY(EditDefaultsOnly)
	FGameplayTag EventTag;
};
