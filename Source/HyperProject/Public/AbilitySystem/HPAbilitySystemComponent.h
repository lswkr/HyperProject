// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemComponent.h"
#include "HPGameplayAbilityTypes.h"
#include "HPGenericTypes.h"

#include "HPAbilitySystemComponent.generated.h"

/**
 * 
 */
class UHPAbilitySystemDataAsset;
class AHPWeaponBase;

DECLARE_MULTICAST_DELEGATE(FAbilitiesGivenDelegate);
DECLARE_DELEGATE_OneParam(FForEachAbilityDelegate, const FGameplayAbilitySpec&);

UCLASS()
class HYPERPROJECT_API UHPAbilitySystemComponent : public UAbilitySystemComponent
{
	GENERATED_BODY()

public:
	UHPAbilitySystemComponent();

	void InitializeBaseAttributes();
	void ApplyFullStatEffect();
	
	void ServerSideInit();

	void OnEquipWeapon(const AHPWeaponBase& NewWeapon);

	ECombatState GetCombatState() const;

	bool AbilitiesGiven = false;

	virtual void OnRep_ActivateAbilities() override;

	FAbilitiesGivenDelegate AbilitiesGivenDelegate;
	
	void ForEachAbility(const FForEachAbilityDelegate& Delegate);

	FGameplayTag GetAbilityTagFromSpec(const FGameplayAbilitySpec& AbilitySpec) const;
	FGameplayTag GetUltTagForCurrentCharacter();
	int32 GetInputIDFromSpec(const FGameplayAbilitySpec& AbilitySpec) const;

	void ApplyDeathEffect();
	void InitAbilityAndEffectAtRespawn();
	
private:
	void AuthApplyGameplayEffect(const TSubclassOf<UGameplayEffect>& GameplayEffect, int32 Level = 1);
	void ApplyInitialEffects();
	void GiveInitialAbilities();
	//void OnHealthUpdate(const FOnAttributeChangeData& ChangedData);
	//void OnUltUpdate(const FOnAttributeChangeData& ChangedData);

	UPROPERTY(EditDefaultsOnly, Category = "GAS")
	TMap<EHPAbilityInputID, TSubclassOf<UGameplayAbility>> Abilities;

	UPROPERTY(EditDefaultsOnly, Category = "GAS")
	TMap<EHPAbilityInputID, TSubclassOf<UGameplayAbility>> BasicAbilities;

	UPROPERTY(EditDefaultsOnly, Category = "GAS")
	TArray<TSubclassOf<UGameplayAbility>> BasicAbilitiesWithNoKey;
	
	UPROPERTY(EditDefaultsOnly, Category = "GAS")
	UHPAbilitySystemDataAsset* HPAbilitySystemDataAsset;

	TArray<FGameplayAbilitySpecHandle> GrantedWeaponAbilityHandles;
};
