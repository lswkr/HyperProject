// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "HPGA_FireBase.h"
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
class HYPERPROJECT_API UHPGA_Fire_Projectile : public UHPGA_FireBase
{
	GENERATED_BODY()

public:
 	UHPGA_Fire_Projectile();
	
private:
	virtual void FireOneShot() override;

	
};
