// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "PlayerInfoTypes.h"
#include "Blueprint/UserWidget.h"
#include "LobbyWidget.generated.h"

/**
 * 
 */
class UWidgetSwitcher;
class UWidget;
class UButton;
class UUniformGridPanel;
class UTeamSelectionWidget;
class UTileView;
class ALobbyPlayerController;
class AHPGameState;
class AHPPlayerState;
class ACharacter_Display;
class UPlayerTeamLayoutWidget;

UCLASS()
class HYPERPROJECT_API ULobbyWidget : public UUserWidget
{
	GENERATED_BODY()
public:
	virtual void NativeConstruct() override;
	
private:
	UPROPERTY(meta = (BindWidget))
	UWidgetSwitcher* MainSwitcher;

	UPROPERTY(meta = (BindWidget))
	UWidget* TeamSelectionRoot;

	UPROPERTY(meta = (BindWidget))
	UButton* StartHeroSelectionButton;

	UPROPERTY(meta = (BindWidget))
	UUniformGridPanel* TeamSelectionSlotGridPanel;

	UPROPERTY(EditDefaultsOnly, Category = "TeamSelection")
	TSubclassOf<UTeamSelectionWidget> TeamSelectionWidgetClass;

	UPROPERTY()
	TArray<UTeamSelectionWidget*> TeamSelectionSlots;

	void ClearAndPopulateTeamSelectionSlots();
	void SlotSelected(uint8 NewSlotID);

	UPROPERTY(meta = (BindWidget))
	UWidget* HeroSelectionRoot;

	UPROPERTY(meta = (BindWidget))
	UTileView* CharacterSelectionTileView;

	UPROPERTY(meta = (BindWidget))
	UPlayerTeamLayoutWidget* PlayerTeamLayoutWidget;

	UPROPERTY(meta=(BindWidget))
	UButton* StartMatchButton;
	
	UPROPERTY()
	ALobbyPlayerController* LobbyPlayerController;

	UPROPERTY()
	AHPPlayerState* HPPlayerState;
	
	void ConfigureGameState();
	FTimerHandle ConfigureGameStateTimerHandle;
	
	UPROPERTY()
	AHPGameState* HPGameState;

	void UpdatePlayerSelectionDisplay(const TArray<FPlayerSelection>& PlayerSelections);

	UFUNCTION()
	void StartHeroSelectionButtonClicked();

	void SwitchToHeroSelection();
	void CharacterDefinitionLoaded();
	
	void CharacterSelected(UObject* SelectedUObject);

	UPROPERTY(EditDefaultsOnly, Category = "Character Display")
	TSubclassOf<ACharacter_Display> CharacterDisplayClass;

	UPROPERTY()
	ACharacter_Display* CharacterDisplay;

	void SpawnCharacterDisplay();
	void UpdateCharacterDisplay(const FPlayerSelection& PlayerSelection);

	UFUNCTION()
	void StartMatchButtonClicked();
};


