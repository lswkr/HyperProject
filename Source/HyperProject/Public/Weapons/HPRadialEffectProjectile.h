// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Weapons/HPProjectileBase.h"
#include "HPRadialEffectProjectile.generated.h"

/**
 * 
 */



class AHPPlayerCharacter;

UCLASS()
class HYPERPROJECT_API AHPRadialEffectProjectile : public AHPProjectileBase
{
	GENERATED_BODY()
public:
	AHPRadialEffectProjectile();



protected:
	virtual void OnBoxComponentHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit) override;
	void ExplodeProjectile();

	virtual void Destroyed() override;
	
private:
	UPROPERTY(EditDefaultsOnly, Category = "HPProjectile")
	float ExplosionInnerRadius = 200.f;

	UPROPERTY(EditDefaultsOnly, Category = "HPProjectile")
	float ExplosionOuterRadius = 500.f;


};
