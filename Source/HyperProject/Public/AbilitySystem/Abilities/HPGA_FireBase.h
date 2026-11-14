// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/Abilities/HPGameplayAbility.h"
#include "HPGA_FireBase.generated.h"

/**
 *
 */

class UAbilitySystemComponent;
class AHPVisualProjectile;

UENUM(BlueprintType)
enum class EFireType : uint8
{
	HitScan,
	Projectile,
	BothType
};

UCLASS()
class HYPERPROJECT_API UHPGA_FireBase : public UHPGameplayAbility
{
	GENERATED_BODY()

public:
	UHPGA_FireBase();

protected:
	
	/* GameplayAbility Override Begin */
	
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;
	
	/* GameplayAbility Override End */

	
	/* Fire Flow Begin */
	
	virtual void FireOneShot() {}

	void Fire_HitScan();
	void Fire_Projectile();

	void StartFireDelay();

	UFUNCTION()
	void OnFireDelayFinished();

	UFUNCTION()
	void OnInputReleased(float TimeHeld);

	bool bEndReserved = false;

	/* Fire Flow End */

	
	/* HitScan Begin */

	void OnServerReceiveTargetData_HitScan(const FGameplayAbilityTargetDataHandle& TargetDataHandle, FGameplayTag ApplicationTag);
	void SendTargetDataToServer_HitScan(const FGameplayAbilityTargetDataHandle& TargetDataHandle) const;
	void ApplyHitGameplayEffect(const FGameplayAbilityTargetDataHandle& TargetDataHandle);
	bool MakeTargetData_HitScan(FGameplayAbilityTargetDataHandle& OutTargetDataHandle);

	/* HitScan End */

	
	/* Projectile Logic Begin */

	bool MakeTargetData_Projectile(FGameplayAbilityTargetDataHandle& OutTargetDataHandle) const;
	void SendTargetDataToServer_Projectile(const FGameplayAbilityTargetDataHandle& TargetDataHandle) const;
	void OnServerReceiveTargetData_Projectile(const FGameplayAbilityTargetDataHandle& TargetDataHandle, FGameplayTag ApplicationTag);

	/* Projectile Logic End */

	
	/* Projectile Begin */
	
	UPROPERTY(EditDefaultsOnly, Category = "HPGameplayAbility|HPProjectile")
	TSubclassOf<AHPProjectileBase> ProjectileClass;

	UPROPERTY(EditDefaultsOnly, Category = "HPGameplayAbility|HPProjectile")
	TSubclassOf<AHPProjectileBase> ServerSideRewindProjectileClass;

	UPROPERTY(EditDefaultsOnly, Category = "HPGameplayAbility|HPProjectile")
	TSubclassOf<AHPVisualProjectile> VisualProjectileClass;

	UPROPERTY(EditDefaultsOnly, Category = "HPGameplayAbility|HPProjectile")
	EFireSocketType FireSocketType = EFireSocketType::Weapon;

	/* Projectile End */

	
	/* Server TargetData Delegate Begin */

	FDelegateHandle TargetDataDelegateHandle_HitScan;
	FDelegateHandle TargetDataDelegateHandle_Projectile;

	/* Server TargetData Delegate End */

	
	/* Status Check Begin */

	bool IsAiming() const;
	bool IsNanoBoosted() const;

	/* Status Check End */

	
	/* Base Fire Settings Begin */

	UPROPERTY(EditDefaultsOnly, Category = "HPGameplayAbility|BaseFireSettings")
	bool bDoesContainAiming = false;

	UPROPERTY(EditDefaultsOnly, Category = "HPGameplayAbility|BaseFireSettings")
	bool bDoesNeedCoolDown = false;
	
	UPROPERTY(EditDefaultsOnly, Category = "HPGameplayAbility|BaseFireSettings")
	EFireType FireType = EFireType::HitScan;

	UPROPERTY(EditDefaultsOnly, Category = "HPGameplayAbility|BaseFireSettings")
	float FireInterval = 0.3f;

	/* Base Fire Settings End */

	
	/* Gameplay Effect Begin */

	UPROPERTY(EditDefaultsOnly, Category = "HPGameplayAbility|Effects")
	TSubclassOf<UGameplayEffect> TeamEffectClass;

	UPROPERTY(EditDefaultsOnly, Category = "HPGameplayAbility|Effects")
	TSubclassOf<UGameplayEffect> EnemyEffectClass;

	UPROPERTY(EditDefaultsOnly, Category = "HPGameplayAbility|Effects")
	FScalableFloat TeamEffectValue;

	UPROPERTY(EditDefaultsOnly, Category = "HPGameplayAbility|Effects")
	FScalableFloat EnemyEffectValue;

	/* Gameplay Effect End */

	
	/* Montages Begin */

	UPROPERTY(EditDefaultsOnly, Category = "HPGameplayAbility|Montage")
	UAnimMontage* FireMontage;

	UPROPERTY(EditDefaultsOnly, Category = "HPGameplayAbility|Montage")
	UAnimMontage* AimingFireMontage;

	UPROPERTY(EditDefaultsOnly, Category = "HPGameplayAbility|Montage")
	float FirePlayRate = 2.0f;

	UPROPERTY(EditDefaultsOnly, Category = "HPGameplayAbility|Montage")
	float AimingPlayRate = 1.0f;

	/* Montages End */

	
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
};
