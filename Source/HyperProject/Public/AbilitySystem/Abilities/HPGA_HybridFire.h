// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/Abilities/HPGameplayAbility.h"
#include "HPGA_HybridFire.generated.h"

class AHPProjectileBase;
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
	UFUNCTION()
	void OnInputReleased(float TimeHeld);

	bool MakeTargetData_HitScan(FGameplayAbilityTargetDataHandle& OutTargetDataHandle) const;
	bool MakeTargetData_Projectile(FGameplayAbilityTargetDataHandle& OutTargetDataHandle) const;
	void FireOneShot();
	
	void Fire_HitScan();
	void Fire_Projectile();
	
	UPROPERTY()
	UGameplayEffect* ApplyEffect;

	UPROPERTY(EditDefaultsOnly)
	UAnimMontage* FireMontage;

	UPROPERTY(EditDefaultsOnly)
	UAnimMontage* ReloadMontage;

	UPROPERTY(EditDefaultsOnly)
	float PrimaryFireInterval = 0.3f;

	UPROPERTY(EditDefaultsOnly)
	float SecondaryFireInterval = 1.f;

	UPROPERTY(EditDefaultsOnly)
	FGameplayTag BeamGameplayCueTag;
	
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

	bool IsAiming() const;

	UPROPERTY(EditDefaultsOnly, Category = "Effects")
	TSubclassOf<UGameplayEffect> PrimaryEffect;

	UPROPERTY(EditDefaultsOnly, Category = "Effects")
	TSubclassOf<UGameplayEffect> SecondaryEffect;

	UPROPERTY(EditDefaultsOnly, Category = "Effects")
	FScalableFloat AppliedValue;

	UPROPERTY(EditDefaultsOnly, Category = "HPProjectile")
	TSubclassOf<AHPProjectileBase> ProjectileClass;

	UPROPERTY(EditDefaultsOnly, Category = "HPProjectile")
	EProjectileSpawnSocketType SpawnSocketType = EProjectileSpawnSocketType::Weapon;
};
