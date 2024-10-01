// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "CatchDogEnums.h"
#include "CatchDogPlayerState.generated.h"

class ACatchDogGameMode;

// Now define the struct that uses the enum

USTRUCT(BlueprintType) struct FInventory
{
	GENERATED_BODY()

	// Always make USTRUCT variables into UPROPERTY()
	// any non-UPROPERTY() struct vars are not replicated

	// So to simplify your life for later debugging, always use UPROPERTY()
	UPROPERTY()
	int32 NumberOfItems = 0;

	UPROPERTY()
	EItemType ItemType = EItemType::None; // item type of inventory

	FInventory()
	{
		NumberOfItems = 0;
		ItemType = EItemType::None;
	}
};


DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnScoreUpdate, int, Score);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnInventoryUpdate, int, NumberOfItems, FText, ItemType);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnShieldStateUpdate, FText, State, FLinearColor, Color);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPlayerStateInitialized, bool, Active);

/**
 * 
 */
UCLASS()
class CATCHDOG_API ACatchDogPlayerState : public APlayerState
{
	GENERATED_BODY()
	
public:

	ACatchDogPlayerState();

	UFUNCTION(BlueprintCallable)
	int32 GetPlayerScore();

	UPROPERTY(BlueprintAssignable)
	FOnScoreUpdate OnScoreUpdate;

	UPROPERTY(BlueprintAssignable)
	FOnInventoryUpdate OnInventoryUpdate;

	UPROPERTY(BlueprintAssignable)
	FOnShieldStateUpdate OnShieldStateUpdate;

	UPROPERTY(BlueprintAssignable)
	FOnPlayerStateInitialized OnPlayerStateInitialized;

	UFUNCTION(BlueprintCallable)
	void PlayerScored(int32 value);

	// Shield related elements
	bool GetShield();

	void ActivateShield(bool ShieldMode);

	// Inventory related elements

	FString GetStringFromEnum(EItemType CustomItemType);

	void RemoveItemFromInventory();

	void AddItemToInventory(EItemType CustomItemType);

	int GetInventoryNumberOfItems();

	EItemType GetItemTypeInInventory();

	void InitializePlayerState();

protected:

	virtual void BeginPlay() override;

	virtual void GetLifetimeReplicatedProps(TArray< FLifetimeProperty >& OutLifetimeProps) const override;

	ACatchDogGameMode* CatchDogGameMode = nullptr;

	UPROPERTY(ReplicatedUsing = OnRep_PlayerScore)
	int32 PlayerScore = 0;

	UFUNCTION()
	void OnRep_PlayerScore();

	UFUNCTION()
	void OnRep_Inventory();

	UFUNCTION()
	void OnRep_Shield();

	UPROPERTY(ReplicatedUsing = OnRep_Shield)
	bool Shield = false;

	FLinearColor ShieldTextColor;

	FText ShieldText;

	UPROPERTY(ReplicatedUsing = OnRep_Inventory)
	FInventory PlayerInventory;
};
