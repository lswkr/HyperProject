// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Weapons/HPProjectileBase.h"
#include "HPSpawningProjectile.generated.h"

/**
 * 
 */
UCLASS()
class HYPERPROJECT_API AHPSpawningProjectile : public AHPProjectileBase
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly, Category = "HPProjectile|Spawning")
	TSubclassOf<AActor> SpawnedActorClass;

public:
	virtual void MakeProjectileEffectParams(FProjectileApplyEffectParams& ProjectileApplyEffectParams) override;
protected:
	virtual void OnBoxComponentHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit) override;
	
};
