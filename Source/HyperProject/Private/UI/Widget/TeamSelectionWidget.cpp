// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/Widget/TeamSelectionWidget.h"
#include "Components/TextBlock.h"
#include "Components/Button.h"

void UTeamSelectionWidget::SetSlotID(uint8 NewSlotID)
{
	SlotID = NewSlotID;
}

void UTeamSelectionWidget::UpdateSlotInfo(const FString& PlayerNickName)
{
	Text_Info->SetText(FText::FromString(PlayerNickName));	
}

void UTeamSelectionWidget::NativeConstruct()
{
	Super::NativeConstruct();

	Button_Select->OnClicked.AddDynamic(this, &UTeamSelectionWidget::SelectButtonClicked);
}

void UTeamSelectionWidget::SelectButtonClicked()
{
	OnSlotClicked.Broadcast(SlotID);
}
