// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UI/WidgetController/HPWidgetController.h"
#include "OverlayWidgetController.generated.h"

/**
 * 
 */
class UAbilityInfo;


DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FAbilityInfoDelegate, const FHPAbilityInfo&, Info);
DECLARE_DYNAMIC_MULTICAST_DELEGATE (FUltEndDelegate);

UCLASS(BlueprintType, Blueprintable)
class HYPERPROJECT_API UOverlayWidgetController : public UHPWidgetController
{
	GENERATED_BODY()

public:
	virtual void BindCallbacksToDependencies() override;
	virtual void BroadcastInitialValues() override;

	UPROPERTY(BlueprintAssignable, Category = "GAS|Attributes")
	FOnAttributeChangedDelegate OnHealthChangedDelegate;

	UPROPERTY(BlueprintAssignable, Category = "GAS|Attributes")
	FOnAttributeChangedDelegate OnMaxHealthChangedDelegate;

	UPROPERTY(BlueprintAssignable, Category = "GAS|Attributes")
	FOnAttributeChangedDelegate OnUltChangedDelegate;

	UPROPERTY(BlueprintAssignable, Category = "GAS|Attributes")
	FOnAttributeChangedDelegate OnMaxUltChangedDelegate;

	UPROPERTY(BlueprintAssignable, Category = "GAS|Attributes")
	FOnAttributeChangedDelegate OnMaxBulletChangedDelegate;

	UPROPERTY(BlueprintAssignable, Category = "GAS|Attributes")
	FOnAttributeChangedDelegate OnBulletChangedDelegate;

	UPROPERTY(BlueprintAssignable, Category = "GAS|Attributes")
	FOnAttributeChangedDelegate OnDamageDoneDelegate;

	UPROPERTY(BlueprintAssignable, Category = "GAS|AbilityInfo")
	FAbilityInfoDelegate AbilityInfoDelegate;

	UPROPERTY(BlueprintAssignable, Category = "GAS|AbilityInfo")
	FAbilityInfoDelegate UltInfoDelegate;

	UPROPERTY(BlueprintAssignable, Category = "GAS|AbilityInfo")
	FUltEndDelegate UltEndDelegate;

	void BroadcastAbilityInfo();

private:
	UPROPERTY(EditDefaultsOnly,Category = "GAS|AbilityInfo")
	UAbilityInfo* AbilityInfo;
	
};
