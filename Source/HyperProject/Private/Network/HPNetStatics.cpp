// Fill out your copyright notice in the Description page of Project Settings.


#include "Network/HPNetStatics.h"

uint8 UHPNetStatics::GetPlayerCountPerTeam()
{
	return 5;
}

bool UHPNetStatics::IsSessionServer(const UObject* WorldContextObject)
{
	return WorldContextObject->GetWorld()->GetNetMode() == ENetMode::NM_DedicatedServer;
}

FString UHPNetStatics::GetSessionNameStr()
{
	return GetCommandLineArgAsString(GetSessionNameKey());
}

FName UHPNetStatics::GetSessionNameKey()
{
	return FName("SESSION_NAME");
}

FString UHPNetStatics::GetSessionSearchIdStr()
{
	return GetCommandLineArgAsString(GetSessionSearchIdKey());
}

FName UHPNetStatics::GetSessionSearchIdKey()
{
	return FName("SESSION_SEARCH_ID");
}

int UHPNetStatics::GetSessionPort()
{
	return GetCommandLineArgAsInt(GetSessionPortKey());
}

FName UHPNetStatics::GetSessionPortKey()
{
	return FName("PORT");
}

FName UHPNetStatics::GetCoordinatorURLKey()
{
	return FName("COORDINATOR_URL");
}

FString UHPNetStatics::GetCoordinatorURL()
{
	FString CoordinatorURL = GetCommandLineArgAsString(GetCoordinatorURLKey());

	if (CoordinatorURL != "")
	{
		return CoordinatorURL;
	}

	return GetDefaultCoordinatorURL();
}

FString UHPNetStatics::GetDefaultCoordinatorURL()
{
	FString CoordinatorURL = "";

	GConfig->GetString(TEXT("HyperProject.Net"), TEXT("CoordinatorURL"),CoordinatorURL, GGameIni);
	UE_LOG(LogTemp, Warning, TEXT("Getting Default Coordinator URL as: %s"), *CoordinatorURL);

	return CoordinatorURL;
}

FString UHPNetStatics::GetCommandLineArgAsString(const FName& ParamName)
{
	FString OutVal = "";

	FString CommandLineArg = FString::Printf(TEXT("%s="), *ParamName.ToString());

	FParse::Value(FCommandLine::Get(), *CommandLineArg, OutVal);

	return OutVal;
}

int UHPNetStatics::GetCommandLineArgAsInt(const FName& ParamName)
{
	int OutVal = 0;

	FString CommandLineArg = FString::Printf(TEXT("%s="), *ParamName.ToString());

	FParse::Value(FCommandLine::Get(), *CommandLineArg, OutVal);

	return OutVal;
}

FString UHPNetStatics::GetTestingURL()
{
	FString TestURL = GetCommandLineArgAsString(GetTestingURLKey());
	UE_LOG(LogTemp, Warning, TEXT("Get Testing URL: %s"),*TestURL);

	return TestURL;
}

FName UHPNetStatics::GetTestingURLKey()
{
	return FName("TESTING_URL");
}

void UHPNetStatics::ReplacePort(FString& OutURLStr, int NewPort)
{
	FURL URL(nullptr, *OutURLStr, ETravelType::TRAVEL_Absolute);
	URL.Port = NewPort;
	OutURLStr = URL.ToString();
}


FOnlineSessionSettings UHPNetStatics::GenerateOnlineSessionSettings(const FName& SessionName,
                                                                    const FString& SessionSearchID, int Port)
{
	FOnlineSessionSettings OnlineSessionSettings{};
	OnlineSessionSettings.bIsLANMatch = false;
	OnlineSessionSettings.NumPublicConnections = GetPlayerCountPerTeam() * 2;
	OnlineSessionSettings.bShouldAdvertise = true;
	OnlineSessionSettings.bUsesPresence = false;//서버라서 false
	OnlineSessionSettings.bAllowJoinViaPresence = false;
	OnlineSessionSettings.bAllowJoinViaPresenceFriendsOnly = false;
	OnlineSessionSettings.bAllowInvites = true;
	OnlineSessionSettings.bAllowJoinInProgress = false; //진행 중에는 못 들어오도록
	OnlineSessionSettings.bUseLobbiesIfAvailable = false;
	OnlineSessionSettings.bUseLobbiesVoiceChatIfAvailable = false;
	OnlineSessionSettings.bUsesStats = true;

	OnlineSessionSettings.Set(
		UHPNetStatics::GetSessionNameKey(),
		SessionName.ToString(),
		EOnlineDataAdvertisementType::ViaOnlineServiceAndPing);

	OnlineSessionSettings.Set(
	UHPNetStatics::GetSessionSearchIdKey(),
	SessionSearchID,
	EOnlineDataAdvertisementType::ViaOnlineServiceAndPing);

	OnlineSessionSettings.Set(
	UHPNetStatics::GetSessionPortKey(),
	Port,
	EOnlineDataAdvertisementType::ViaOnlineServiceAndPing);
	
	return OnlineSessionSettings;
}

IOnlineSessionPtr UHPNetStatics::GetSessionPtr()
{
	IOnlineSubsystem* OnlineSubsystem = IOnlineSubsystem::Get();
	if (OnlineSubsystem)
		return OnlineSubsystem->GetSessionInterface();
	return nullptr;
}

IOnlineIdentityPtr UHPNetStatics::GetIdentityPtr()
{
	IOnlineSubsystem* OnlineSubsystem = IOnlineSubsystem::Get();
	if (OnlineSubsystem)
		return OnlineSubsystem->GetIdentityInterface();
	return nullptr;
}
