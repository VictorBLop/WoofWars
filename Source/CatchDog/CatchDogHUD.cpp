// Fill out your copyright notice in the Description page of Project Settings.


#include "CatchDogHUD.h"
#include "Blueprint/UserWidget.h"

ACatchDogHUD::ACatchDogHUD()
{
	bShowOverlays = true;
}

void ACatchDogHUD::BeginPlay()
{
	// BeginPlay
}

void ACatchDogHUD::TogglePauseMenu()
{
	/* TODO-02: Implement the logic to toggle the PauseMenu here. */
	if (PauseMenuWidgetClass && PauseMenuWidget == nullptr)
	{
		PauseMenuWidget = CreateWidget<UUserWidget>(GetOwningPlayerController(), PauseMenuWidgetClass);
	}

	if(GetWorld() && PauseMenuWidget)
	{
		if (!PauseMenuWidget->IsInViewport())
		{
			GetOwningPlayerController()->SetIgnoreMoveInput(true);
			GetOwningPlayerController()->SetInputMode(FInputModeGameAndUI());
			GetOwningPlayerController()->SetShowMouseCursor(true);

			PauseMenuWidget->AddToViewport();

		}
		else
		{
			GetOwningPlayerController()->SetIgnoreMoveInput(false);
			GetOwningPlayerController()->SetInputMode(FInputModeGameOnly());
			GetOwningPlayerController()->SetShowMouseCursor(false);
		
			PauseMenuWidget->RemoveFromParent();		
		}
	}
}


