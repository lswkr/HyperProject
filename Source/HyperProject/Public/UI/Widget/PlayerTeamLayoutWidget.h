// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "PlayerInfoTypes.h"
#include "Blueprint/UserWidget.h"
#include "PlayerTeamLayoutWidget.generated.h"

/**
 * 
 */
class UPlayerTeamSlotWidget;
class UHorizontalBox;

UCLASS()
class HYPERPROJECT_API UPlayerTeamLayoutWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:
	virtual void NativeConstruct() override;
	void UpdatePlayerSelection(const TArray<FPlayerSelection>& PlayerSelections);
	
private:
	UPROPERTY(EditDefaultsOnly, Category = "Visual")
	TSubclassOf<UPlayerTeamSlotWidget> PlayerTeamSlotWidgetClass;

	UPROPERTY(EditDefaultsOnly, Category = "Visual")
	float PlayerTeamWidgetSlotMargin = 5.f;
	
	UPROPERTY(meta=(BindWidget))
	UHorizontalBox* HorizontalBox_TeamOneLayoutBox;

	UPROPERTY(meta=(BindWidget))
	UHorizontalBox* HorizontalBox_TeamTwoLayoutBox;
	
	UPROPERTY()
	TArray<UPlayerTeamSlotWidget*> TeamSlotWidgets;
};
