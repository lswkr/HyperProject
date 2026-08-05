// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "AbilitySystem/HPGameplayAbilityTypes.h"
#include "HPGameplayAbility.generated.h"

/**
 * 
 */

UCLASS()
class HYPERPROJECT_API UHPGameplayAbility : public UGameplayAbility
{
	GENERATED_BODY()

protected:
	FProjectileParams MakeProjectileParams();
};
