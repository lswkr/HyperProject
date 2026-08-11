// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/Widget/LobbyWidget.h"
#include "Components/UniformGridPanel.h"
#include "Components/UniformGridSlot.h"
#include "UI/Widget/TeamSelectionWidget.h"
#include "Network/HPNetStatics.h"
#include "Controller/LobbyPlayerController.h"
#include "GameMode/HPGameState.h"

void ULobbyWidget::NativeConstruct()
{
	Super::NativeConstruct();

	ClearAndPopulateTeamSelectionSlots();

	LobbyPlayerController=GetOwningPlayer<ALobbyPlayerController>();
	ConfigureGameState();
}

void ULobbyWidget::ClearAndPopulateTeamSelectionSlots()
{
	TeamSelectionSlotGridPanel->ClearChildren();

	for (int i=0;i<UHPNetStatics::GetPlayerCountPerTeam()*2;i++)
	{
		UTeamSelectionWidget* NewSelectionSlot = CreateWidget<UTeamSelectionWidget>(this, TeamSelectionWidgetClass);
		if(NewSelectionSlot)
		{
			NewSelectionSlot->SetSlotID(i);
			UUniformGridSlot* NewGridSlot = TeamSelectionSlotGridPanel->AddChildToUniformGrid(NewSelectionSlot);

			if (NewGridSlot)
			{
				int Row = i % UHPNetStatics::GetPlayerCountPerTeam();
				int Column = i < UHPNetStatics::GetPlayerCountPerTeam() ? 0 : 1;

				NewGridSlot->SetRow(Row);
				NewGridSlot->SetColumn(Column);
			}

			NewSelectionSlot -> OnSlotClicked.AddUObject(this, &ULobbyWidget::SlotSelected);
			TeamSelectionSlots.Add(NewSelectionSlot);
		}
	}
}

void ULobbyWidget::SlotSelected(uint8 NewSlotID)
{
	if (LobbyPlayerController)
	{
		LobbyPlayerController->Server_RequestSlotSelectionChange(NewSlotID);
	}
}

void ULobbyWidget::ConfigureGameState()
{
	UWorld* World = GetWorld();
	if (!World)
		return;

	HPGameState = World->GetGameState<AHPGameState>();

	if (!HPGameState) // 아직 Set되지 않았을 경우
	{
		World->GetTimerManager().SetTimer(
			ConfigureGameStateTimerHandle,
			this,
			&ULobbyWidget::ConfigureGameState,
			1.f
			);
	}
	else
	{
		HPGameState -> OnPlayerSelectionUpdatedDelegate.AddUObject(this, &ULobbyWidget::UpdatePlayerSelectionDisplay);
		UpdatePlayerSelectionDisplay(HPGameState->GetPlayerSelection());
	}
}

void ULobbyWidget::UpdatePlayerSelectionDisplay(const TArray<FPlayerSelection>& PlayerSelections)
{
	for (UTeamSelectionWidget* SelectionSlot:TeamSelectionSlots)
	{
		SelectionSlot -> UpdateSlotInfo("Empty");
	}

	for (const FPlayerSelection& PlayerSelection:PlayerSelections)
	{
		if (!PlayerSelection.IsValid())
			continue;
		
		TeamSelectionSlots[PlayerSelection.GetPlayerSlot()]->UpdateSlotInfo(PlayerSelection.GetPlayerNickName());
	}
}


