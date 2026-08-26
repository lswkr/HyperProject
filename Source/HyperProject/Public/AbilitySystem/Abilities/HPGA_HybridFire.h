// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/Abilities/HPGameplayAbility.h"
#include "HPGA_HybridFire.generated.h"

class AHPProjectileBase;
class AHPVisualProjectile;
enum class EProjectileSpawnSocketType : uint8;
/**
 * 
 */
UCLASS()
class HYPERPROJECT_API UHPGA_HybridFire : public UHPGameplayAbility
{
	GENERATED_BODY()
	
public:
	UHPGA_HybridFire();

protected:
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;

private:
	/* HPGameplayAbility Settings Begin */
	UPROPERTY(EditDefaultsOnly, Category="HPGameplayAbility|Montage")
	UAnimMontage* FireMontage;

	UPROPERTY(EditDefaultsOnly, Category="HPGameplayAbility|Montage")
	UAnimMontage* ReloadMontage;

	UPROPERTY(EditDefaultsOnly, Category="HPGameplayAbility|Montage")
	UAnimMontage* AimingFireMontage;
	
	UPROPERTY(EditDefaultsOnly, Category="HPGameplayAbility")
	float PrimaryFireInterval = 0.3f;
	
	UPROPERTY(EditDefaultsOnly, Category="HPGameplayAbility")
	float SecondaryFireInterval = 1.f;

	UPROPERTY(EditDefaultsOnly, Category = "HPGameplayAbility")
	bool bIsForBoth = false;

	UPROPERTY(EditDefaultsOnly, Category = "HPGameplayAbility")
	bool DoesContainAiming = false;

	bool IsAiming() const;
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

	bool MakeTargetData_HitScan(FGameplayAbilityTargetDataHandle& OutTargetDataHandle);
	bool MakeTargetData_Projectile(FGameplayAbilityTargetDataHandle& OutTargetDataHandle) const;

	void FireOneShot();
	void Fire_HitScan();
	void Fire_Projectile();
	
	FDelegateHandle TargetDataDelegateHandle_Projectile;
	FDelegateHandle TargetDataDelegateHandle_HitScan;

	void OnServerReceiveTargetData_HitScan (const FGameplayAbilityTargetDataHandle& TargetDataHandle, FGameplayTag ApplicationTag);
	void SendTargetDataToServer_HitScan(const FGameplayAbilityTargetDataHandle& TargetDataHandle) const;
	void OnServerReceiveTargetData_Projectile (const FGameplayAbilityTargetDataHandle& TargetDataHandle, FGameplayTag ApplicationTag);
	void SendTargetDataToServer_Projectile(const FGameplayAbilityTargetDataHandle& TargetDataHandle) const;
	
	void ApplyHitGameplayEffect(const FGameplayAbilityTargetDataHandle& TargetDataHandle);

	UFUNCTION()
	void OnFireDelayFinished();
	void StartFireDelay();
	/* Server-Client Logic End */

	/* Gameplay Effect Begin */
	UPROPERTY(EditDefaultsOnly, Category = "HPGameplayAbility|Effects")
	TSubclassOf<UGameplayEffect> PrimaryEffectClass;

	UPROPERTY(EditDefaultsOnly, Category = "HPGameplayAbility|Effects")
	TSubclassOf<UGameplayEffect> SecondaryEffectClass;

	UPROPERTY(EditDefaultsOnly, Category = "HPGameplayAbility|Effects")
	FScalableFloat PrimaryValue;

	UPROPERTY(EditDefaultsOnly, Category = "HPGameplayAbility|Effects")
	FScalableFloat SecondaryValue;
	/* Gameplay Effect End */

	/* Projectile Begin */
	UPROPERTY(EditDefaultsOnly, Category = "HPGameplayAbility|HPProjectile")
	TSubclassOf<AHPProjectileBase> ProjectileClass;

	UPROPERTY(EditDefaultsOnly, Category = "HPGameplayAbility|HPProjectile")
	EProjectileSpawnSocketType SpawnSocketType = EProjectileSpawnSocketType::Weapon;

	UPROPERTY(EditDefaultsOnly, Category = "HPGameplayAbility|HPProjectile")
	TSubclassOf<AHPProjectileBase> ServerSideRewindProjectileClass;

	UPROPERTY(EditDefaultsOnly, Category = "HPGameplayAbility|HPProjectile")
	TSubclassOf<AHPVisualProjectile> VisualProjectileClass;
	/* Projectile End */
};
