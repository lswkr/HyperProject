// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/Abilities/HPGameplayAbility.h"
#include "HPGA_SpawnProjectile_AN.generated.h"

/**
 * 
 */
class UAbilityTask_PlayMontageAndWait;
class AHPProjectileBase;

UCLASS()
class HYPERPROJECT_API UHPGA_SpawnProjectile_AN : public UHPGameplayAbility
{
	GENERATED_BODY()

public:
	UHPGA_SpawnProjectile_AN();

protected:
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;

private:
	UPROPERTY(EditDefaultsOnly)
	UAnimMontage* FireMontage;

	FDelegateHandle TargetDataDelegateHandle;

	bool MakeTargetData(FGameplayAbilityTargetDataHandle& OutTargetDataHandle) const;
	void FireOneShot();

	void OnServerReceiveTargetData (const FGameplayAbilityTargetDataHandle& TargetDataHandle, FGameplayTag ApplicationTag);
	void SendTargetDataToServer(const FGameplayAbilityTargetDataHandle& TargetDataHandle) const;

	UFUNCTION()
	void SpawnProjectile(FGameplayEventData Payload);

	UPROPERTY()
	UAbilityTask_PlayMontageAndWait* PlayFireMontageTask;

	UPROPERTY(EditDefaultsOnly)
	EProjectileSpawnSocketType SpawnSocketType;

	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<AHPProjectileBase> ProjectileClass;

	const FGameplayAbilityTargetData* TargetData;
};
