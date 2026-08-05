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
protected:
	virtual void BeginPlay() override;

public:
	virtual void Tick(float DeltaTime) override;

	void SetProjectileEffectParams(const FProjectileParams& InProjectileParams);

	FORCEINLINE bool CanHeadShot() const { return bCanHeadShot; }
	
	FORCEINLINE bool IsMine() const { return bIsMine;}
	void BindExplosionCallbackFunction(AActor* PlayerCharacter);
	
protected:
	//Projectile종류마다 Mesh의 사용여부나 ProjectileMovementComponent사용설정이 달라 Subobject는 각 Projectile의 종류에 따라 붙이기
	UPROPERTY(EditDefaultsOnly, Category = "HPProjectile")
	UProjectileMovementComponent* ProjectileMovementComponent;

	UPROPERTY(EditDefaultsOnly, Category = "HPProjectile")
	float InitialSpeed = 15000.f;

	UPROPERTY(EditDefaultsOnly, Category = "Projectile")
	float GravityScale = 0.f;

	UPROPERTY(EditDefaultsOnly, Category = "HPProjectile")
	UBoxComponent* BoxComponent;

	UPROPERTY(EditDefaultsOnly, Category = "HPProjectile")
	UStaticMeshComponent* ProjectileMesh;

	FTimerHandle DestroyTimerHandle;

	UPROPERTY(EditDefaultsOnly, Category = "HPProjectile")
	float DestroyTime = 3.f;

	UFUNCTION()
	virtual void OnBoxComponentHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);

	UPROPERTY(EditDefaultsOnly, Category = "HPProjectile")
	TSubclassOf<UGameplayEffect> DamageEffectClass;

	UPROPERTY(EditDefaultsOnly, Category = "HPProjectile")
	TSubclassOf<UGameplayEffect> AdditionalEffectClass;

	UPROPERTY(EditDefaultsOnly, Category = "HPProjectile")
	FScalableFloat Damage;

	UPROPERTY(EditDefaultsOnly, Category = "HPProjectile")
	FScalableFloat AdditionalValue;

	FProjectileParams ProjectileParams;

	UPROPERTY(EditDefaultsOnly, Category = "HPProjectile")
	bool bCanHeadShot = false;

	#if WITH_EDITOR
		virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
	#endif
		
	UPROPERTY(VisibleDefaultsOnly)
	UProjectileMovementComponent* ProjectileMovement;

	UPROPERTY(EditDefaultsOnly, Category = "HPProjectile")
	bool bIsForMyTeam = false;

	

	UPROPERTY(EditDefaultsOnly, Category = "HPProjectile")
	bool bIsMine = false;

	FOnMineExplodeDelegate OnMineExplodeDelegate;

	/* IGenericTeam Interface Begin*/

public:
	virtual void SetGenericTeamId(const FGenericTeamId& NewTeamID) override;
	
	virtual FGenericTeamId GetGenericTeamId() const override { return TeamID; }
	
private:
	UPROPERTY(Replicated)
	FGenericTeamId TeamID;

	/* IGenericTeam Interface End*/

	
};
