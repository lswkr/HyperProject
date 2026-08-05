// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "HPDamageAbility.h"
#include "AbilitySystem/Abilities/HPGameplayAbility.h"
#include "HPGA_Fire.generated.h"

/**
 * 
 */
UCLASS()
class HYPERPROJECT_API UHPGA_Fire : public UHPDamageAbility
{
	GENERATED_BODY()

public:
	UHPGA_Fire();

protected:
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;

private:
	UFUNCTION()
	void OnInputReleased(float TimeHeld);

	bool MakeTargetData(FGameplayAbilityTargetDataHandle& OutTargetDataHandle) const;
	void FireOneShot();
	
	// UPROPERTY()
	// UGameplayEffect* ApplyEffect; //힐, 딜 등 적용시킬 이펙트 달라 ApplyEffect로 이름 지음

	UPROPERTY(EditDefaultsOnly)
	UAnimMontage* FireMontage;

	UPROPERTY(EditDefaultsOnly)
	UAnimMontage* ReloadMontage;

	UPROPERTY(EditDefaultsOnly)
	float FireInterval = 0.3f;

	UPROPERTY(EditDefaultsOnly)
	FGameplayTag BeamGameplayCueTag;
	
	FDelegateHandle TargetDataDelegateHandle;

	void OnServerReceiveTargetData (const FGameplayAbilityTargetDataHandle& TargetDataHandle, FGameplayTag ApplicationTag);
	void SendTargetDataToServer(const FGameplayAbilityTargetDataHandle& TargetDataHandle) const;
	void ApplyHitGameplayEffect(const FGameplayAbilityTargetDataHandle& TargetDataHandle);

	UFUNCTION()
	void OnFireDelayFinished();
	void StartFireDelay();
};
