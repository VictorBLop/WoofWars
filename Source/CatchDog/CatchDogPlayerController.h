// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "CatchDogEnums.h"
#include "Blueprint/UserWidget.h" 
#include "CatchDogPlayerController.generated.h"

class ACatchDogPlayerState;
class UInputAction;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPlayerStateReady, ACatchDogPlayerState*, PlayerState);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnIsWinnerController, bool, Active);


/**
 * 
 */
UCLASS()
class CATCHDOG_API ACatchDogPlayerController : public APlayerController
{
	GENERATED_BODY()

public:

	UPROPERTY(BlueprintAssignable)
	FOnPlayerStateReady OnPlayerStateReady;

	UPROPERTY(BlueprintAssignable)
	FOnIsWinnerController OnIsWinnerController;

	void PlayerScored(int32 value);

	bool GetShield();

	void ActivateShield(bool ShieldMode);

	int GetInventoryNumberOfItems();

	EItemType GetItemTypeInInventory();

	void AddItemToInventory(EItemType ItemType);
	void RemoveItemFromInventory();

	int32 GetPlayerScore();

	void RemovePreGameTimer();

	UFUNCTION(BlueprintCallable)
	void RestartGame();

	UFUNCTION(Server, Reliable)
	void Server_RestartGame();
	
	UFUNCTION(BlueprintCallable)
	void ShowEndScreen();

protected:

	UPROPERTY(BlueprintReadOnly)
	UUserWidget* PlayerUIWidget = nullptr;

	UPROPERTY(BlueprintReadOnly)
	UUserWidget* PreGameWidget = nullptr;

	virtual void BeginPlay() override;

	void OnRep_PlayerState() override;

	UPROPERTY(EditAnywhere, Category = "UI")
	TSubclassOf<UUserWidget> MainMenuWidgetClass = nullptr;

	UPROPERTY(EditAnywhere, Category = "UI")
	TSubclassOf<UUserWidget> PreGameWidgetClass = nullptr;

	UPROPERTY(EditAnywhere, Category = "UI")
	TSubclassOf<UUserWidget> PlayerUIWidgetClass = nullptr;

	UPROPERTY(EditAnywhere, Category = "UI")
	TSubclassOf<UUserWidget> EndgameWidgetClass = nullptr;

	virtual void GetLifetimeReplicatedProps(TArray< FLifetimeProperty >& OutLifetimeProps) const override;

	//UPROPERTY(Replicated)
	ACatchDogPlayerState* CatchDogPlayerState = nullptr;

};
