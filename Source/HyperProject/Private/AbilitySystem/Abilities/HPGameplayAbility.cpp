// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/Abilities/HPGameplayAbility.h"

#include "AbilitySystemComponent.h"
#include "AbilitySystem/HPGameplayAbilityTypes.h"

FProjectileParams UHPGameplayAbility::MakeProjectileParams()
{
	FProjectileParams ProjectileParams;

	ProjectileParams.SourceASC = GetAbilitySystemComponentFromActorInfo();
	ProjectileParams.WorldContextObject = GetAvatarActorFromActorInfo();
	
	return ProjectileParams;
}
