// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/Widget/LobbyWidget.h"

#include "Components/Button.h"
#include "Components/WidgetSwitcher.h"
#include "Components/UniformGridPanel.h"
#include "Components/UniformGridSlot.h"
#include "UI/Widget/TeamSelectionWidget.h"
#include "Network/HPNetStatics.h"
#include "Controller/LobbyPlayerController.h"
#include "HPAssetManager.h"
#include "Characters/PDA_CharacterDefinition.h"
#include "Components/TileView.h"
#include "GameFramework/PlayerStart.h"
#include "GameMode/HPGameState.h"
#include "Kismet/GameplayStatics.h"
#include "PlayerState/HPPlayerState.h"
#include "UI/Widget/CharacterEntryWidget.h"
#include "UI/Widget/Character_Display.h"
#include "UI/Widget/PlayerTeamLayoutWidget.h"


void ULobbyWidget::NativeConstruct()
{
	Super::NativeConstruct();

	ClearAndPopulateTeamSelectionSlots();
	ConfigureGameState();
	LobbyPlayerController=GetOwningPlayer<ALobbyPlayerController>();

	if (LobbyPlayerController)
	{
		LobbyPlayerController -> OnSwitchToHeroSelectionDelegate.BindUObject(this, &ULobbyWidget::SwitchToHeroSelection);
	}
	
	StartHeroSelectionButton->SetIsEnabled(false);
	StartHeroSelectionButton->OnClicked.AddDynamic(this, &ULobbyWidget::StartHeroSelectionButtonClicked);

	StartMatchButton->SetIsEnabled(false);
	StartMatchButton->OnClicked.AddDynamic(this, &ULobbyWidget::StartMatchButtonClicked);
	UHPAssetManager::Get().LoadCharacterDefinitions(FStreamableDelegate::CreateUObject(this, &ULobbyWidget::CharacterDefinitionLoaded));

	if (CharacterSelectionTileView)
	{
		CharacterSelectionTileView->OnItemSelectionChanged().AddUObject(this,&ULobbyWidget::CharacterSelected);
	}
	SpawnCharacterDisplay();
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
	for (UTeamSelectionWidget* SelectionSlot : TeamSelectionSlots)
	{
		SelectionSlot -> UpdateSlotInfo("EmptySlot");
	}

	for (UUserWidget* CharacterEntryAsWidget : CharacterSelectionTileView->GetDisplayedEntryWidgets())
	{
		if (UCharacterEntryWidget* CharacterEntryWidget = Cast<UCharacterEntryWidget>(CharacterEntryAsWidget))
		{
			CharacterEntryWidget->SetSelected(false);
		}
	}
	
	for (const FPlayerSelection& PlayerSelection:PlayerSelections)
	{
		if (!PlayerSelection.IsValid())
			continue;
		TeamSelectionSlots[PlayerSelection.GetPlayerSlot()]->UpdateSlotInfo(PlayerSelection.GetPlayerNickName());

		UCharacterEntryWidget* SelectedEntry = CharacterSelectionTileView->GetEntryWidgetFromItem<UCharacterEntryWidget>(PlayerSelection.GetCharacterDefinition());

		bool bIsSameTeam = IsSameTeam(PlayerSelection.GetPlayerSlot());

		if (SelectedEntry && bIsSameTeam)
		{
			SelectedEntry->SetSelected(true);
		}
		if (PlayerSelection.IsForPlayer(GetOwningPlayerState()))
		{
			UpdateCharacterDisplay(PlayerSelection);
		}
	}

	if (HPGameState)
	{
		StartHeroSelectionButton->SetIsEnabled(HPGameState->CanStartHeroSelection());
		StartMatchButton->SetIsEnabled(HPGameState->CanStartMatch());
	}

	if (PlayerTeamLayoutWidget)
	{
		PlayerTeamLayoutWidget->UpdatePlayerSelection(PlayerSelections);
	}
}

void ULobbyWidget::StartHeroSelectionButtonClicked()
{
	if (LobbyPlayerController)
	{
		LobbyPlayerController->Server_StartHeroSelection();
	}
}

void ULobbyWidget::SwitchToHeroSelection()
{
	MainSwitcher->SetActiveWidget(HeroSelectionRoot);
}

void ULobbyWidget::CharacterDefinitionLoaded()
{
	TArray<UPDA_CharacterDefinition*> LoadedCharacterDefinitions;

	if (UHPAssetManager::Get().GetLoadedCharacterDefinitions(LoadedCharacterDefinitions))
	{
		CharacterSelectionTileView->SetListItems(LoadedCharacterDefinitions);
	}
}

void ULobbyWidget::CharacterSelected(UObject* SelectedUObject)
{
	if (!HPPlayerState)
	{
		HPPlayerState = GetOwningPlayerState<AHPPlayerState>();
	}

	if (!HPPlayerState)
	{
		return;
	}

	if (const UPDA_CharacterDefinition* CharacterDefinition = Cast<UPDA_CharacterDefinition>(SelectedUObject))
	{
		HPPlayerState->Server_SetSelectedCharacterDefinition(CharacterDefinition);
	}
}

void ULobbyWidget::SpawnCharacterDisplay()
{
	if (CharacterDisplay)
		return;
	
	if (!CharacterDisplayClass)
		return;
	
	FTransform CharacterDisplayTransform = FTransform::Identity;

	AActor* PlayerStart = UGameplayStatics::GetActorOfClass(GetWorld(), APlayerStart::StaticClass());

	if (PlayerStart)
	{
		CharacterDisplayTransform = PlayerStart->GetActorTransform();
	}

	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	
	CharacterDisplay = GetWorld()->SpawnActor<ACharacter_Display>(CharacterDisplayClass, CharacterDisplayTransform, SpawnParams);
	GetOwningPlayer()->SetViewTarget(CharacterDisplay);
}

void ULobbyWidget::UpdateCharacterDisplay(const FPlayerSelection& PlayerSelection)
{
	if (!PlayerSelection.GetCharacterDefinition())
		return;

	CharacterDisplay->ConfigureWithCharacterDefinition(PlayerSelection.GetCharacterDefinition());
}

void ULobbyWidget::StartMatchButtonClicked()
{
	if (LobbyPlayerController)
	{
		LobbyPlayerController->Server_RequestStartMatch();
	}
}

bool ULobbyWidget::IsSameTeam(uint8 SelectedTeamSlot)
{
	bool bIsMyTeam = true;
	if (LobbyPlayerController)
	{
		uint8 MySelectionSlot = LobbyPlayerController->GetSelectedSlotId();
		if ((SelectedTeamSlot >= 0 && SelectedTeamSlot < UHPNetStatics::GetPlayerCountPerTeam())&&
			(MySelectionSlot >= UHPNetStatics::GetPlayerCountPerTeam()))
		{
			bIsMyTeam = false;
		}
		else if ((SelectedTeamSlot >= UHPNetStatics::GetPlayerCountPerTeam())
			&&	(MySelectionSlot >= 0 && MySelectionSlot < UHPNetStatics::GetPlayerCountPerTeam()))
		{
			bIsMyTeam = false;
		}
	}
	return bIsMyTeam;
}
