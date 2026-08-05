// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/HUD/HPHUD.h"

#include "UI/Widget/HPInGameOverlayWidget.h"
#include "UI/Widget/HPUserWidget.h"
#include "UI/WidgetController/OverlayWidgetController.h"

void AHPHUD::InitOverlay(APlayerController* PC, UAbilitySystemComponent* ASC, UAttributeSet* AS, UHPCombatComponent* CC)
{
	checkf(OverlayWidgetClass, TEXT("Overlay Widget Class didn't set"))
	checkf(OverlayWidgetControllerClass, TEXT("Overlay Widget Controller Class didn't set"))

	UUserWidget* Widget = CreateWidget<UUserWidget>(GetWorld(), OverlayWidgetClass);
	OverlayWidget = Cast<UHPInGameOverlayWidget>(Widget);

	const FWidgetControllerParams WidgetControllerParams(PC, ASC, AS, CC);
	UOverlayWidgetController* WidgetController = GetOverlayWidgetController(WidgetControllerParams);

	OverlayWidget->SetWidgetController(WidgetController);
	WidgetController->BroadcastInitialValues();
	Widget->AddToViewport();
}


UOverlayWidgetController* AHPHUD::GetOverlayWidgetController(const FWidgetControllerParams& WidgetControllerParams)
{
	if (OverlayWidgetController == nullptr)
	{
		OverlayWidgetController = NewObject<UOverlayWidgetController>(this, OverlayWidgetControllerClass);
		OverlayWidgetController->SetWidgetControllerParams(WidgetControllerParams);
		OverlayWidgetController->BindCallbacksToDependencies();
	}
	return OverlayWidgetController;
}

void AHPHUD::GetOverlayWidgetFloatKillLog(FName VictimNickName, float ContributionValue)
{
	if (OverlayWidget)
	{
		OverlayWidget->FloatKillLog(VictimNickName, ContributionValue);
	}
}
