// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameState.h"
#include "CatchDogEnums.h"
#include "PickableActor.h"
#include "Puppy.h"
#include "CatchDogGameState.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPreTimeUpdate, int, Seconds);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnTimeUpdate, int, Minutes, int, Seconds);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnSpawnDestroyActionUpdate, FText, Action, int, Seconds);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnSpawnPuppies, bool, Activate);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnGameOver, int, Score);


class ACatchDogGameMode;
class ACatchDogPlayerState;
class ACatchDogPlayerController;
/**
 * 
 */
UCLASS()
class CATCHDOG_API ACatchDogGameState : public AGameState
{
	GENERATED_BODY()

public:

	ACatchDogGameState();
	
	UPROPERTY(BlueprintAssignable)
	FOnPreTimeUpdate OnPreTimeUpdate;

	UPROPERTY(BlueprintAssignable)
	FOnTimeUpdate OnTimeUpdate;

	UPROPERTY(BlueprintAssignable)
	FOnSpawnDestroyActionUpdate OnSpawnDestroyActionUpdate;

	UPROPERTY(BlueprintAssignable)
	FOnGameOver OnGameOver;

	UPROPERTY(BlueprintAssignable)
	FOnSpawnPuppies OnSpawnPuppies;

	void ShowEndScreen();
 
protected:

	virtual void BeginPlay() override;

	// Timer related elements
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Timer")
	float DelayBeforeGameStart = 10.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Timer")
	float GameTimerStartValue = 120.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Timer")
	float SpawnTimerZone1Value = 10.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Timer")
	float SpawnTimerZone2Value = 20.0f;

	UPROPERTY(ReplicatedUsing = OnRep_PreGameTimer)
	int SecondsPreGame;

	UPROPERTY(ReplicatedUsing = OnRep_GameTimer)
	int Seconds;

	UPROPERTY(ReplicatedUsing = OnRep_GameOver)
	bool bGameOver = false;

	UFUNCTION()
	void OnRep_GameOver();

	UFUNCTION()
	void OnRep_PreGameTimer();

	UFUNCTION()
	void OnRep_GameTimer();

	UPROPERTY(ReplicatedUsing = OnRep_SpawnTimer)
	int TimeLeft;

	UFUNCTION()
	void OnRep_SpawnTimer();

	virtual void GetLifetimeReplicatedProps(TArray< FLifetimeProperty >& OutLifetimeProps) const override;

	void StartGame();

	void StartTimers();

	void StartSpawnTimerZone1();

	void StartSpawnTimerZone2();

	void OnSpawnTimerZone1End();

	void OnSpawnTimerZone2End();

	void UpdatePreTimer();

	void UpdateTimer();

	void Endgame();

	// List of players to have finished the game and their score

	UPROPERTY(Replicated)
	int32 WinnerScore = 0;

private:

	ACatchDogGameMode* CatchDogGameMode = nullptr;

	FTimerHandle PreGameTimer;

	FTimerHandle GameTimer;

	FTimerHandle UpdateUITimer;

	FTimerHandle SpawnTimerZone1;

	FTimerHandle SpawnTimerZone2;

	UPROPERTY(Replicated)
	FText ActionString;
};
