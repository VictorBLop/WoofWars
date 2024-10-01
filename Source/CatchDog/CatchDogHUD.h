// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "CatchDogHUD.generated.h"

class UUserWidget;

/**
 * 
 */
UCLASS()
class CATCHDOG_API ACatchDogHUD : public AHUD
{
	GENERATED_BODY()
	

public:

	void TogglePauseMenu();

protected:

	ACatchDogHUD();

	virtual void BeginPlay() override;

	UPROPERTY(EditAnywhere, Category = "Pause Menu")
	TSubclassOf<UUserWidget> PauseMenuWidgetClass;

	UUserWidget* PauseMenuWidget = nullptr;
};
