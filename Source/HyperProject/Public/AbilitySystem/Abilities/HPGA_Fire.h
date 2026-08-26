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
	/* HPGameplayAbility Settings Begin */
	UPROPERTY(EditDefaultsOnly, Category="HPGameplayAbility|Montage")
	UAnimMontage* FireMontage;

	UPROPERTY(EditDefaultsOnly, Category="HPGameplayAbility|Montage")
	UAnimMontage* AimingFireMontage;

	UPROPERTY(EditDefaultsOnly, Category="HPGameplayAbility|Montage")
	UAnimMontage* ReloadMontage;

	UPROPERTY(EditDefaultsOnly, Category="HPGameplayAbility")
	bool DoesContainAiming = false;

	UPROPERTY(EditDefaultsOnly, Category="HPGameplayAbility")
	float FireInterval = 0.3f;
	/* HPGameplayAbility Settings End */

	/* FX Begin */
	UPROPERTY(EditDefaultsOnly, Category="HPGameplayAbility|VFX")
	FGameplayTag BeamGameplayCueTag;

	UPROPERTY(EditDefaultsOnly, Category="HPGameplayAbility|VFX")
	FGameplayTag HitVFXCueTag;

	UPROPERTY(EditDefaultsOnly, Category="HPGameplayAbility|VFX")
	UParticleSystem* LocalHitParticle;
	
	UPROPERTY(EditDefaultsOnly, Category="HPGameplayAbility|SFX")
	FGameplayTag HitSoundCueTag;

	UPROPERTY(EditDefaultsOnly, Category="HPGameplayAbility|SFX")
	FGameplayTag LocalBodyHitSoundCueTag;

	UPROPERTY(EditDefaultsOnly, Category="HPGameplayAbility|SFX")
	FGameplayTag LocalHeadHitSoundCueTag;
	/* FX End */
	
	/* Server-Client Logic Begin */
	UFUNCTION()
	void OnInputReleased(float TimeHeld);
	
	bool MakeTargetData(FGameplayAbilityTargetDataHandle& OutTargetDataHandle);
	
	void FireOneShot();

	FDelegateHandle TargetDataDelegateHandle;

	void OnServerReceiveTargetData (const FGameplayAbilityTargetDataHandle& TargetDataHandle, FGameplayTag ApplicationTag);
	void SendTargetDataToServer(const FGameplayAbilityTargetDataHandle& TargetDataHandle) const;
	void ApplyHitGameplayEffect(const FGameplayAbilityTargetDataHandle& TargetDataHandle);

	UFUNCTION()
	void OnFireDelayFinished();
	void StartFireDelay();
	/* Server-Client Logic End */
	
};
