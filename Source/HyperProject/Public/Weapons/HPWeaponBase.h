// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "WeaponInterface.h"
#include "AbilitySystem/HPGameplayAbilityTypes.h"
#include "GameFramework/Actor.h"
#include "HPWeaponBase.generated.h"

class UGameplayAbility;
class UGameplayEffect;
class UNiagaraComponent;

UENUM(BlueprintType)
enum class EHPWeaponType : uint8
{
	HitScan			UMETA(DisplayName="HitScan"),
	Projectile		UMETA(DisplayName="Projectile")
};

USTRUCT(BlueprintType)
struct FHPWeaponInfo
{
	GENERATED_BODY()
	
public:
	UPROPERTY(EditDefaultsOnly)
	float Damage = 0.f;

	UPROPERTY(EditDefaultsOnly)
	float FireDelay = 0.2f;

	UPROPERTY(EditDefaultsOnly)
	float MaxBullet = 30.f; //SetByCaller로 활용

	UPROPERTY(EditDefaultsOnly)
	EHPWeaponType WeaponType = EHPWeaponType::HitScan;

	UPROPERTY(EditDefaultsOnly)
	TMap<EHPAbilityInputID, TSubclassOf<UGameplayAbility>> WeaponAbilityMap;
	
	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UGameplayEffect> WeaponInitBulletEffect;

	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UGameplayEffect> WeaponBulletChargeEffect;

	UPROPERTY(EditDefaultsOnly)
	UNiagaraSystem* WeaponVFX;
};

UCLASS()
class HYPERPROJECT_API AHPWeaponBase : public AActor, public IWeaponInterface
{
	GENERATED_BODY()
	
public:	
	AHPWeaponBase();

	UFUNCTION(BlueprintCallable)
	FORCEINLINE FHPWeaponInfo GetWeaponInfo() const { return WeaponInfo; };

	FORCEINLINE UStaticMeshComponent* GetWeaponMesh() const { return WeaponMesh; }
	
	UFUNCTION(BlueprintCallable)
	FTransform GetMuzzleSocketTransform() const;
	
	virtual UNiagaraSystem* GetWeaponNiagaraSystem_Implementation() const override;
	virtual FVector GetWeaponMuzzleLocation_Implementation() const override;

	UFUNCTION(BlueprintCallable)
	FORCEINLINE bool CanAim() { return bCanAim; }

	UFUNCTION(BlueprintCallable)
	FORCEINLINE float GetZoomedFOV() { return ZoomedFOV; }

	UFUNCTION(BlueprintCallable)
	FORCEINLINE float GetZoomInterpSpeed() { return ZoomInterpSpeed; }
	
protected:
	virtual void BeginPlay() override;

	
private:
	UPROPERTY(EditDefaultsOnly)
	UStaticMeshComponent* WeaponMesh;
	
	UPROPERTY(EditDefaultsOnly)
	FHPWeaponInfo WeaponInfo;

	UPROPERTY(EditDefaultsOnly)
	bool bCanAim = false;

	/* Aim Begin */
	UPROPERTY(EditAnywhere)
	float ZoomedFOV = 30.f;

	UPROPERTY(EditAnywhere)
	float ZoomInterpSpeed = 20.f;
	/* Aim End */
};
