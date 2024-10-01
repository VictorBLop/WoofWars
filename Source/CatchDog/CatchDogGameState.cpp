// Fill out your copyright notice in the Description page of Project Settings.


#include "CatchDogGameState.h"
#include "Kismet/GameplayStatics.h"
#include "CatchDogGameMode.h"
#include "Math/UnrealMathUtility.h"
#include "Bone.h"
#include "BearTrapPickable.h"
#include "NetTrapPickable.h"
#include "Shield.h"
#include "SpeedBoost.h"
#include "CatchDogAIController.h"
#include "CatchDogPlayerState.h"
#include "Engine/Engine.h"
#include "Net/UnrealNetwork.h"
#include "CatchDogPlayerController.h"
#include "CatchDogCharacter.h"


ACatchDogGameState::ACatchDogGameState()
{
	PrimaryActorTick.bCanEverTick = true;

	bReplicates = true;
}

void ACatchDogGameState::BeginPlay()
{
	Super::BeginPlay();

	CatchDogGameMode = Cast<ACatchDogGameMode>(GetWorld()->GetAuthGameMode()); // does not work for multiplayer

	if (HasAuthority())
	{
		// Start Game Timer
		GetWorld()->GetTimerManager().SetTimer(
			PreGameTimer, // handle to cancel timer at a later time
			this, // the owning object
			&ACatchDogGameState::StartGame, // function to call on elapsed
			DelayBeforeGameStart, // float delay until elapsed
			false); // looping

		// Start Game Timer
		GetWorld()->GetTimerManager().SetTimer(
			UpdateUITimer, // handle to cancel timer at a later time
			this, // the owning object
			&ACatchDogGameState::UpdatePreTimer, // function to call on elapsed
			1.0f, // float delay until elapsed
			true); // looping

		WinnerScore = 0;

	}

}

void ACatchDogGameState::StartGame()
{
	// Start actual game

	if(HasAuthority())
	{
		GetWorld()->GetTimerManager().ClearTimer(PreGameTimer);
		GetWorld()->GetTimerManager().ClearTimer(UpdateUITimer);

		StartTimers(); // Start your timers when the class begins play

		for (int32 index = 0; index < PlayerArray.Num(); index++)
		{
			ACatchDogPlayerState* playerState = Cast<ACatchDogPlayerState>(PlayerArray[index]);

			if (playerState)
			{
				if (ACatchDogCharacter* playerCharacter = Cast<ACatchDogCharacter>(playerState->GetPawn()))
				{
					// Change skeletal mesh in Character
					if(playerCharacter->CharacterMeshes.Num() != 0)
					{
						playerCharacter->ChangeSkeletalMesh(index % playerCharacter->CharacterMeshes.Num());
					}
				}
			}
		}
	}

	OnSpawnPuppies.Broadcast(true);
}


void ACatchDogGameState::StartTimers()
{
	// Start Game Timer
	GetWorld()->GetTimerManager().SetTimer(
		UpdateUITimer, // handle to cancel timer at a later time
		this, // the owning object
		&ACatchDogGameState::UpdateTimer, // function to call on elapsed
		1.0f, // float delay until elapsed
		true); // looping

	// Start Update UI Timer
	GetWorld()->GetTimerManager().SetTimer(
		GameTimer, // handle to cancel timer at a later time
		this, // the owning object
		&ACatchDogGameState::Endgame, // function to call on elapsed
		GameTimerStartValue, // float delay until elapsed
		false); // looping

	OnTimeUpdate.Broadcast((int)GameTimerStartValue / 60, (int)GameTimerStartValue % 60); // Broadcast event Update Timer

	StartSpawnTimerZone2();
}

void ACatchDogGameState::StartSpawnTimerZone1()
{
	if(HasAuthority())
	{
		// Start Destroy objects Timer
		GetWorld()->GetTimerManager().SetTimer(
			SpawnTimerZone1, // handle to cancel timer at a later time
			this, // the owning object
			&ACatchDogGameState::OnSpawnTimerZone1End, // function to call on elapsed
			SpawnTimerZone1Value, // float delay until elapsed
			false); // looping

		ActionString = FText::FromString(TEXT("Spawning"));

		CatchDogGameMode->SpawnPowerUps();
		CatchDogGameMode->SpawnPuppies();


		OnSpawnDestroyActionUpdate.Broadcast(ActionString, SpawnTimerZone1Value);
	}
}

void ACatchDogGameState::StartSpawnTimerZone2()
{
	if(HasAuthority())
	{
		// Start Destroy objects Timer
		GetWorld()->GetTimerManager().SetTimer(
			SpawnTimerZone2, // handle to cancel timer at a later time
			this, // the owning object
			&ACatchDogGameState::OnSpawnTimerZone2End, // function to call on elapsed
			SpawnTimerZone2Value, // float delay until elapsed
			false); // looping

		ActionString = FText::FromString(TEXT("Destroying"));

		CatchDogGameMode->SpawnPowerUps();
		CatchDogGameMode->SpawnPuppies();

		OnSpawnDestroyActionUpdate.Broadcast(ActionString, SpawnTimerZone2Value);
	}

}

void ACatchDogGameState::OnSpawnTimerZone1End()
{
	// Spawn Power-Ups!
	if (HasAuthority())
	{
		CatchDogGameMode->DestroyPowerUps();
		CatchDogGameMode->DestroyPuppiesAndControllers();

		StartSpawnTimerZone2();

		GetWorld()->GetTimerManager().ClearTimer(SpawnTimerZone1);
	}
}

void ACatchDogGameState::OnSpawnTimerZone2End()
{
	// Destroy Power-Ups!
	if(HasAuthority())
	{
		CatchDogGameMode->DestroyPowerUps();
		CatchDogGameMode->DestroyPuppiesAndControllers();

		StartSpawnTimerZone1();

		GetWorld()->GetTimerManager().ClearTimer(SpawnTimerZone2);
	}
}



void ACatchDogGameState::UpdatePreTimer()
{
	SecondsPreGame = GetWorld()->GetTimerManager().GetTimerRemaining(PreGameTimer);

	if (SecondsPreGame < 0)
	{
		SecondsPreGame = 0;
	}

	OnRep_PreGameTimer();
}

void ACatchDogGameState::UpdateTimer()
{
	// Game Timer
	if (HasAuthority())
	{
		Seconds = GetWorld()->GetTimerManager().GetTimerRemaining(GameTimer);

		OnRep_GameTimer();

		if (Seconds > 0)
		{ 
			// Spawn/Destroy timer

			TimeLeft = 999999999; // In case of error, TimeLeft will be 999999999

			// Get Time Left of Spawn/Destroy Timer
			if (GetWorld()->GetTimerManager().GetTimerRemaining(SpawnTimerZone1) >= 0.0f) // Spawn Timer is active
			{
				TimeLeft = (int)GetWorld()->GetTimerManager().GetTimerRemaining(SpawnTimerZone1);
			}
			else if (GetWorld()->GetTimerManager().GetTimerRemaining(SpawnTimerZone2) >= 0.0f) // Destroy Timer is active
			{
				TimeLeft = (int)GetWorld()->GetTimerManager().GetTimerRemaining(SpawnTimerZone2);
			}

			OnRep_SpawnTimer();
		}
	}

}

void ACatchDogGameState::OnRep_GameOver()
{
	ShowEndScreen();
}

void ACatchDogGameState::OnRep_PreGameTimer()
{
	OnPreTimeUpdate.Broadcast(SecondsPreGame); // Broadcast event Update Timer before Game Starts

	if(SecondsPreGame != 0)
	{
		return;
	}

	if (ACatchDogPlayerController* CatchDogPlayerController = Cast<ACatchDogPlayerController>(UGameplayStatics::GetPlayerController(this, 0)))
	{
		CatchDogPlayerController->RemovePreGameTimer();
	}
	
}

void ACatchDogGameState::OnRep_GameTimer()
{
	OnTimeUpdate.Broadcast(Seconds / 60, Seconds % 60); // Broadcast event Update Timer

	if (Seconds <= 0)
	{
		Endgame();
	}
}

void ACatchDogGameState::OnRep_SpawnTimer()
{
	OnSpawnDestroyActionUpdate.Broadcast(ActionString, TimeLeft);
}

void ACatchDogGameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ACatchDogGameState, bGameOver);
	DOREPLIFETIME(ACatchDogGameState, SecondsPreGame);
	DOREPLIFETIME(ACatchDogGameState, Seconds);
	DOREPLIFETIME(ACatchDogGameState, TimeLeft);
	DOREPLIFETIME(ACatchDogGameState, ActionString);
	DOREPLIFETIME(ACatchDogGameState, WinnerScore);
}



void ACatchDogGameState::Endgame()
{
	if(HasAuthority())
	{
		GetWorld()->GetTimerManager().ClearTimer(UpdateUITimer);
		GetWorld()->GetTimerManager().ClearTimer(SpawnTimerZone1);
		GetWorld()->GetTimerManager().ClearTimer(SpawnTimerZone2);
		GetWorld()->GetTimerManager().ClearTimer(GameTimer);

		// Game finished, let's check winner score
		for (int32 index = 0; index < PlayerArray.Num(); index++)
		{
			if (ACatchDogPlayerState* CatchDogPlayerState = Cast<ACatchDogPlayerState>(PlayerArray[index]))
			{
				if (CatchDogPlayerState->GetPlayerScore() >= WinnerScore)
				{
					WinnerScore = CatchDogPlayerState->GetPlayerScore();
				}
			}
		}

		bGameOver = true;

		OnRep_GameOver();
	}
}

void ACatchDogGameState::ShowEndScreen()
{
	/* This OnRepNotify gets the player state and checks its player controller.
	 If it is a local player controller, then the delegate OnGameOver is executed.
	 This delegates has a bound function in the Player Controller Blueprint, which creates the
	 End Screen Widget and sets the final score for each of the players. */

	if (ACatchDogPlayerController* CatchDogPlayerController = Cast<ACatchDogPlayerController>(UGameplayStatics::GetPlayerController(this, 0)))
	{
		CatchDogPlayerController->ShowEndScreen();

		OnGameOver.Broadcast(CatchDogPlayerController->GetPlayerScore()); // Execute Delegate

		// Broadcast delegate to print You won! or You lost! depending on your score.
		if (CatchDogPlayerController->GetPlayerScore() == WinnerScore)
		{
			CatchDogPlayerController->OnIsWinnerController.Broadcast(true);
		}
		else
		{
			CatchDogPlayerController->OnIsWinnerController.Broadcast(false);
		}
	}		
}
