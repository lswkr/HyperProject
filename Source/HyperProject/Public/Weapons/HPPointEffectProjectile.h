// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Weapons/HPProjectileBase.h"
#include "HPPointEffectProjectile.generated.h"

/**
 * 
 */
UCLASS()
class HYPERPROJECT_API AHPPointEffectProjectile : public AHPProjectileBase
{
	GENERATED_BODY()

protected:
	virtual void OnBoxComponentHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit) override;
};
