// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "HPGA_FireBase.h"

#include "HPGA_HybridFire.generated.h"

class AHPProjectileBase;
class AHPVisualProjectile;
enum class EFireSocketType : uint8;
/**
 * 
 */
UCLASS()
class HYPERPROJECT_API UHPGA_HybridFire : public UHPGA_FireBase
{
	GENERATED_BODY()
	
public:
	UHPGA_HybridFire();

private:
	virtual void FireOneShot() override;
};
