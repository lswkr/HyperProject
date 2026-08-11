// Fill out your copyright notice in the Description page of Project Settings.


#include "PlayerState/HPPlayerState.h"

#include "Characters/PDA_CharacterDefinition.h"
#include "GameMode/HPGameState.h"
#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"
#include "Characters/Player/HPPlayerCharacter.h"
#include "Network/HPNetStatics.h"

AHPPlayerState::AHPPlayerState()
{
	bReplicates = true;
	NetUpdateFrequency = 100.f;
}

void AHPPlayerState::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AHPPlayerState, PlayerSelection);
}

void AHPPlayerState::BeginPlay()
{
	Super::BeginPlay();

	HPGameState = Cast<AHPGameState>(UGameplayStatics::GetGameState(this));

	if (HPGameState)
	{
		HPGameState->OnPlayerSelectionUpdatedDelegate.AddUObject(this, &AHPPlayerState::PlayerSelectionUpdated);
	}
}

void AHPPlayerState::CopyProperties(APlayerState* PlayerState)
{
	Super::CopyProperties(PlayerState);

	AHPPlayerState* NewPlayerState = Cast<AHPPlayerState>(PlayerState);
	if (NewPlayerState)
	{
		NewPlayerState->PlayerSelection = PlayerSelection;
	}

}

TSubclassOf<APawn> AHPPlayerState::GetSelectedCharacterClass() const
{
	if (PlayerSelection.GetCharacterDefinition())
	{
		return PlayerSelection.GetCharacterDefinition()->LoadCharacterClass();
	}
	return nullptr;
}

FGenericTeamId AHPPlayerState::GetTeamIdBasedOnSlot() const
{
	return PlayerSelection.GetPlayerSlot() < UHPNetStatics::GetPlayerCountPerTeam() ? FGenericTeamId{0} : FGenericTeamId{1};
}

void AHPPlayerState::Server_SetSelectedCharacterDefinition_Implementation(
	const UPDA_CharacterDefinition* NewCharacterDefinition)
{
	if (!HPGameState)
		return;

	if (!NewCharacterDefinition)
		return;
	
	if (HPGameState->IsDefinitionSelected(NewCharacterDefinition))
		return;

	if (PlayerSelection.GetCharacterDefinition() != nullptr)
	{
		HPGameState->SetCharacterDeselected(PlayerSelection.GetCharacterDefinition());
	}

	PlayerSelection.SetCharacterDefinition(NewCharacterDefinition);
	HPGameState->SetCharacterSelected(this, NewCharacterDefinition);
}

bool AHPPlayerState::Server_SetSelectedCharacterDefinition_Validate(
	const UPDA_CharacterDefinition* NewCharacterDefinition)
{
	return true;
}

void AHPPlayerState::PlayerSelectionUpdated(const TArray<FPlayerSelection>& NewPlayerSelections)
{
	for (const FPlayerSelection& NewPlayerSelection : NewPlayerSelections)
	{
		if (NewPlayerSelection.IsForPlayer(this))
		{
			PlayerSelection = NewPlayerSelection;
		}
	}
}
