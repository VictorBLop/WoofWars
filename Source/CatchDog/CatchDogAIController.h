// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "NavigationSystem.h"
#include "Navigation/PathFollowingComponent.h"
#include "Math/UnrealMathUtility.h"
#include "GameFramework/CharacterMovementComponent.h" // Include if needed
#include "Navigation/PathFollowingComponent.h"
#include "CatchDogAIController.generated.h"

/**
 * 
 */
UCLASS()
class CATCHDOG_API ACatchDogAIController : public AAIController
{
	GENERATED_BODY()
	
protected:

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Random Movement")
	float RadiusToFindRandomTarget = 1000.0f;

	//virtual void BeginPlay() override;


public:
	// Called to move the object to a target point
	void MoveToTarget(FVector TargetLocation);

	FVector GetRandomPointInRadius();

	FVector GetRandomPointInRadiusWithOrigin(FVector Origin, float RadiusToPoint);

	void OnMoveCompleted(FAIRequestID RequestID, const FPathFollowingResult& Result);

	virtual void OnPossess(APawn* InPawn) override;

protected:

	FTimerHandle SpawnDelayTimer;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Random Movement")
	float DelayToSpawnAfterPossess = 0.5f;

	void StartMoving();

};
