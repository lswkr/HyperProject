// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "TeamSelectionWidget.generated.h"

/**
 * 
 */
class UButton;
class UTextBlock;

DECLARE_MULTICAST_DELEGATE_OneParam(FOnSlotClicked, uint8 /*SlotID*/);

UCLASS()
class HYPERPROJECT_API UTeamSelectionWidget : public UUserWidget
{
	GENERATED_BODY()
public:
	void SetSlotID(uint8 NewSlotID);
	void UpdateSlotInfo(const FString& PlayerNickName);

	virtual void NativeConstruct() override;
	
	FOnSlotClicked OnSlotClicked;
private:
	UPROPERTY(meta = (BindWidget))
	UButton* Button_Select;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* Text_Info;

	UFUNCTION()
	void SelectButtonClicked();

	uint8 SlotID;
	
};
