// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/Button.h"
#include "WaitingWidget.generated.h"

/**
 * 
 */
class UTextBlock;

UCLASS()
class HYPERPROJECT_API UWaitingWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:
	virtual void NativeConstruct() override;

	FOnButtonClickedEvent& ClearAndGetButtonClickedEvent();
	void SetWaitInfo(const FText& WaitInfo, bool bAllowCancel = false);

private:
	UPROPERTY(meta = (BindWidget))
	UTextBlock* TextBlock_WaitInfoText;
	UPROPERTY(meta = (BindWidget))
	UButton* Button_Cancel;
};
