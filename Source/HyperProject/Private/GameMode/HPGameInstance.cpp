// Fill out your copyright notice in the Description page of Project Settings.


#include "GameMode/HPGameInstance.h"

#include "Network/HPNetStatics.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "Interfaces/OnlineIdentityInterface.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "HttpModule.h"
#include "Interfaces/IHttpResponse.h"

void UHPGameInstance::StartMatch()
{
	if (GetWorld()->GetNetMode()==ENetMode::NM_DedicatedServer || GetWorld()->GetNetMode() == ENetMode::NM_ListenServer)
	{
		LoadLevelAndListen(GameLevel);
		return;
	}
	
}

void UHPGameInstance::LoadLevelAndListen(TSoftObjectPtr<UWorld> Level)
{
	const FName LevelURL = FName(*FPackageName::ObjectPathToPackageName(Level.ToString()));

	if (LevelURL != "")
	{
		//NEXTTHINGTODO: LISTEN빼기
		FString TravelStr = FString::Printf(TEXT("%s?listen?port=%d"),*LevelURL.ToString(), SessionServerPort);
		UE_LOG(LogTemp, Warning, TEXT("Server Traveling to: %s"), *TravelStr);
		GetWorld()->ServerTravel(TravelStr);
	}
}

void UHPGameInstance::Init()
{
	Super::Init();
	
	if(GetWorld()->IsEditorWorld()) //에디터일 경우 반환
	{
		return;
	}

	if (UHPNetStatics::IsSessionServer(this))
	{
		CreateSession();
	}
	
}

bool UHPGameInstance::IsLoggedIn() const
{
	if (IOnlineIdentityPtr IdentityPtr = UHPNetStatics::GetIdentityPtr())
	{
		return IdentityPtr->GetLoginStatus(0) == ELoginStatus::LoggedIn; 
	}
	return false;
}

bool UHPGameInstance::IsLoggingIn() const
{
	return LoggingInDelegateHandle.IsValid();
}

void UHPGameInstance::ClientAccountPortalLogin()
{
	ClientLogin("AccountPortal", "", "");
}

void UHPGameInstance::ClientLogin(const FString& Type, const FString& Id, const FString& Token)
{
	if (IOnlineIdentityPtr IdentityPtr= UHPNetStatics::GetIdentityPtr())
	{
		if (LoggingInDelegateHandle.IsValid()) //핸들에 델리게이트 등록 전 이전 델리게이트 다 지우기
		{
			IdentityPtr->OnLoginCompleteDelegates->Remove(LoggingInDelegateHandle);
			LoggingInDelegateHandle.Reset();
		}
		
		LoggingInDelegateHandle = IdentityPtr->OnLoginCompleteDelegates->AddUObject(this, &UHPGameInstance::LoginCompleted);

		if (!IdentityPtr->Login(0, FOnlineAccountCredentials(Type,Id,Token))) 
		{
			UE_LOG(LogTemp, Warning,TEXT("Login Failed Right Away"));

			if (LoggingInDelegateHandle.IsValid())
			{
				IdentityPtr->OnLoginCompleteDelegates->Remove(LoggingInDelegateHandle);
				LoggingInDelegateHandle.Reset();
			}
			OnLoginCompleted.Broadcast(false, "","Login Failed Right Away");
		}
	}
}

void UHPGameInstance::LoginCompleted(int NumOfLocalPlayer, bool bWasSuccessful, const FUniqueNetId& UserId,
	const FString& ErrorMsg)
{
	if (IOnlineIdentityPtr IdentityPtr= UHPNetStatics::GetIdentityPtr())
	{
		if (LoggingInDelegateHandle.IsValid())
		{
			IdentityPtr->OnLoginCompleteDelegates->Remove(LoggingInDelegateHandle);
			LoggingInDelegateHandle.Reset();
		}

		FString PlayerNickName = "";
		if (bWasSuccessful) //로그인 성공 시
		{
			PlayerNickName = IdentityPtr->GetPlayerNickname(UserId); //닉네임
			UE_LOG(LogTemp, Warning, TEXT("Logged In Successfully as: %s"), *PlayerNickName);
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("Logged In Failed: %s"), *ErrorMsg);
		}

		OnLoginCompleted.Broadcast(bWasSuccessful, PlayerNickName, ErrorMsg); //UMainMenuWidget::LoginCompleted에 브로드캐스트
	}
	else
	{
		OnLoginCompleted.Broadcast(false, "", "Can't Find the Identity Ptr");
	}
}

void UHPGameInstance::RequestCreateAndJoinSession(const FName& NewSessionName) //UMainMenuWidget::CreateSessionButtonClicked
{
	UE_LOG(LogTemp, Warning,TEXT("Request Create And Join Session: %s"), *NewSessionName.ToString());
	FHttpRequestRef Request =  FHttpModule::Get().CreateRequest();

	FGuid SessionSearchId= FGuid::NewGuid();

	FString CoordinatorURL = UHPNetStatics::GetCoordinatorURL();

	FString URL = FString::Printf(TEXT("%s/Sessions"), *CoordinatorURL);
	UE_LOG(LogTemp, Warning, TEXT("Sending Request Session Creation to URL: %s"), *URL);

	Request->SetURL(URL);
	Request->SetVerb("POST");

	Request->SetHeader(TEXT("Content-Type"), TEXT("application/json"));

	TSharedPtr<FJsonObject> JsonObject = MakeShareable(new FJsonObject);
	JsonObject->SetStringField(UHPNetStatics::GetSessionNameKey().ToString(), NewSessionName.ToString());
	JsonObject->SetStringField(UHPNetStatics::GetSessionSearchIdKey().ToString(), SessionSearchId.ToString());

	FString RequestBody;
	TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&RequestBody);
	FJsonSerializer::Serialize(JsonObject.ToSharedRef(), Writer);

	Request->SetContentAsString(RequestBody);
	Request->OnProcessRequestComplete().BindUObject(this, &UHPGameInstance::SessionCreationRequestCompleted,SessionSearchId);

	if (!Request->ProcessRequest())
	{
		UE_LOG(LogTemp, Warning, TEXT("Session Creation Request Failed Right Away"));
	}
}

void UHPGameInstance::CancelSessionCreation() //UMainMenuWidget::CancelSessionCreation-세션 찾던 중 Cancel버튼 누른 경우
{
	UE_LOG(LogTemp, Warning, TEXT("Canceling Session Creation"));
	StopAllSessionFindings();

	if (IOnlineSessionPtr SessionPtr = UHPNetStatics::GetSessionPtr())
	{
		SessionPtr->OnFindSessionsCompleteDelegates.RemoveAll(this);
		SessionPtr->OnJoinSessionCompleteDelegates.RemoveAll(this);
	}

	StartGlobalSessionSearch();
}

void UHPGameInstance::StartGlobalSessionSearch()
{
	UE_LOG(LogTemp, Warning,TEXT("Starting Global Session search"));

	GetWorld()->GetTimerManager().SetTimer(GlobalSessionSearchTimerHandle, this, &UHPGameInstance::FindGlobalSessions, GlobalSessionSearchInterval, true, 0.f);
}

bool UHPGameInstance::JoinSessionWithId(const FString& SessionIdStr)
{
	if (SessionSearch.IsValid())
	{
		const FOnlineSessionSearchResult* SessionSearchResult = SessionSearch->SearchResults.FindByPredicate(
			[=](const FOnlineSessionSearchResult& Result)
			{
				return Result.GetSessionIdStr() == SessionIdStr;
			}
		);

		if (SessionSearchResult)
		{
			JoinSessionWithSearchResult(*SessionSearchResult);
			return true;
		}
	}
	return false;
}

void UHPGameInstance::SessionCreationRequestCompleted(FHttpRequestPtr Request, FHttpResponsePtr Response,
                                                      bool bConnectedSuccessfully, FGuid SessionSearchId) //세션 리퀘스트 완료되면 콜백되는 함수
{
	if (!bConnectedSuccessfully)
	{
		UE_LOG(LogTemp, Warning, TEXT("Connection Responded with connection was not successful"));
	}
	else //성공적으로 Coordinator에 연결된 경우
	{
		UE_LOG(LogTemp, Warning, TEXT("Connection to Coordinator Successful"));

		int32 ResponseCode = Response->GetResponseCode();

		if (ResponseCode != 200)
		{
			UE_LOG(LogTemp, Warning,TEXT("Session Creation Failed, with code: %d"), ResponseCode);
			return;
		}

		FString ResponseStr = Response->GetContentAsString();

		TSharedPtr<FJsonObject> JsonObject;
		TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(ResponseStr);

		int32 Port = 0;

		if (FJsonSerializer::Deserialize(Reader, JsonObject) && JsonObject.IsValid())
		{
			Port = JsonObject->GetIntegerField(*UHPNetStatics::GetSessionPortKey().ToString());
		}

		UE_LOG(LogTemp, Warning,TEXT("Connected to coordinator successfully and the new session created is on port: %d"), Port);
		StartFindingCreatedSession(SessionSearchId);
	}
}

void UHPGameInstance::StartFindingCreatedSession(const FGuid& SessionSearchId)
{
	if (!SessionSearchId.IsValid())
	{
		UE_LOG(LogTemp, Warning, TEXT("Session Search Id is invalid, can't start finding"));
		return;
	}
	StopAllSessionFindings(); //기존 세션 찾기 중단
	UE_LOG(LogTemp, Warning, TEXT("Start Finding Created Session with Id: %s"), *SessionSearchId.ToString());

	//FindCreatedSessionSearchInterval마다 세션 찾기
	GetWorld()->GetTimerManager().SetTimer(
		FindCreatedSessionTimerHandle,
		FTimerDelegate::CreateUObject(this, &UHPGameInstance::FindCreatedSession, SessionSearchId),
		FindCreatedSessionSearchInterval,
		true,
		0.f
		);

	//FindCreatedSessionTimeoutDuration동안 없으면 중단
	GetWorld()->GetTimerManager().SetTimer(
	FindCreatedSessionTimeoutTimerHandle,
	this,
	&UHPGameInstance::FindCreatedSessionTimeout,
	FindCreatedSessionTimeoutDuration
	);
}

void UHPGameInstance::StopAllSessionFindings()
{
	UE_LOG(LogTemp, Warning, TEXT("Stoping all session search"));
	StopFindingCreatedSession();
	StopGlobalSessionSearch();
}

void UHPGameInstance::StopFindingCreatedSession()
{
	UE_LOG(LogTemp, Warning, TEXT("Stop Finding Created Session"))

	//세션 찾던 타이머, 타임아웃 타이머 모두 Clear
	GetWorld()->GetTimerManager().ClearTimer(FindCreatedSessionTimerHandle);
	GetWorld()->GetTimerManager().ClearTimer(FindCreatedSessionTimeoutTimerHandle);

	//델리게이트 정리
	if (IOnlineSessionPtr SessionPtr = UHPNetStatics::GetSessionPtr())
	{
		SessionPtr->OnFindSessionsCompleteDelegates.RemoveAll(this);
		SessionPtr->OnJoinSessionCompleteDelegates.RemoveAll(this);
	}
}

void UHPGameInstance::StopGlobalSessionSearch()
{
	UE_LOG(LogTemp, Warning, TEXT("Stop Global Session Search"));

	if (GlobalSessionSearchTimerHandle.IsValid())
	{
		GetWorld()->GetTimerManager().ClearTimer(GlobalSessionSearchTimerHandle);
	}

	IOnlineSessionPtr SessionPtr = UHPNetStatics::GetSessionPtr();
	if (SessionPtr)
	{
		SessionPtr->OnFindSessionsCompleteDelegates.RemoveAll(this);
	}

}

void UHPGameInstance::FindGlobalSessions()
{
	UE_LOG(LogTemp, Warning, TEXT("-------Retrying Global Session Search -------"));

	IOnlineSessionPtr SessionPtr = UHPNetStatics::GetSessionPtr();

	if (!SessionPtr)
	{
		UE_LOG(LogTemp, Warning, TEXT("Can't find session interface, wait for the next global session search"));
		return;
	}

	SessionSearch = MakeShareable(new FOnlineSessionSearch);
	SessionSearch->bIsLanQuery = false;
	SessionSearch->MaxSearchResults = 20;

	SessionPtr->OnFindSessionsCompleteDelegates.RemoveAll(this);
	SessionPtr->OnFindSessionsCompleteDelegates.AddUObject(this, &UHPGameInstance::GlobalSessionSearchCompleted);
	
	if (!SessionPtr->FindSessions(0, SessionSearch.ToSharedRef()))
	{
		UE_LOG(LogTemp, Warning, TEXT("Find Global Session Failed Right Away"));
		SessionPtr->OnFindSessionsCompleteDelegates.RemoveAll(this);
	}
}

void UHPGameInstance::GlobalSessionSearchCompleted(bool bWasSuccessful)
{
	if (bWasSuccessful)
	{
		OnGlobalSessionSearchCompleted.Broadcast(SessionSearch->SearchResults);
		for (const FOnlineSessionSearchResult& OnlineSessionSearchResult : SessionSearch->SearchResults)
		{
			FString SessionName = "Name_None";
			OnlineSessionSearchResult.Session.SessionSettings.Get<FString>(UHPNetStatics::GetSessionNameKey(), SessionName);
			UE_LOG(LogTemp, Warning, TEXT("Found Session: %s after global session search"), *SessionName);
		}
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Global Session Search completed unsuccessful"));
	}

	IOnlineSessionPtr SessionPtr = UHPNetStatics::GetSessionPtr();

	if (SessionPtr)
	{
		SessionPtr->OnFindSessionsCompleteDelegates.RemoveAll(this);
	}
}

void UHPGameInstance::FindCreatedSession(FGuid SessionSearchId)
{
	UE_LOG(LogTemp, Warning, TEXT("Trying to find Created Session"))

	IOnlineSessionPtr SessionPtr= UHPNetStatics::GetSessionPtr();

	if (!SessionPtr)
	{
		UE_LOG(LogTemp, Warning, TEXT("can't find Session Ptr, canceling session search"));
		return;
	}

	SessionSearch = MakeShareable(new FOnlineSessionSearch);
	if (!SessionSearch)
	{
		UE_LOG(LogTemp, Warning,TEXT("Unable to create session search, canceling sessionSearch"))
		return;
	}
	SessionSearch->bIsLanQuery = false;
	SessionSearch->MaxSearchResults = 1;
	SessionSearch->QuerySettings.Set(UHPNetStatics::GetSessionSearchIdKey(), SessionSearchId.ToString(), EOnlineComparisonOp::Equals);

	SessionPtr->OnFindSessionsCompleteDelegates.RemoveAll(this);
	SessionPtr->OnFindSessionsCompleteDelegates.AddUObject(this, &UHPGameInstance::FindCreatedSessionCompleted);

	if (!SessionPtr->FindSessions(0, SessionSearch.ToSharedRef()))
	{
		UE_LOG(LogTemp, Warning,TEXT("FindSessionFailed Right away"));
		SessionPtr->OnFindSessionsCompleteDelegates.RemoveAll(this);
	}
}

void UHPGameInstance::FindCreatedSessionTimeout()
{
	UE_LOG(LogTemp, Warning, TEXT("Find Created Session Timeout Reached"))
	StopFindingCreatedSession();
}

void UHPGameInstance::FindCreatedSessionCompleted(bool bWasSuccessful)
{
	if (!bWasSuccessful || SessionSearch->SearchResults.Num() == 0)
	{
		return;
	}

	StopFindingCreatedSession();
	JoinSessionWithSearchResult(SessionSearch->SearchResults[0]);	
	
}

void UHPGameInstance::JoinSessionWithSearchResult(const FOnlineSessionSearchResult& SearchResult)
{
	UE_LOG(LogTemp, Warning,TEXT("Joining session with Search Result"));

	IOnlineSessionPtr SessionPtr = UHPNetStatics::GetSessionPtr();//상태에 따라 없을 수 있으므로 항상 새로 get
	if (!SessionPtr)
	{
		UE_LOG(LogTemp, Warning, TEXT("Can't find SessionPtr, Cancel Joining"));
		return;
	}

	FString SessionName = "";
	SearchResult.Session.SessionSettings.Get<FString>(UHPNetStatics::GetSessionNameKey(), SessionName);

	const FOnlineSessionSetting* PortSetting = SearchResult.Session.SessionSettings.Settings.Find(UHPNetStatics::GetSessionPortKey());
	int64 Port = 7777;
	PortSetting->Data.GetValue(Port);
	UE_LOG(LogTemp, Warning, TEXT("trying to join session: %s, at Port: %d"), *SessionName, Port);

	SessionPtr->OnJoinSessionCompleteDelegates.RemoveAll(this);
	SessionPtr->OnJoinSessionCompleteDelegates.AddUObject(this, &UHPGameInstance::JoinSessionCompleted,(int)Port);

	if (!SessionPtr->JoinSession(0, FName(SessionName), SearchResult))
	{
		UE_LOG(LogTemp, Warning, TEXT("Joining Session Failed Right away"));	
		SessionPtr->OnJoinSessionCompleteDelegates.RemoveAll(this);
		OnJoinSessionFailed.Broadcast();
	}
}

void UHPGameInstance::JoinSessionCompleted(FName SessionName, EOnJoinSessionCompleteResult::Type JoinResult, int Port)
{
	IOnlineSessionPtr SessionPtr = UHPNetStatics::GetSessionPtr();

	if (!SessionPtr)
	{
		UE_LOG(LogTemp, Warning, TEXT("Join Session Completed, but can't find session Pointer "));	
		OnJoinSessionFailed.Broadcast();
		return;
	}

	if (JoinResult == EOnJoinSessionCompleteResult::Success)
	{
		StopAllSessionFindings();
		UE_LOG(LogTemp, Warning, TEXT("Joining Session: %s successful, the port is :%d"),*SessionName.ToString(), Port);	

		FString TravelURL = "";
		SessionPtr->GetResolvedConnectString(SessionName, TravelURL);

		FString TestingURL  = UHPNetStatics::GetTestingURL();
		if (!TestingURL.IsEmpty())
		{
			TravelURL = TestingURL;
		}

		UHPNetStatics::ReplacePort(TravelURL, Port);
		UE_LOG(LogTemp, Warning, TEXT("Traveling to session at: %s"), *TravelURL);

		GetFirstLocalPlayerController(GetWorld())->ClientTravel(TravelURL, ETravelType::TRAVEL_Absolute);
	}
	else
	{
		OnJoinSessionFailed.Broadcast();
	}
	SessionPtr->OnJoinSessionCompleteDelegates.RemoveAll(this);
}


void UHPGameInstance::PlayerJoined(const FUniqueNetIdRepl& UniqueId)
{
	if (WaitPlayerJoinTimeOutHandle.IsValid())
	{
		GetWorld()->GetTimerManager().ClearTimer(WaitPlayerJoinTimeOutHandle);
	}
	PlayerRecord.Add(UniqueId);
}

void UHPGameInstance::PlayerLeft(const FUniqueNetIdRepl& UniqueId)
{
	PlayerRecord.Remove(UniqueId);
	if (PlayerRecord.Num() == 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("All Player left the session, terminating"));
		TerminateSessionServer();
	}
}

void UHPGameInstance::CreateSession()
{
	IOnlineSessionPtr SessionPtr = UHPNetStatics::GetSessionPtr();

	if (SessionPtr)
	{
		ServerSessionName = UHPNetStatics::GetSessionNameStr();
		FString SessionSearchID = UHPNetStatics::GetSessionSearchIdStr();
		SessionServerPort = UHPNetStatics::GetSessionPort();
		UE_LOG(LogTemp, Warning, TEXT("#### Create Session With Name %s, ID: %s, Port: %d"),
				*ServerSessionName, *SessionSearchID, SessionServerPort);
		
		FOnlineSessionSettings OnlineSessionSetting = UHPNetStatics::GenerateOnlineSessionSettings(FName(ServerSessionName), SessionSearchID, SessionServerPort);
		SessionPtr->OnCreateSessionCompleteDelegates.RemoveAll(this);
		SessionPtr->OnCreateSessionCompleteDelegates.AddUObject(this, &UHPGameInstance::OnSessionCreated);
		if (!SessionPtr->CreateSession(0, FName(ServerSessionName), OnlineSessionSetting))
		{
			UE_LOG(LogTemp, Warning, TEXT("Session Creating Failed Right Away"));
			SessionPtr->OnCreateSessionCompleteDelegates.RemoveAll(this);
			TerminateSessionServer();
		}
	
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Can't Find Session Pointer, Terminating"));
		TerminateSessionServer();
	}

}

void UHPGameInstance::OnSessionCreated(FName SessionName, bool bWasSuccessful)
{
	if (bWasSuccessful)
	{
		UE_LOG(LogTemp, Warning, TEXT("---------SessionCreated!"));
	GetWorld()->GetTimerManager().SetTimer(WaitPlayerJoinTimeOutHandle,
		this,
		&UHPGameInstance::WaitPlayerJoinTimeOutReached,
		WaitPlayerJoinTimeOutDuration
		);
		LoadLevelAndListen(LobbyMenuLevel);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("---------SessionCreation Failed!"));
		TerminateSessionServer();
	}

	if (IOnlineSessionPtr SessionPtr = UHPNetStatics::GetSessionPtr())
	{
		SessionPtr->OnCreateSessionCompleteDelegates.RemoveAll(this);
	}
}

void UHPGameInstance::EndSessionCompleted(FName SessionName, bool bWasSuccessful)
{
	FGenericPlatformMisc::RequestExit(false);
}

void UHPGameInstance::WaitPlayerJoinTimeOutReached()
{
	UE_LOG(LogTemp, Warning,TEXT("Session Server Shutdown After %f Seconds without PlayerJoining"), WaitPlayerJoinTimeOutDuration);
	TerminateSessionServer();
}

void UHPGameInstance::TerminateSessionServer()
{
	if (IOnlineSessionPtr SessionPtr = UHPNetStatics::GetSessionPtr())
	{
		SessionPtr->OnEndSessionCompleteDelegates.RemoveAll(this);
		
		SessionPtr->OnEndSessionCompleteDelegates.AddUObject(this,&UHPGameInstance::EndSessionCompleted);

		if (!SessionPtr->EndSession(FName{ServerSessionName}))
		{
			FGenericPlatformMisc::RequestExit(false);
		}
	}
	else
	{
		FGenericPlatformMisc::RequestExit(false);
	}
}
