// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/WidgetController/OverlayWidgetController.h"

#include "HPGameplayTags.h"
#include "AbilitySystem/AbilityInfo.h"
#include "AbilitySystem/HPAbilitySystemComponent.h"
#include "AbilitySystem/HPAttributeSet.h"
#include "Components/HPCombatComponent.h"

void UOverlayWidgetController::BindCallbacksToDependencies()
{
	AbilitySystemComponent->RegisterGameplayTagEvent(FHPGameplayTags::Get().State_Ult_Full, EGameplayTagEventType::NewOrRemoved).
	AddLambda([this](const FGameplayTag CallbackTag, int32 NewCount)
	{
		if (NewCount > 0)
		{
			FGameplayTag UltTag = HPAbilitySystemComponent->GetUltTagForCurrentCharacter();
			FHPAbilityInfo Info = AbilityInfo->FindAbilityInfoByTag(UltTag);
			UltInfoDelegate.Broadcast(Info);
		}
		else if (NewCount == 0)
		{
			UltEndDelegate.Broadcast();
		}
	});

	AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(GetHPAttributeSet()->GetHealthAttribute()).
	AddLambda([this](const FOnAttributeChangeData& Data)
	{
		OnHealthChangedDelegate.Broadcast(Data.NewValue);
	}
	);
	
	AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(GetHPAttributeSet()->GetMaxHealthAttribute()).
	AddLambda([this](const FOnAttributeChangeData& Data)
	{
		OnMaxHealthChangedDelegate.Broadcast(Data.NewValue);
	}
	);

	AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(GetHPAttributeSet()->GetUltAttribute()).
	AddLambda([this](const FOnAttributeChangeData& Data)
	{
		OnUltChangedDelegate.Broadcast(Data.NewValue);
	}
	);
	
	AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(GetHPAttributeSet()->GetMaxUltAttribute()).
	AddLambda([this](const FOnAttributeChangeData& Data)
	{
		OnMaxUltChangedDelegate.Broadcast(Data.NewValue);
	}
	);
	
	AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(GetHPAttributeSet()->GetBulletAttribute()).
	AddLambda([this](const FOnAttributeChangeData& Data)
	{
		
		OnBulletChangedDelegate.Broadcast(Data.NewValue - GetHPCombatComponent()->GetClientPendingBullets(Data.NewValue));
		//GetHPCombatComponent()->SetServerBullets(Data.NewValue);
	}
	);
	
	AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(GetHPAttributeSet()->GetMaxBulletAttribute()).
	AddLambda([this](const FOnAttributeChangeData& Data)
	{
		OnMaxBulletChangedDelegate.Broadcast(Data.NewValue);
	}
	);
	
	//if (GetHPAbilitySystemComponent()->AbilitiesGiven)
	//{
	//	BroadcastAbilityInfo();
	//}
	//else
	//{
		GetHPAbilitySystemComponent()->AbilitiesGivenDelegate.AddUObject(this, &UOverlayWidgetController::BroadcastAbilityInfo);
	//}
}

void UOverlayWidgetController::BroadcastInitialValues()
{
	OnHealthChangedDelegate.Broadcast(GetHPAttributeSet()->GetHealth());
	OnMaxHealthChangedDelegate.Broadcast(GetHPAttributeSet()->GetMaxHealth());
	OnUltChangedDelegate.Broadcast(GetHPAttributeSet()->GetUlt());
	OnMaxUltChangedDelegate.Broadcast(GetHPAttributeSet()->GetMaxUlt());
	OnBulletChangedDelegate.Broadcast(GetHPAttributeSet()->GetBullet());
	OnMaxBulletChangedDelegate.Broadcast(GetHPAttributeSet()->GetMaxBullet());

	if (GetHPAbilitySystemComponent()->AbilitiesGiven)
	{
		BroadcastAbilityInfo();
	}
}

void UOverlayWidgetController::BroadcastAbilityInfo()
{
	if (!GetHPAbilitySystemComponent()->AbilitiesGiven) return;

	UE_LOG(LogTemp, Warning, TEXT("BroadcastAbilityInfo"));
	FForEachAbilityDelegate BroadcastAbilityInfoDelegate;
	BroadcastAbilityInfoDelegate.BindLambda([this](const FGameplayAbilitySpec& AbilitySpec)
	{

		FHPAbilityInfo Info = AbilityInfo->FindAbilityInfoByTag(GetHPAbilitySystemComponent()->GetAbilityTagFromSpec(AbilitySpec));

		AbilityInfoDelegate.Broadcast(Info);
	});
	GetHPAbilitySystemComponent()->ForEachAbility(BroadcastAbilityInfoDelegate);
}
