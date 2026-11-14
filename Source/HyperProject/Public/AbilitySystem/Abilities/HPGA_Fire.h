// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "HPDamageAbility.h"
#include "HPGA_FireBase.h"
#include "AbilitySystem/Abilities/HPGameplayAbility.h"
#include "HPGA_Fire.generated.h"

/**
 * 
 */
UCLASS()
class HYPERPROJECT_API UHPGA_Fire : public UHPGA_FireBase
{
	GENERATED_BODY()

public:
	UHPGA_Fire();

private:
	virtual void FireOneShot() override;
};
