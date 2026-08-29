// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/Widget/MainMenuTestWidget.h"




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

void UMainMenuTestWidget::NativeConstruct()
{
	Super::NativeConstruct();

	HPGameInstance = GetGameInstance<UHPGameInstance>();

	if (HPGameInstance)
	{
		HPGameInstance->OnLoginCompleted.AddUObject(this, &UMainMenuTestWidget::LoginCompleted);

		if (HPGameInstance->IsLoggedIn()) //이미 로그인 되어있으면 메인 위젯으로 넘기기
		{
			SwitchToMainWidget();
		}
	}
	Button_Login->OnClicked.AddDynamic(this,&UMainMenuTestWidget::LoginButtonClicked);
}

void UMainMenuTestWidget::SwitchToMainWidget()
{
	if (WidgetSwitcher_Main)
	{
		//WidgetSwitcher_Main->SetActiveWidget(MainWidgetRoot);
	}
}

void UMainMenuTestWidget::LoginButtonClicked()
{
	UE_LOG(LogTemp, Warning, TEXT("Login"));
	if (HPGameInstance && !HPGameInstance->IsLoggingIn() && !HPGameInstance->IsLoggedIn())
	{
		HPGameInstance -> ClientAccountPortalLogin();
		SwitchToWaitingWidget(FText::FromString("Logging In"), false);
	}
}

void UMainMenuTestWidget::LoginCompleted(bool bWasSuccessful, const FString& PlayerNickname, const FString& ErrorMsg)
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

FOnButtonClickedEvent& UMainMenuTestWidget::SwitchToWaitingWidget(const FText& WaitInfo, bool bAllowCancel)
{
	WidgetSwitcher_Main->SetActiveWidget(WaitingWidget);
	WaitingWidget->SetWaitInfo(WaitInfo, bAllowCancel);

	return WaitingWidget->ClearAndGetButtonClickedEvent();
}
