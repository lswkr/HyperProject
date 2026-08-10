// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "HPAbilitySystemDataAsset.generated.h"

/**
 * 
 */
class UGameplayAbility;
class UGameplayEffect;

UCLASS()
class HYPERPROJECT_API UHPAbilitySystemDataAsset : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	FORCEINLINE TSubclassOf<UGameplayEffect> GetFullStatEffect() const {return FullStatEffect;}
	FORCEINLINE TSubclassOf<UGameplayEffect> GetDeathEffect() const {return DeathEffect;}
	FORCEINLINE TArray<TSubclassOf<UGameplayEffect>>  GetInitialEffects() const {return InitialEffects;}
	FORCEINLINE TArray<TSubclassOf<UGameplayAbility>> GetPassiveAbilities() const {return PassiveAbilities;}
	FORCEINLINE TArray<TSubclassOf<UGameplayAbility>> GetPassiveAbilities_EventTriggered() const {return PassiveAbilities_EventTriggered;}
	
	FORCEINLINE UDataTable* GetBaseStatDataTable() const {return BaseStatDataTable;}
	

private:
	UPROPERTY(EditDefaultsOnly, Category = "GAS")
	TSubclassOf<UGameplayEffect> FullStatEffect;

	UPROPERTY(EditDefaultsOnly, Category = "GAS")
	TSubclassOf<UGameplayEffect> DeathEffect;

	UPROPERTY(EditDefaultsOnly, Category = "GAS")
	TArray<TSubclassOf<UGameplayEffect>> InitialEffects;

	UPROPERTY(EditDefaultsOnly, Category = "GAS")
	TArray<TSubclassOf<UGameplayAbility>> PassiveAbilities;

	UPROPERTY(EditDefaultsOnly, Category = "GAS")
	TArray<TSubclassOf<UGameplayAbility>> PassiveAbilities_EventTriggered;

	UPROPERTY(EditDefaultsOnly, Category = "GAS")
	UDataTable* BaseStatDataTable;
};
