// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "Interfaces/IHttpRequest.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "OnlineSessionSettings.h"
#include "UI/Widget/MainMenuWidget.h"
#include "HPGameInstance.generated.h"

/**
 * 
 */

class FOnlineSessionSearch;
class FOnlineSessionSearchResult;

DECLARE_MULTICAST_DELEGATE_ThreeParams(FOnLoginCompleted, bool /*bWasSuccessful*/, const FString&/*PlayerNickname*/,
                                       const FString& /*ErrorMsg*/);
DECLARE_MULTICAST_DELEGATE_OneParam(FOnFindGlobalSeesionSearchCompleted, const TArray<FOnlineSessionSearchResult>& /*Search Results*/)

DECLARE_MULTICAST_DELEGATE(FOnJoinSeesionFailed)

UCLASS()
class HYPERPROJECT_API UHPGameInstance : public UGameInstance
{
	GENERATED_BODY()
	
public:
	void StartMatch();
	virtual void Init() override;
	
	/* Login */

public:
	bool IsLoggedIn() const;
	bool IsLoggingIn() const;
	void ClientAccountPortalLogin();
	
	FOnLoginCompleted OnLoginCompleted;
	
private:
	void ClientLogin(const FString& Type, const FString& Id, const FString& Token);
	void LoginCompleted(int NumOfLocalPlayer, bool bWasSuccessful, const FUniqueNetId& UserId, const FString& ErrorMsg);
	FDelegateHandle LoggingInDelegateHandle;

	/* Client Session Creation and Search */
public:
	void RequestCreateAndJoinSession(const FName& NewSessionName);
	void CancelSessionCreation();
	void StartGlobalSessionSearch();
	bool JoinSessionWithId(const FString& SessionIdStr);
	
	FOnJoinSeesionFailed OnJoinSessionFailed;
	FOnFindGlobalSeesionSearchCompleted OnGlobalSessionSearchCompleted;

private:
	void SessionCreationRequestCompleted(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bConnectedSuccessfully, FGuid SessionSearchId);
	void StartFindingCreatedSession(const FGuid& SessionSearchId);
	void StopAllSessionFindings();
	void StopFindingCreatedSession();
	void StopGlobalSessionSearch();
	void FindGlobalSessions();
	void GlobalSessionSearchCompleted(bool bWasSuccessful);

	FTimerHandle FindCreatedSessionTimerHandle;
	FTimerHandle FindCreatedSessionTimeoutTimerHandle;
	FTimerHandle GlobalSessionSearchTimerHandle;

	UPROPERTY(EditDefaultsOnly, Category = "Session Search")
	float FindCreatedSessionSearchInterval = 1.f;

	UPROPERTY(EditDefaultsOnly, Category = "Session Search")
	float FindCreatedSessionTimeoutDuration = 60.f;

	UPROPERTY(EditDefaultsOnly, Category = "Session Search")
	float GlobalSessionSearchInterval = 1.f;

	void FindCreatedSession(FGuid SessionSearchId);
	void FindCreatedSessionTimeout();
	void FindCreatedSessionCompleted(bool bWasSuccessful);
	void JoinSessionWithSearchResult(const FOnlineSessionSearchResult& SearchResult);
	void JoinSessionCompleted(FName SessionName, EOnJoinSessionCompleteResult::Type JoinResult, int Port);

	TSharedPtr<FOnlineSessionSearch> SessionSearch;
	 /* Session Server */
public:
	void PlayerJoined(const FUniqueNetIdRepl& UniqueId);
	void PlayerLeft(const FUniqueNetIdRepl& UniqueId);
	
private:
	void CreateSession();
	void OnSessionCreated(FName SessionName, bool bWasSuccessful);
	void EndSessionCompleted(FName SessionName, bool bWasSuccessful);
	void WaitPlayerJoinTimeOutReached();

	FString ServerSessionName;
	int SessionServerPort ;

	void TerminateSessionServer();

	FTimerHandle WaitPlayerJoinTimeOutHandle;

	UPROPERTY(EditDefaultsOnly , Category = "Session")
	float WaitPlayerJoinTimeOutDuration = 60.f;

	TSet<FUniqueNetIdRepl> PlayerRecord;
private:
	UPROPERTY(EditDefaultsOnly, Category = "Map")
	TSoftObjectPtr<UWorld> MainMenuLevel; //전체를 불러오지 않기 위해 SoftObjectPtr

	UPROPERTY(EditDefaultsOnly, Category = "Map")
	TSoftObjectPtr<UWorld> LobbyMenuLevel;

	UPROPERTY(EditDefaultsOnly, Category = "Map")
	TSoftObjectPtr<UWorld> GameLevel;

	void LoadLevelAndListen(TSoftObjectPtr<UWorld> Level);
};
