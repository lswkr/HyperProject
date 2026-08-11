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
class ALobbyPlayerController;
class AHPGameState;
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

	UPROPERTY()
	ALobbyPlayerController* LobbyPlayerController;

	void ConfigureGameState();
	FTimerHandle ConfigureGameStateTimerHandle;
	
	UPROPERTY()
	AHPGameState* HPGameState;

	void  UpdatePlayerSelectionDisplay(const TArray<FPlayerSelection>& PlayerSelections);
};
