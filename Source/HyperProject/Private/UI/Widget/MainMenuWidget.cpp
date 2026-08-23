// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/Widget/MainMenuWidget.h"
#include "Components/Button.h"
#include "Components/EditableText.h"
#include "Components/ScrollBox.h"
#include "Components/WidgetSwitcher.h"
#include "GameMode/HPGameInstance.h"
#include "Network/HPNetStatics.h"
#include "UI/Widget/WaitingWidget.h"
#include "UI/Widget/SessionEntryWidget.h"

void UMainMenuWidget::NativeConstruct()
{
	Super::NativeConstruct();

	HPGameInstance = GetGameInstance<UHPGameInstance>();

	if (HPGameInstance)
	{
		HPGameInstance->OnLoginCompleted.AddUObject(this, &UMainMenuWidget::LoginCompleted);

		if (HPGameInstance->IsLoggedIn()) //이미 로그인 되어있으면 메인 위젯으로 넘기기
		{
			SwitchToMainWidget();
		}
		HPGameInstance->OnJoinSessionFailed.AddUObject(this, &UMainMenuWidget::JoinSessionFailed);
		HPGameInstance->OnGlobalSessionSearchCompleted.AddUObject(this, &UMainMenuWidget::UpdateLobbyList);
		HPGameInstance->StartGlobalSessionSearch();
	}
	Button_Login->OnClicked.AddDynamic(this,&UMainMenuWidget::LoginButtonClicked);
	Button_CreateSession->OnClicked.AddDynamic(this,&UMainMenuWidget::CreateSessionButtonClicked);
	Button_CreateSession->SetIsEnabled(false);
	
	EditableText_NewSessionName->OnTextChanged.AddDynamic(this, &UMainMenuWidget::NewSessionNameTextChanged);
	Button_JoinSession->OnClicked.AddDynamic(this, &UMainMenuWidget::JoinSessionButtonClicked);
	Button_JoinSession->SetIsEnabled(false);
}

void UMainMenuWidget::SwitchToMainWidget()
{
	if (WidgetSwitcher_Main)
	{
		WidgetSwitcher_Main->SetActiveWidget(MainWidgetRoot);
	}
}

void UMainMenuWidget::CancelSessionCreation()
{
	if (HPGameInstance)
	{
		HPGameInstance->CancelSessionCreation();
	}
	SwitchToMainWidget();
}

void UMainMenuWidget::CreateSessionButtonClicked()
{
	if (HPGameInstance && HPGameInstance->IsLoggedIn())
	{
		HPGameInstance->RequestCreateAndJoinSession(FName(EditableText_NewSessionName->GetText().ToString()));
		SwitchToWaitingWidget(FText::FromString("Creating Lobby"), true).
		AddDynamic(this, &UMainMenuWidget::CancelSessionCreation);
	}
}

void UMainMenuWidget::NewSessionNameTextChanged(const FText& NewText)
{
	Button_CreateSession->SetIsEnabled(!NewText.IsEmpty());

}

void UMainMenuWidget::JoinSessionFailed()
{
	SwitchToMainWidget();
}

void UMainMenuWidget::UpdateLobbyList(const TArray<FOnlineSessionSearchResult>& OnlineSessionSearchResults)
{
	UE_LOG(LogTemp, Warning, TEXT("Updating Session Search Results"))
	ScrollBox_Session->ClearChildren();

	bool bCurrentSelectedSessionValid = false;

	for (const FOnlineSessionSearchResult& SearchResult : OnlineSessionSearchResults)
	{
		USessionEntryWidget* NewSessionWidget = CreateWidget<USessionEntryWidget>(GetOwningPlayer(), SessionEntryWidgetClass);
		if (NewSessionWidget)
		{
			FString SessionName = "Name_None";
			SearchResult.Session.SessionSettings.Get<FString>(UHPNetStatics::GetSessionNameKey(), SessionName);

			FString SessionIdStr = SearchResult.Session.GetSessionIdStr();
			NewSessionWidget->InitializeEntry(SessionName, SessionIdStr);
			NewSessionWidget->OnSessionEntrySelected.AddUObject(this, &UMainMenuWidget::SessionEntrySelected);
			ScrollBox_Session->AddChild(NewSessionWidget);

			if (CurrentSelectedSessionId == SessionIdStr)
			{
				bCurrentSelectedSessionValid = true;
			}
		}
	}
	CurrentSelectedSessionId = bCurrentSelectedSessionValid?CurrentSelectedSessionId: "";
	Button_JoinSession->SetIsEnabled(bCurrentSelectedSessionValid);
	
}

void UMainMenuWidget::JoinSessionButtonClicked()
{
	if (HPGameInstance && !CurrentSelectedSessionId.IsEmpty())
	{
		UE_LOG(LogTemp, Warning, TEXT("Trying to join session with id: %s"), *CurrentSelectedSessionId);
		if (HPGameInstance->JoinSessionWithId(CurrentSelectedSessionId))
		{
			SwitchToWaitingWidget(FText::FromString("Joining"), false);
		}
		
	}
	if (!CurrentSelectedSessionId.IsEmpty())
	{
		UE_LOG(LogTemp , Warning, TEXT("Trying to join session with id : %s"), *CurrentSelectedSessionId)
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Can't Join Session, no Session Selected"));
	}
}

void UMainMenuWidget::SessionEntrySelected(const FString& SelectedEntryIdStr)
{
	CurrentSelectedSessionId = SelectedEntryIdStr;
}

void UMainMenuWidget::LoginButtonClicked()
{
	UE_LOG(LogTemp, Warning, TEXT("Login"));
	if (HPGameInstance && !HPGameInstance->IsLoggingIn() && !HPGameInstance->IsLoggedIn())
	{
		HPGameInstance -> ClientAccountPortalLogin();
		SwitchToWaitingWidget(FText::FromString("Logging In"), false);
	}
}

void UMainMenuWidget::LoginCompleted(bool bWasSuccessful, const FString& PlayerNickname, const FString& ErrorMsg)
{
	if (bWasSuccessful)
	{
		UE_LOG(LogTemp, Warning, TEXT("Login Successful"));
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Login Failed"));
	}
	SwitchToMainWidget();
}

FOnButtonClickedEvent& UMainMenuWidget::SwitchToWaitingWidget(const FText& WaitInfo, bool bAllowCancel)
{
	WidgetSwitcher_Main->SetActiveWidget(WaitingWidget);
	WaitingWidget->SetWaitInfo(WaitInfo, bAllowCancel);

	return WaitingWidget->ClearAndGetButtonClickedEvent();
}
