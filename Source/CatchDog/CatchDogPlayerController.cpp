// Fill out your copyright notice in the Description page of Project Settings.


#include "CatchDogPlayerController.h"
#include "CatchDogPlayerState.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/Engine.h"
#include "Net/UnrealNetwork.h"
#include "CatchDogCharacter.h"
#include "CatchDogGameState.h"
#include "CatchDogGameMode.h"
#include "Blueprint/WidgetLayoutLibrary.h"
#include "EnhancedInputComponent.h"
#include "InputAction.h"
#include "CatchDogHUD.h"

void ACatchDogPlayerController::BeginPlay()
{
	Super::BeginPlay();

	SetInputMode(FInputModeGameOnly()); // Set Game Only as Input Mode

	SetShowMouseCursor(false);

	if (HasAuthority())
	{
		OnRep_PlayerState();
	}
}


void ACatchDogPlayerController::OnRep_PlayerState()
{
	Super::OnRep_PlayerState();

	if (PreGameWidgetClass && PlayerUIWidgetClass && IsLocalController())
	{
		PlayerUIWidget = CreateWidget(this, PlayerUIWidgetClass);

		PlayerUIWidget->AddToViewport();

		PreGameWidget = CreateWidget(this, PreGameWidgetClass);

		PreGameWidget->AddToViewport();
	}

	if (ACatchDogPlayerState* playerState = Cast<ACatchDogPlayerState>(PlayerState))
	{
		OnPlayerStateReady.Broadcast(playerState);
		playerState->InitializePlayerState();
	}
}

void ACatchDogPlayerController::RemovePreGameTimer()
{
	if (PreGameWidget && IsLocalController())
	{
		PreGameWidget->RemoveFromParent();

		PreGameWidget = nullptr;
	}
}

void ACatchDogPlayerController::ShowEndScreen()
{
	// This function is used to create the End Screen Widget and add it to the viewport.
	// For that, it is necessary to remove the existent widgets on screen and add the new one.
	//  In addition, the input mode and the mouse cursor are modified.

	if(IsLocalController())
	{
		if (PlayerUIWidget)
		{
			//PlayerUIWidget->RemoveFromParent();
			//PlayerUIWidget = nullptr;

			UWidgetLayoutLibrary::RemoveAllWidgets(this);
		}

		if (EndgameWidgetClass)
		{
			PlayerUIWidget = CreateWidget(this, EndgameWidgetClass);

			PlayerUIWidget->AddToViewport();

			SetInputMode(FInputModeUIOnly());

			SetShowMouseCursor(true);
		}
	}
}

void ACatchDogPlayerController::RestartGame() 
{
	// This function is used to restart the game from the client or the server.
	// If it is the server, just call RestartGame from Game Mode.
	// if it is client, then call the RPC to be executed on the server, which will
	// call the same function as if it was the server (Restart Game).

	// This function is called from the Victory/Defeat Widget, from the 
	// Restart Game BUTTON. This way the RPC can be used because the Player Controller
	// owns the widget, which is the object over which the RPC is being called.

	ACatchDogGameMode* CatchDogGameMode = Cast<ACatchDogGameMode>(UGameplayStatics::GetGameMode(GetWorld()));

	if (HasAuthority()) // Restart Game in server
	{
		if (CatchDogGameMode)
		{
			CatchDogGameMode->CatchDogRestartGame();
		}
	}
	else // Restart game from client
	{
		if (IsLocalController())
		{
			Server_RestartGame();
		}
	}
}

void ACatchDogPlayerController::Server_RestartGame_Implementation()
{
	// Server RPC to restart the game from the client.

	ACatchDogGameMode* CatchDogGameMode = Cast<ACatchDogGameMode>(UGameplayStatics::GetGameMode(GetWorld()));

	if (CatchDogGameMode)
	{
		CatchDogGameMode->CatchDogRestartGame();
	}
}

void ACatchDogPlayerController::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
}



void ACatchDogPlayerController::PlayerScored(int32 value)
{
	CatchDogPlayerState = Cast<ACatchDogPlayerState>(PlayerState);

	if (CatchDogPlayerState)
	{
		CatchDogPlayerState->PlayerScored(value);
	}
}

bool ACatchDogPlayerController::GetShield()
{
	CatchDogPlayerState = Cast<ACatchDogPlayerState>(PlayerState);


	if (CatchDogPlayerState->IsValidLowLevel())
	{
		return CatchDogPlayerState->GetShield();
	}
	else
	{
		return false;
	}
}

void ACatchDogPlayerController::ActivateShield(bool ShieldMode)
{
	CatchDogPlayerState = Cast<ACatchDogPlayerState>(PlayerState);

	if (CatchDogPlayerState)
	{
		CatchDogPlayerState->ActivateShield(ShieldMode);
	}
}

int ACatchDogPlayerController::GetInventoryNumberOfItems()
{
	CatchDogPlayerState = Cast<ACatchDogPlayerState>(PlayerState);

	if (CatchDogPlayerState)
	{
		return CatchDogPlayerState->GetInventoryNumberOfItems();
	}
	else
	{
		return 0;
	}

}

EItemType ACatchDogPlayerController::GetItemTypeInInventory()
{
	CatchDogPlayerState = Cast<ACatchDogPlayerState>(PlayerState);

	if (CatchDogPlayerState)
	{
		return CatchDogPlayerState->GetItemTypeInInventory();
	}
	else
	{
		return EItemType::None;
	}


	
}

void ACatchDogPlayerController::AddItemToInventory(EItemType ItemType)
{
	CatchDogPlayerState = Cast<ACatchDogPlayerState>(PlayerState);

	if (CatchDogPlayerState)
	{
		CatchDogPlayerState->AddItemToInventory(ItemType);
	}

}

void ACatchDogPlayerController::RemoveItemFromInventory()
{
	CatchDogPlayerState = Cast<ACatchDogPlayerState>(PlayerState);

	if (CatchDogPlayerState)
	{
		CatchDogPlayerState->RemoveItemFromInventory();
	}
}

int32 ACatchDogPlayerController::GetPlayerScore()
{
	CatchDogPlayerState = Cast<ACatchDogPlayerState>(PlayerState);

	if (CatchDogPlayerState)
	{
		return CatchDogPlayerState->GetPlayerScore();
	}
	else
	{
		return -1;
	}
}


