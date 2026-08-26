// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/Abilities/HPGameplayAbility.h"
#include "HPDamageAbility.generated.h"

/**
 * 
 */



UCLASS()
class HYPERPROJECT_API UHPDamageAbility : public UHPGameplayAbility
{
	GENERATED_BODY()

protected:
	UPROPERTY(EditDefaultsOnly, Category = "HPGameplayAbility")
	TSubclassOf<UGameplayEffect> DamageEffectClass;
	
	UPROPERTY(EditDefaultsOnly, Category = "HPGameplayAbility")
	FScalableFloat Damage;
};
