// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/Button.h"
#include "MainMenuWidget.generated.h"

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
class HYPERPROJECT_API UMainMenuWidget : public UUserWidget
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

	UPROPERTY(meta = (BindWidget))
	UWidget* MainWidgetRoot;

	/* Session */
	UPROPERTY(meta = (BindWidget))
	UButton* Button_CreateSession;

	UPROPERTY(meta = (BindWidget))
	UEditableText* EditableText_NewSessionName;

	UFUNCTION()
	void CancelSessionCreation();
	UFUNCTION()
	void CreateSessionButtonClicked();

	UFUNCTION()
	void NewSessionNameTextChanged(const FText& NewText);

	void JoinSessionFailed();

	void UpdateLobbyList(const TArray<FOnlineSessionSearchResult>& OnlineSessionSearchResults);

	UPROPERTY(meta = (BindWidget))
	UScrollBox* ScrollBox_Session;

	UPROPERTY(meta = (BindWidget))
	UButton* Button_JoinSession;
	
	UPROPERTY(EditDefaultsOnly, Category = "Session")
	TSubclassOf<USessionEntryWidget> SessionEntryWidgetClass;

	FString CurrentSelectedSessionId = "";

	UFUNCTION()
	void JoinSessionButtonClicked();

	void SessionEntrySelected(const FString& SelectedEntryIdStr);
	
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
