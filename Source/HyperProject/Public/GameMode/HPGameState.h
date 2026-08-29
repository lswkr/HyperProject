// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "PlayerInfoTypes.h"
#include "GameFramework/GameStateBase.h"
#include "HPGameState.generated.h"

class UPDA_CharacterDefinition;
class ALobbyPlayerController;

DECLARE_MULTICAST_DELEGATE_OneParam(FOnPlayerSelectionUpdatedDelegate, const TArray<FPlayerSelection>& /*NewPlayerSelection*/)
/**
 * 
 */
UCLASS()
class HYPERPROJECT_API AHPGameState : public AGameStateBase
{
	GENERATED_BODY()

public:
	void RequestPlayerSelectionChange(const APlayerState* RequestingPlayer, ALobbyPlayerController* LobbyPlayerController, uint8 DesiredSlot);
	void SetCharacterSelected(const APlayerState* SelectingPlayer, const UPDA_CharacterDefinition* SelectedDefinition, uint8 MySelectionSlot);
	bool IsSlotOccupied(uint8 SlotId) const;
	bool IsDefinitionSelected(const UPDA_CharacterDefinition* CharacterDefinition, uint8 MySelectionSlot) const;
	void SetCharacterDeselected(const UPDA_CharacterDefinition* CharacterDefinitionToDeselect, uint8 MySelectionSlot);
	
	FOnPlayerSelectionUpdatedDelegate OnPlayerSelectionUpdatedDelegate;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	const TArray<FPlayerSelection>& GetPlayerSelection() const;

	bool CanStartHeroSelection() const;
	bool CanStartMatch() const;
	
private:
	UPROPERTY(ReplicatedUsing  = OnRep_PlayerSelectionArray)
	TArray<FPlayerSelection> PlayerSelectionArray;

	UFUNCTION()
	void  OnRep_PlayerSelectionArray();
	
};
