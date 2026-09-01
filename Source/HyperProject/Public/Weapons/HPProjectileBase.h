// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GenericTeamAgentInterface.h"
#include "ScalableFloat.h"
#include "AbilitySystem/HPGameplayAbilityTypes.h"
#include "GameFramework/Actor.h"
#include "HPProjectileBase.generated.h"

DECLARE_DYNAMIC_DELEGATE(FOnMineExplodeDelegate);

class UProjectileMovementComponent;
class UBoxComponent;
class UGameplayEffect;
class AHPPlayerCharacter;

UCLASS()
class HYPERPROJECT_API AHPProjectileBase : public AActor, public IGenericTeamAgentInterface
{
	GENERATED_BODY()
	
public:	
	AHPProjectileBase();

	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;

	void SetPlayerCharactersToIgnore(TArray<AHPPlayerCharacter*> PlayerCharactersToIgnore);
	
protected:
	virtual void BeginPlay() override;

protected:

#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif
	
	UPROPERTY(EditDefaultsOnly, Category = "HPProjectile")
	UStaticMeshComponent* BulletMesh;

	UPROPERTY(EditDefaultsOnly, Category = "HPProjectile")
	UBoxComponent* BoxComponent;

	UPROPERTY(EditDefaultsOnly, Category = "HPProjectile")
	FVector BoxExtent;

	//ProjectileMovement 추적 시, 판정 구의 반경을 가로길이 또는 세로길이로 쓰도록
	UPROPERTY(EditDefaultsOnly, Category = "HPProjectile")
	float FrontSideWidth;
	
	UFUNCTION()
	virtual void OnBoxComponentHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);

	FTimerHandle DestroyTimerHandle;

	UPROPERTY(EditDefaultsOnly, Category = "HPProjectile")
	float DestroyTime = 3.f;

	UPROPERTY(EditDefaultsOnly, Category = "HPProjectile")
	EEffectApplyTargetPolicy EffectApplyTargetPolicy = EEffectApplyTargetPolicy::EnemyOnly;
	/* Effect Begin*/
public:
	void SetProjectileParams(const FProjectileParams& InProjectileParams);

	FORCEINLINE bool CanHeadShot() const { return bCanHeadShot; }

	virtual void MakeProjectileEffectParams(FProjectileApplyEffectParams& ProjectileApplyEffectParams);
	//FORCEINLINE bool IsMine() const { return bIsMine;}
	//void BindExplosionCallbackFunction(AActor* PlayerCharacter);

protected:
	UPROPERTY(EditDefaultsOnly, Category = "HPProjectile")
	TSubclassOf<UGameplayEffect> EnemyEffectClass;

	UPROPERTY(EditDefaultsOnly, Category = "HPProjectile")
	TSubclassOf<UGameplayEffect> TeamEffectClass;

	UPROPERTY(EditDefaultsOnly, Category = "HPProjectile")
	TArray<TSubclassOf<UGameplayEffect>> AdditionalEnemyEffectClasses;

	UPROPERTY(EditDefaultsOnly, Category = "HPProjectile")
	TArray<TSubclassOf<UGameplayEffect>> AdditionalTeamEffectClasses;

	UPROPERTY(EditDefaultsOnly, Category = "HPProjectile")
	FScalableFloat EnemyEffectValue;

	UPROPERTY(EditDefaultsOnly, Category = "HPProjectile")
	FScalableFloat TeamEffectValue;

	UPROPERTY(EditDefaultsOnly, Category = "HPProjectile")
	bool bCanHeadShot = false;

	// UPROPERTY(EditDefaultsOnly, Category = "HPProjectile")
	// bool bIsMine = false;

	// FOnMineExplodeDelegate OnMineExplodeDelegate;
	//
	FProjectileParams ProjectileParams;

	EEffectApplyTargetPolicy EffectType = EEffectApplyTargetPolicy::EnemyOnly;
	/* Effect End*/

	
		

	/* Movement Begin*/
	
protected:
	//Projectile종류마다 ProjectileMovementComponent사용설정이 달라 Subobject는 각 Projectile의 종류에 따라 붙이기
	UPROPERTY(EditDefaultsOnly, Category = "HPProjectile")
	UProjectileMovementComponent* ProjectileMovement;

	UPROPERTY(EditDefaultsOnly, Category = "HPProjectile")
	float InitialSpeed = 15000.f;

	UPROPERTY(EditDefaultsOnly, Category = "HPProjectile")
	float GravityScale = 0.f;
	/* Movement End*/

	/* IGenericTeam Interface Begin*/

public:
	virtual void SetGenericTeamId(const FGenericTeamId& NewTeamID) override;
	
	virtual FGenericTeamId GetGenericTeamId() const override { return TeamID; }
	
protected:
	UPROPERTY(Replicated)
	FGenericTeamId TeamID;

	/* IGenericTeam Interface End*/

public:
	FORCEINLINE void ShouldUseServerSideRewind (bool ShouldUse) {	bServerSideRewind = ShouldUse;	}
	FORCEINLINE void SetTraceStart(FVector InTraceStart) { TraceStart=InTraceStart; }
	FORCEINLINE void SetInitialVelocity(FVector InInitialVelocity) { InitialVelocity = InInitialVelocity;}
	FORCEINLINE float  GetInitialSpeed() const {return InitialSpeed;}
	
protected:
	bool bServerSideRewind = true;
	FVector TraceStart;
	FVector InitialVelocity;
	
	UPROPERTY(EditDefaultsOnly, Category = "HPProjectile")
	USoundBase* HitSound;

	UPROPERTY(EditDefaultsOnly, Category = "HPProjectile")
	float ProjectileLifeSpan = 3.f;
};
