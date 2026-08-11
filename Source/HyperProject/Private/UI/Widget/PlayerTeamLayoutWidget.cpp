// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/Widget/PlayerTeamLayoutWidget.h"

#include "Components/HorizontalBox.h"
#include "Components/HorizontalBoxSlot.h"
#include "Network/HPNetStatics.h"
#include "UI/Widget/PlayerTeamSlotWidget.h"

void UPlayerTeamLayoutWidget::NativeConstruct()
{
	Super::NativeConstruct();

	HorizontalBox_TeamOneLayoutBox->ClearChildren();
	HorizontalBox_TeamTwoLayoutBox->ClearChildren();

	if (!PlayerTeamSlotWidgetClass)
	{
		return;
	}

	for (int i=0;i<UHPNetStatics::GetPlayerCountPerTeam()*2;i++)
	{
		UPlayerTeamSlotWidget* NewSlotWidget = CreateWidget<UPlayerTeamSlotWidget>(GetOwningPlayer(), PlayerTeamSlotWidgetClass);
		TeamSlotWidgets.Add(NewSlotWidget);

		UHorizontalBoxSlot* NewSlot;
		if (i<UHPNetStatics::GetPlayerCountPerTeam())
		{
			NewSlot = HorizontalBox_TeamOneLayoutBox->AddChildToHorizontalBox(NewSlotWidget);
		}
		else
		{
			NewSlot = HorizontalBox_TeamTwoLayoutBox->AddChildToHorizontalBox(NewSlotWidget);
		}

		NewSlot->SetPadding(FMargin{PlayerTeamWidgetSlotMargin});
	}
}

void UPlayerTeamLayoutWidget::UpdatePlayerSelection(const TArray<FPlayerSelection>& PlayerSelections)
{
	for (UPlayerTeamSlotWidget* SlotWidget: TeamSlotWidgets)
	{
		SlotWidget->UpdateSlot("", nullptr);
	}

	for (const FPlayerSelection& PlayerSelection:PlayerSelections)
	{
		if (!PlayerSelection.IsValid())
			continue;
		TeamSlotWidgets[PlayerSelection.GetPlayerSlot()]->UpdateSlot(PlayerSelection.GetPlayerNickName(), PlayerSelection.GetCharacterDefinition());
	}
}
