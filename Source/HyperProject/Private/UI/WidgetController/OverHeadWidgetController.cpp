// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/WidgetController/OverHeadWidgetController.h"

#include "AbilitySystemComponent.h"
#include "HPGameplayTags.h"

// void UOverHeadWidgetController::BindCallbacksToDependencies()
// {
// 	FHPGameplayTags GameplayTags = FHPGameplayTags::Get();
// 	
// 	AbilitySystemComponent->RegisterGameplayTagEvent(GameplayTags.State_Debuff_HealBan, EGameplayTagEventType::NewOrRemoved).
// 	AddUObject(this, &UOverHeadWidgetController::OnHealBanTagChanged);
// }
//
// void UOverHeadWidgetController::BroadcastInitialValues()
// {
// 	OnHealBanTagChangedDelegate.Broadcast(false);
// }
//
//
