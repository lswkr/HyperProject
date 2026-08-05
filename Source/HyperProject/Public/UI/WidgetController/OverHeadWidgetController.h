// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UI/WidgetController/HPWidgetController.h"
#include "OverHeadWidgetController.generated.h"

/**
 * 
 */

struct FGameplayTag;
UCLASS(Blueprintable)
class HYPERPROJECT_API UOverHeadWidgetController : public UHPWidgetController
{
	GENERATED_BODY()

public:
	// virtual void BindCallbacksToDependencies() override;
	// virtual void BroadcastInitialValues() override;
	//
	// UPROPERTY(BlueprintAssignable, Category = "GAS|Attributes")
	// FOnAttributeChangedDelegate OnHealthChangedDelegate;
	//
	// UPROPERTY(BlueprintAssignable, Category = "GAS|Attributes")
	// FOnAttributeChangedDelegate OnMaxHealthChangedDelegate;

};
