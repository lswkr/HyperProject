// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/Widget/SessionEntryWidget.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"

void USessionEntryWidget::NativeConstruct()
{
	Super::NativeConstruct();

	Button_Session->OnClicked.AddDynamic(this, &USessionEntryWidget::SessionEntrySelected);

}

void USessionEntryWidget::InitializeEntry(const FString& Name, const FString& SessionIdStr)
{
	TextBlock_SessionName->SetText(FText::FromString(Name));
	CachedSessionIdStr = SessionIdStr;
}

void USessionEntryWidget::SessionEntrySelected()
{
	OnSessionEntrySelected.Broadcast(CachedSessionIdStr);
}
