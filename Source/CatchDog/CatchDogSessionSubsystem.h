// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include <Subsystems/GameInstanceSubsystem.h>
#include <OnlineSessionSettings.h>
#include <Interfaces/OnlineSessionInterface.h>
#include "CatchDogSessionSubsystem.generated.h"

USTRUCT(BlueprintType)
struct FCatchDogSessionSettings
{
	GENERATED_BODY()

	/* This structure is used to store information
	for the Session Settings. It includes:
	- Max number of players for the session
	- bLan: if session is LAN or not.
	- Name of the level
	- IsValid() function to check if there are more 
	than 2 players. Otherwise it will not be valid.
	*/

	UPROPERTY(BlueprintReadWrite)
	int32 MaxPlayers;

	UPROPERTY(BlueprintReadWrite)
	bool bLan = true;

	UPROPERTY(BlueprintReadWrite)
	FName LevelName;

	bool IsValid() const;
};

// This delegate is created to be used when a session is created successfully or not.
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnSessionDelegate, bool, Successful);

USTRUCT(BlueprintType)
struct FCatchDogSessionResult
{
	GENERATED_BODY()

	FOnlineSessionSearchResult SessionResult;
};

// This delegate is created to be used when finding sessions, with the Session results as output.
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnSessionsFoundDelegate, const TArray<FCatchDogSessionResult>&, SessionResults);

UCLASS()
class CATCHDOG_API UCatchDogSessionSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:

	/* Create Session*/
	UFUNCTION(BlueprintCallable)
	void CreateSession(const FCatchDogSessionSettings SessionSettings);
	void OnSessionCreated(FName SessionName, bool bSuccessful);
	UPROPERTY(BlueprintAssignable)
	FOnSessionDelegate OnCreateSessionCompleted;

	/* Start Session */
	UFUNCTION(BlueprintCallable)
	void StartSession();
	void OnSessionStarted(FName SessionName, bool bSuccessful);
	UPROPERTY(BlueprintAssignable)
	FOnSessionDelegate OnStartSessionCompleted;

	/* Update Session */
	UFUNCTION(BlueprintCallable)
	void UpdateSession(const FCatchDogSessionSettings SessionSettings);
	void OnSessionUpdated(FName SessionName, bool bSuccessful);
	UPROPERTY(BlueprintAssignable)
	FOnSessionDelegate OnUpdateSessionCompleted;

	/* Find Sessions */
	UFUNCTION(BlueprintCallable)
	void FindSessions(int32 MaxSearchResults, bool bLan);
	void OnFindSessions(bool bSuccessful);
	UPROPERTY(BlueprintAssignable)
	FOnSessionsFoundDelegate OnSessionsFound;

	UFUNCTION(BlueprintPure)
	bool IsValid(const FCatchDogSessionResult& Session);

	UFUNCTION(BlueprintPure)
	int32 GetPingInMs(const FCatchDogSessionResult& Session);

	UFUNCTION(BlueprintPure)
	FString GetHostName(const FCatchDogSessionResult& Session);

	UFUNCTION(BlueprintPure)
	FString GetMapName(const FCatchDogSessionResult& Session);

	UFUNCTION(BlueprintPure)
	int32 GetCurrentPlayers(const FCatchDogSessionResult& Session);

	UFUNCTION(BlueprintPure)
	int32 GetMaxPlayers(const FCatchDogSessionResult& Session);

	/* Join Session*/
	UFUNCTION(BlueprintCallable)
	void JoinSession(const FCatchDogSessionResult& Session);
	void OnSessionJoined(FName SessionName, EOnJoinSessionCompleteResult::Type Result);
	UPROPERTY(BlueprintAssignable)
	FOnSessionDelegate OnJoinSessionCompleted;

	/* Destroy Session*/
	UFUNCTION(BlueprintCallable)
	void DestroySession();
	void OnSessionDestroyed(FName SessionName, bool bSuccessful);
	UPROPERTY(BlueprintAssignable)
	FOnSessionDelegate OnDestroySessionCompleted;

private:
	/* Create Session*/
	TSharedPtr<FOnlineSessionSettings> LastSessionSettings;
	FDelegateHandle OnCreateSessionCompleteHandle;
	
	/* Start Session*/
	FDelegateHandle OnStartSessionCompleteHandle;
	FDelegateHandle OnUpdateSessionCompleteHandle;

	/* Find Sessions*/
	TSharedPtr<FOnlineSessionSearch> LastSessionSearch;
	FDelegateHandle OnFindSessionsCompleteHandle;

	/* Join Sessions*/
	FDelegateHandle OnJoinSessionsCompleteHandle;
	
	/* Destroy Session*/
	FDelegateHandle OnDestroySessionCompleteHandle;
};
