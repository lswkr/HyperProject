// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/Widget/WaitingWidget.h"
#include "Components/TextBlock.h"

void UWaitingWidget::NativeConstruct()
{
	Super::NativeConstruct();

	
}

FOnButtonClickedEvent& UWaitingWidget::ClearAndGetButtonClickedEvent()
{
	Button_Cancel->OnClicked.Clear();

	return Button_Cancel->OnClicked;
}

void UWaitingWidget::SetWaitInfo(const FText& WaitInfo, bool bAllowCancel)
{
	if (Button_Cancel)
	{
		Button_Cancel->SetVisibility(bAllowCancel? ESlateVisibility::Visible:ESlateVisibility::Hidden);
	}

	if (TextBlock_WaitInfoText)
	{
		TextBlock_WaitInfoText->SetText(WaitInfo);
	}
}
