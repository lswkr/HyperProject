// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "AbilitySystem/HPGameplayAbilityTypes.h"
#include "HPGameplayAbility.generated.h"

/**
 * 
 */
class AHPPlayerCharacter;
class AHPPlayerController;


UCLASS()
class HYPERPROJECT_API UHPGameplayAbility : public UGameplayAbility
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintPure)
	AHPPlayerCharacter* GetHPPlayerCharacterFromActorInfo();

	UFUNCTION(BlueprintPure)
	AHPPlayerController* GetHPPlayerControllerFromActorInfo();


private:
	TWeakObjectPtr<AHPPlayerCharacter> CachedPlayerCharacter;
	TWeakObjectPtr<AHPPlayerController> CachedPlayerController;
	
protected:
	FProjectileParams MakeProjectileParams();
	
};
