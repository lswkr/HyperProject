// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "PlayerInfoTypes.h"
#include "GameFramework/PlayerState.h"
#include "GenericTeamAgentInterface.h"

#include "HPPlayerState.generated.h"

/**
 * 
 */
class AHPGameState;
class UPDA_CharacterDefinition;

UCLASS()
class HYPERPROJECT_API AHPPlayerState : public APlayerState
{
	GENERATED_BODY()

public:
	AHPPlayerState();
	
	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void BeginPlay() override;
	virtual void CopyProperties(APlayerState* PlayerState) override;
	
	TSubclassOf<APawn> GetSelectedCharacterClass() const;
	FGenericTeamId GetTeamIdBasedOnSlot() const;
	
	UFUNCTION(Server, Reliable, WithValidation)
	void Server_SetSelectedCharacterDefinition(const UPDA_CharacterDefinition* NewCharacterDefinition);
	
private:
	UPROPERTY(Replicated)
	FPlayerSelection PlayerSelection;
	
	UPROPERTY()
	AHPGameState* HPGameState;

	void PlayerSelectionUpdated(const TArray<FPlayerSelection>& NewPlayerSelections);
};
