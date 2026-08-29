// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/Button.h"
#include "MainMenuTestWidget.generated.h"

/**
 * 
 */
class UWidgetSwitcher;
class UHPGameInstance;
class UButton;
class UWaitingWidget;
class UEditableText;
class UScrollBox;
class USessionEntryWidget;

UCLASS()
class HYPERPROJECT_API UMainMenuTestWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:
	virtual void NativeConstruct() override;

	/* Main */
private:
	UPROPERTY(meta = (BindWidget))
	UWidgetSwitcher* WidgetSwitcher_Main;

	UPROPERTY()
	UHPGameInstance* HPGameInstance;

	void SwitchToMainWidget();

	/* Login */
private:
	UPROPERTY(meta = (BindWidget))
	UWidget* LoginWidgetRoot;

	UPROPERTY(meta = (BindWidget))
	UButton* Button_Login;

	UFUNCTION()
	void LoginButtonClicked();

	void LoginCompleted(bool bWasSuccessful, const FString& PlayerNickname, const FString& ErrorMsg);

	/* Waiting */
private:
	UPROPERTY(meta = (BindWidget))
	UWaitingWidget* WaitingWidget;

	FOnButtonClickedEvent& SwitchToWaitingWidget(const FText& WaitInfo, bool bAllowCancel = false);
	
};
