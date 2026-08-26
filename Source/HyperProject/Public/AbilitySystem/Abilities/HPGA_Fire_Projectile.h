// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/Abilities/HPDamageAbility.h"
#include "HPGA_Fire_Projectile.generated.h"

/**
 * 
 */


class AHPProjectileBase; 
class AHPVisualProjectile;
/*
 * 투사체에 데미지나 힐 같은 이펙트를 넣을 것이기에 DamageGameAbility를 상속하지 않고
 * 기본 어빌리티 클래스 상속
 */
UCLASS()
class HYPERPROJECT_API UHPGA_Fire_Projectile : public UHPGameplayAbility
{
	GENERATED_BODY()

public:
	UHPGA_Fire_Projectile();
	
protected:
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;
private:
	/* Server-Client Logic Begin */
	UFUNCTION()
	void OnInputReleased(float TimeHeld);

	void FireOneShot();

	UFUNCTION()
	void OnFireDelayFinished();
	void StartFireDelay();

	FDelegateHandle TargetDataDelegateHandle;
	bool MakeTargetData(FGameplayAbilityTargetDataHandle& OutTargetDataHandle);
	void SendTargetDataToServer(const FGameplayAbilityTargetDataHandle& TargetDataHandle);
	void OnServerReceiveTargetData(const FGameplayAbilityTargetDataHandle& GameplayAbilityTargetDataHandle, FGameplayTag GameplayTag);
	/* Server-Client Logic End */

	/* HPGameplayAbility Settings Begin */ 
	UPROPERTY(EditDefaultsOnly)
	UAnimMontage* FireMontage;

	UPROPERTY(EditDefaultsOnly)
	UAnimMontage* ReloadMontage;

	UPROPERTY(EditDefaultsOnly)
	float FireInterval = 0.75f;
	
	UPROPERTY(EditDefaultsOnly)
	bool bNeedCooldown = false;
	/* HPGameplayAbility Settings End */
	
	/* Projectile Begin */
	UPROPERTY(EditDefaultsOnly, Category = "HPGameplayAbility|HPProjectile")
	TSubclassOf<AHPProjectileBase> ProjectileClass;

	UPROPERTY(EditDefaultsOnly, Category = "HPGameplayAbility|HPProjectile")
	TSubclassOf<AHPProjectileBase> ServerSideRewindProjectileClass;

	UPROPERTY(EditDefaultsOnly, Category = "HPGameplayAbility|HPProjectile")
	TSubclassOf<AHPVisualProjectile> VisualProjectileClass;

	UPROPERTY(EditDefaultsOnly, Category = "HPGameplayAbility|HPProjectile")
	EProjectileSpawnSocketType SpawnSocketType;
	/* Projectile End */
	
	
};
