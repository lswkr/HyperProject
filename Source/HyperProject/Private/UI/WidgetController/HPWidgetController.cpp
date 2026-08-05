// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/WidgetController/HPWidgetController.h"

#include "AbilitySystem/HPAbilitySystemComponent.h"
#include "AbilitySystem/HPAttributeSet.h"
#include "Controller/HPPlayerController.h"

void UHPWidgetController::SetWidgetControllerParams(const FWidgetControllerParams& WidgetControllerParams)
{
	PlayerController = WidgetControllerParams.PlayerController;
	AbilitySystemComponent = WidgetControllerParams.AbilitySystemComponent;
	AttributeSet = WidgetControllerParams.AttributeSet;
	HPCombatComponent = WidgetControllerParams.CombatComponent;
	//DamageContributionComponent = WidgetControllerParams.DamageContributionComponent;
}

void UHPWidgetController::BindCallbacksToDependencies(){}

void UHPWidgetController::BroadcastInitialValues(){}

AHPPlayerController* UHPWidgetController::GetHPPlayerController()
{
	if (HPPlayerController==nullptr)
	{
		HPPlayerController = Cast<AHPPlayerController>(PlayerController);
	}
	return HPPlayerController;
}

UHPAbilitySystemComponent* UHPWidgetController::GetHPAbilitySystemComponent()
{
	if (HPAbilitySystemComponent==nullptr)
	{
		HPAbilitySystemComponent = Cast<UHPAbilitySystemComponent>(AbilitySystemComponent);
	}
	return HPAbilitySystemComponent;
	
}

UHPAttributeSet* UHPWidgetController::GetHPAttributeSet()
{
	if (HPAttributeSet==nullptr)
	{
		HPAttributeSet = Cast<UHPAttributeSet>(AttributeSet);
	}
	return HPAttributeSet;
}

UHPCombatComponent* UHPWidgetController::GetHPCombatComponent() const
{
	return HPCombatComponent;
}

// UDamageContributionComponent* UHPWidgetController::GetDamageContributionComponent() const
// {
// 	return DamageContributionComponent;
// }