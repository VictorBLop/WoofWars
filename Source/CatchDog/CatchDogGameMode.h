// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameMode.h"
#include "CatchDogEnums.h"
#include "PickableActor.h"
#include "Puppy.h"
#include "Math/UnrealMathUtility.h"
#include "Bone.h"
#include "BearTrapPickable.h"
#include "NetTrapPickable.h"
#include "Shield.h"
#include "SpeedBoost.h"
#include "CatchDogAIController.h"
#include "AIController.h"
#include "NavigationSystem.h"
#include "Navigation/PathFollowingComponent.h"
#include "GameFramework/CharacterMovementComponent.h" // Include if needed
#include "Navigation/PathFollowingComponent.h"
#include "CatchDogGameMode.generated.h"

class ACatchDogGameState;
class ACatchDogPlayerController;

/**
 * 
 */
UCLASS()
class CATCHDOG_API ACatchDogGameMode : public AGameMode
{
	GENERATED_BODY()

public:
	ACatchDogGameMode();

	UFUNCTION()
	void GameOver();

	UPROPERTY(BlueprintReadOnly)
	TArray<APuppy*> TPuppiesArray;

	UPROPERTY(BlueprintReadOnly)
	TArray<ACatchDogAIController*> TPuppiesAIControllersArray;

	UPROPERTY(BlueprintReadOnly)
	TArray<APickableActor*> TPickableActorsArray;

protected:

	virtual void BeginPlay() override;

	UFUNCTION(BlueprintCallable)
	void CatchDogQuitLevel(APlayerController* controller);

	// Pickable Actors related elements

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Timer")
	int32 PickableActorsArraySize = 10;

	
	TSubclassOf<APickableActor> GetSubclassOfPickableActor(EPickableActorsTypes PickableActorType);

	TSubclassOf<APickableActor> GetSubclassOfPickableActorWithInt(int32 index);

	EPickableActorsTypes GetRandomEnumValue();

	UPROPERTY(EditDefaultsOnly, Category = "Pickable Objects")
	TSubclassOf<APickableActor> SpeedBoostClass;

	UPROPERTY(EditDefaultsOnly, Category = "Pickable Objects")
	TSubclassOf<APickableActor> ShieldClass;

	UPROPERTY(EditDefaultsOnly, Category = "Pickable Objects")
	TSubclassOf<APickableActor> BearTrapPickableClass;

	UPROPERTY(EditDefaultsOnly, Category = "Pickable Objects")
	TSubclassOf<APickableActor> NetTrapPickableClass;

	UPROPERTY(EditDefaultsOnly, Category = "Pickable Objects")
	TSubclassOf<APickableActor> BonePickableClass;

	// Puppies related elements

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Puppy Objects")
	int32 PuppiesArraySize = 100;

	UPROPERTY(EditDefaultsOnly, Category = "Puppy Objects")
	TSubclassOf<APuppy> PuppyClass;

	UPROPERTY(EditAnywhere, Category = "Map Origin Point")
	FVector Origin = FVector(3360.0f, -3630.0f, 50.0f); // Center of the map (aprox)

	bool bIsZone1OrZone2 = false;

	UPROPERTY(EditAnywhere, Category = "PowerUps Spawning - Zone 1 Points")
	TArray<FVector> Zone1OriginPoints;

	UPROPERTY(EditAnywhere , Category = "PowerUps Spawning - Zone 2 Points")
	TArray<FVector> Zone2OriginPoints;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Map Radius")
	float MapRadius = 16000.0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Map Radius")
	float ZoneMapRadius = 2000.0;

public:

	UFUNCTION(BlueprintCallable)
	void CatchDogRestartGame();

	// Functions to spawn/destroy elements

	void SpawnPowerUps();

	void DestroyPowerUps();

	void SpawnPuppies();

	void DestroyPuppiesAndControllers();

private:

	ACatchDogGameState* CatchDogGameState = nullptr;

};
