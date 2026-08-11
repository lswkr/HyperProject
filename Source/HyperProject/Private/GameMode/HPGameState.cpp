// Fill out your copyright notice in the Description page of Project Settings.


#include "GameMode/HPGameState.h"

#include "Net/UnrealNetwork.h"

void AHPGameState::RequestPlayerSelectionChange(const APlayerState* RequestingPlayer, uint8 DesiredSlot)
{
	
	if (!HasAuthority() || IsSlotOccupied(DesiredSlot))
	{
		return;
	}

	FPlayerSelection* PlayerSelectionPtr = PlayerSelectionArray.FindByPredicate([&] (const FPlayerSelection& PlayerSelection)
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

	OnPlayerSelectionUpdatedDelegate.Broadcast(PlayerSelectionArray);
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

const TArray<FPlayerSelection>& AHPGameState::GetPlayerSelection() const
{
	return PlayerSelectionArray;
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
