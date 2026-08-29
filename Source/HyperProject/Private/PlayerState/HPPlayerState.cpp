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

	UE_LOG(LogTemp, Error,
		TEXT("CopyProperties CALLED - Selection=%s"),
		*PlayerSelection.GetCharacterDefinition()->GetName());
	
	AHPPlayerState* NewPlayerState = Cast<AHPPlayerState>(PlayerState);
	if (NewPlayerState)
	{
		NewPlayerState->PlayerSelection = PlayerSelection;
		UE_LOG(LogTemp, Error,
		TEXT("NewPlayerState CopyProperties CALLED - Selection=%s"),
		*PlayerSelection.GetCharacterDefinition()->GetName());
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
	if (!NewCharacterDefinition)
		return;
	
	if (!HPGameState)
		return;

	if (HPGameState->IsDefinitionSelected(NewCharacterDefinition, PlayerSelection.GetPlayerSlot())) //우리팀이 선택 안 했을 경우 진행되도록
		return;

	if (PlayerSelection.GetCharacterDefinition() != nullptr) //이미 뭔가 골랐다면
	{
		HPGameState->SetCharacterDeselected(PlayerSelection.GetCharacterDefinition(), PlayerSelection.GetPlayerSlot()); //비우기
	}

	PlayerSelection.SetCharacterDefinition(NewCharacterDefinition); //새로 선택
	HPGameState->SetCharacterSelected(this, NewCharacterDefinition,  PlayerSelection.GetPlayerSlot()); 
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
			return;
		}
	}
}
