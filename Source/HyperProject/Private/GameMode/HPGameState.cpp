// Fill out your copyright notice in the Description page of Project Settings.


#include "GameMode/HPGameState.h"

#include "Controller/LobbyPlayerController.h"
#include "Net/UnrealNetwork.h"
#include "Network/HPNetStatics.h"

void AHPGameState::RequestPlayerSelectionChange(const APlayerState* RequestingPlayer, ALobbyPlayerController* LobbyPlayerController, uint8 DesiredSlot)
{
	if (!HasAuthority() || IsSlotOccupied(DesiredSlot))
		return;

	FPlayerSelection* PlayerSelectionPtr = PlayerSelectionArray.FindByPredicate([&](const FPlayerSelection& PlayerSelection)
		{
			return PlayerSelection.IsForPlayer(RequestingPlayer);
		}
	);
	
	if (PlayerSelectionPtr)
	{
		PlayerSelectionPtr->SetSlot(DesiredSlot);
	}
	else
	{
		PlayerSelectionArray.Add(FPlayerSelection(DesiredSlot, RequestingPlayer));
	}

	LobbyPlayerController->SetSelectedSlotId(DesiredSlot);
	OnPlayerSelectionUpdatedDelegate.Broadcast(PlayerSelectionArray);
}

void AHPGameState::SetCharacterSelected(const APlayerState* SelectingPlayer,
	const UPDA_CharacterDefinition* SelectedDefinition, uint8 MySelectionSlot)
{
	if (IsDefinitionSelected(SelectedDefinition, MySelectionSlot))
		return;
	FPlayerSelection* FoundPlayerSelection = PlayerSelectionArray.FindByPredicate(
		[&](const FPlayerSelection& PlayerSelection)
		{
			return PlayerSelection.IsForPlayer(SelectingPlayer);
		}
		);
	if (FoundPlayerSelection)
	{
		FoundPlayerSelection->SetCharacterDefinition(SelectedDefinition);
		OnPlayerSelectionUpdatedDelegate.Broadcast(PlayerSelectionArray);
	}
}

const TArray<FPlayerSelection>& AHPGameState::GetPlayerSelection() const
{
	UE_LOG(LogTemp, Warning, TEXT("AHPGameState- PlayerSelectionArraySize: %d"), PlayerSelectionArray.Num());
	return PlayerSelectionArray;
}

bool AHPGameState::CanStartHeroSelection() const
{
	return PlayerSelectionArray.Num() == PlayerArray.Num();
}

bool AHPGameState::CanStartMatch() const
{
	for (const FPlayerSelection& PlayerSelection : PlayerSelectionArray)
	{
		if (PlayerSelection.GetCharacterDefinition() == nullptr)
		{
			return false;
		}
	}
	return true;
}

bool AHPGameState::IsSlotOccupied(uint8 SlotId) const
{
	for (const FPlayerSelection& PlayerSelection : PlayerSelectionArray)
	{
		if (PlayerSelection.GetPlayerSlot() == SlotId)
		{
			return true;
		}
	}
	return false;
}

bool AHPGameState::IsDefinitionSelected(const UPDA_CharacterDefinition* CharacterDefinition, uint8 MySelectionSlot) const
{
	const FPlayerSelection* FoundPlayerSelection = PlayerSelectionArray.FindByPredicate(
		[&] (const FPlayerSelection& PlayerSelection)
		{
			return PlayerSelection.GetCharacterDefinition() == CharacterDefinition && PlayerSelection.GetPlayerSlot()/UHPNetStatics::GetPlayerCountPerTeam() == MySelectionSlot/UHPNetStatics::GetPlayerCountPerTeam();	
		}
		);

	return FoundPlayerSelection != nullptr;
}

void AHPGameState::SetCharacterDeselected(const UPDA_CharacterDefinition* CharacterDefinitionToDeselect, uint8 MySelectionSlot)
{
	FPlayerSelection* FoundPlayerSelection = PlayerSelectionArray.FindByPredicate(
		[&](const FPlayerSelection& PlayerSelection)
		{
			return PlayerSelection.GetCharacterDefinition() == CharacterDefinitionToDeselect && PlayerSelection.GetPlayerSlot()/UHPNetStatics::GetPlayerCountPerTeam() == MySelectionSlot/UHPNetStatics::GetPlayerCountPerTeam();	
		}
		);
	
	if (FoundPlayerSelection)
	{
		FoundPlayerSelection->SetCharacterDefinition(nullptr);
		OnPlayerSelectionUpdatedDelegate.Broadcast(PlayerSelectionArray);
	}
}

void AHPGameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION_NOTIFY(AHPGameState, PlayerSelectionArray,COND_None,REPNOTIFY_Always);
}

void AHPGameState::OnRep_PlayerSelectionArray()
{
	OnPlayerSelectionUpdatedDelegate.Broadcast(PlayerSelectionArray);
}
