// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/Abilities/HPGameplayAbility.h"

#include "AbilitySystemComponent.h"
#include "AbilitySystem/HPGameplayAbilityTypes.h"
#include "Characters/Player/HPPlayerCharacter.h"
#include "Controller/HPPlayerController.h"

FProjectileParams UHPGameplayAbility::MakeProjectileParams()
{
	FProjectileParams ProjectileParams;

	ProjectileParams.SourceASC = GetAbilitySystemComponentFromActorInfo();
	ProjectileParams.WorldContextObject = GetAvatarActorFromActorInfo();
	
	return ProjectileParams;
}
AHPPlayerCharacter* UHPGameplayAbility::GetHPPlayerCharacterFromActorInfo()
{   
	if (!CachedPlayerCharacter.IsValid())
	{
		CachedPlayerCharacter = Cast<AHPPlayerCharacter>(CurrentActorInfo->AvatarActor);
	}
   
	return CachedPlayerCharacter.IsValid()? CachedPlayerCharacter.Get() : nullptr;
}

AHPPlayerController* UHPGameplayAbility::GetHPPlayerControllerFromActorInfo()
{	
	if (!CachedPlayerController.IsValid())
	{
		CachedPlayerController = Cast<AHPPlayerController>(CurrentActorInfo->PlayerController);
	}

	return CachedPlayerController.IsValid()? CachedPlayerController.Get() : nullptr;
}