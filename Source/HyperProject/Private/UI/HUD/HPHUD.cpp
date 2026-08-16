// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/HUD/HPHUD.h"

#include "UI/Widget/HPInGameOverlayWidget.h"
#include "UI/Widget/HPUserWidget.h"
#include "UI/WidgetController/OverlayWidgetController.h"
#include "UI/Widget/HitFeedbackWidget.h"

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

void AHPHUD::PlayHitFeedback(bool bIsHeadShot)
{
	if (!HitFeedbackWidgetClass)
	{
		UE_LOG(LogTemp, Error, TEXT("HitFeedbackWidgetClass isn't set"));
		return;
	}
	UE_LOG(LogTemp, Warning, TEXT("HitFeedbackWidgetClass set"));
	if (!HitFeedbackWidget)
	{
		HitFeedbackWidget = CreateWidget<UHitFeedbackWidget>(GetWorld(), HitFeedbackWidgetClass);
		HitFeedbackWidget->AddToViewport();
		HitFeedbackWidget->SetAlignmentInViewport(FVector2D(0.5f,0.5f));

		
		if (GetOwningPlayerController())
		{
			int32 ViewportWidth = 0;
			int32 ViewportHeight = 0;
			
			GetOwningPlayerController()->GetViewportSize(ViewportWidth, ViewportHeight);
			HitFeedbackWidget->SetPositionInViewport(FVector2D(ViewportWidth/2,ViewportHeight/2));
		}
	}

	UE_LOG(LogTemp, Warning, TEXT("Play HitFeedback"));
	bIsHeadShot ?
	HitFeedbackWidget->PlayHeadShotAnimation():
	HitFeedbackWidget->PlayBodyShotAnimation();

}

