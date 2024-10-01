// Fill out your copyright notice in the Description page of Project Settings.


#include "CatchDogPlayerState.h"
#include "CatchDogGameMode.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/Engine.h"
#include "Net/UnrealNetwork.h"
#include "CatchDogGameState.h"
#include "CatchDogPlayerController.h"
#include "Blueprint/UserWidget.h" 

ACatchDogPlayerState::ACatchDogPlayerState()
{
	PrimaryActorTick.bCanEverTick = false;

	bReplicates = true;
}
void ACatchDogPlayerState::BeginPlay()
{
	Super::BeginPlay();
}

void ACatchDogPlayerState::InitializePlayerState()
{
	if (GetWorld())
		CatchDogGameMode = Cast<ACatchDogGameMode>(UGameplayStatics::GetGameMode(GetWorld()));

	OnScoreUpdate.Broadcast(PlayerScore);

	OnInventoryUpdate.Broadcast(0, FText::FromString(TEXT("None")));

	OnShieldStateUpdate.Broadcast(FText::FromString(TEXT("OFF")), FLinearColor(1.0f, 0.0f, 0.0f, 1.0f));

}

void ACatchDogPlayerState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ACatchDogPlayerState, PlayerInventory);
	DOREPLIFETIME(ACatchDogPlayerState, PlayerScore);
	DOREPLIFETIME(ACatchDogPlayerState, Shield);
}

void ACatchDogPlayerState::OnRep_PlayerScore()
{
	OnScoreUpdate.Broadcast(PlayerScore);
}

void ACatchDogPlayerState::OnRep_Inventory()
{
	OnInventoryUpdate.Broadcast(PlayerInventory.NumberOfItems, FText::FromString(GetStringFromEnum(PlayerInventory.ItemType)));
}

void ACatchDogPlayerState::OnRep_Shield()
{
	if (Shield)
	{
		ShieldTextColor = { 0.0f, 1.0f, 0.0f, 1.0f }; // GREEN
		ShieldText = FText::FromString(TEXT("ON"));
	}
	else
	{
		ShieldTextColor = { 1.0f, 0.0f, 0.0f, 1.0f }; // RED
		ShieldText = FText::FromString(TEXT("OFF"));
	}

	OnShieldStateUpdate.Broadcast(ShieldText, ShieldTextColor);
}

int32 ACatchDogPlayerState::GetPlayerScore()
{
	return PlayerScore;
}

void ACatchDogPlayerState::PlayerScored(int32 value)
{
	// Update player score
	PlayerScore += value;

	if (PlayerScore < 0) // PlayerScore minimum is zero.
	{
		PlayerScore = 0;
	}

	// Broadcast PlayerScore to the UI
	OnRep_PlayerScore();
}

bool ACatchDogPlayerState::GetShield()
{
	return Shield;
}

void ACatchDogPlayerState::ActivateShield(bool ShieldMode)
{
	Shield = ShieldMode;
	
	OnRep_Shield();
}

FString ACatchDogPlayerState::GetStringFromEnum(EItemType CustomItemType)
{
	FString ReturnString = "Error";

	if (CustomItemType == EItemType::BearTrap)
	{
		ReturnString = "Bear Trap";
	}
	else if (CustomItemType == EItemType::NetTrap)
	{
		ReturnString = "Net Trap";
	}
	else if (CustomItemType == EItemType::Bone)
	{
		ReturnString = "Bone";
	}
	else if (CustomItemType == EItemType::None)
	{
		ReturnString = "None";
	}

	return ReturnString;
}

void ACatchDogPlayerState::RemoveItemFromInventory()
{
	PlayerInventory.NumberOfItems--;

	PlayerInventory.ItemType = EItemType::None;

	OnRep_Inventory();	
}

void ACatchDogPlayerState::AddItemToInventory(EItemType CustomItemType)
{
	if (PlayerInventory.NumberOfItems == 0)
	{
		PlayerInventory.NumberOfItems++;

		PlayerInventory.ItemType = CustomItemType;

		OnRep_Inventory();
	}
}



int ACatchDogPlayerState::GetInventoryNumberOfItems()
{
	return PlayerInventory.NumberOfItems;
}

EItemType ACatchDogPlayerState::GetItemTypeInInventory()
{
	return PlayerInventory.ItemType;
}

