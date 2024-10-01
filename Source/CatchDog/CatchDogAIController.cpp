// Fill out your copyright notice in the Description page of Project Settings.


#include "CatchDogAIController.h"

void ACatchDogAIController::OnPossess(APawn* InPawn)
{   
    Super::OnPossess(InPawn);

    DelayToSpawnAfterPossess += 0.1f;

    // Start Start Moving Timer
    GetWorld()->GetTimerManager().SetTimer(
        SpawnDelayTimer, // handle to cancel timer at a later time
        this, // the owning object
        &ACatchDogAIController::StartMoving, // function to call on elapsed
        DelayToSpawnAfterPossess, // float delay until elapsed
        false); // looping
}

void ACatchDogAIController::StartMoving()
{
    // Call MoveTo again to start moving towards the new target
    FVector TargetLocation = GetRandomPointInRadius();

    //GEngine->AddOnScreenDebugMessage(-1, 60.0f, FColor::Red, FString::Printf(TEXT("TARGET LOCATION IS %f, %f, %f"), TargetLocation.X, TargetLocation.Y, TargetLocation.Z));

    // Spawn actor

    if (TargetLocation != FVector(0.0f, 0.0f, 0.0f))
    {
        MoveToTarget(TargetLocation);
    }

    GetWorld()->GetTimerManager().ClearTimer(SpawnDelayTimer); // clear timer
}
    


void ACatchDogAIController::MoveToTarget(FVector TargetLocation)
{
    
    // Assuming you have a reference to the AIController that controls this character

    FAIMoveRequest MoveRequest;
    MoveRequest.SetGoalLocation(TargetLocation);
    MoveRequest.SetAcceptanceRadius(200.0f); // Radius within which the goal is considered reached

    FNavPathSharedPtr OutPath; // The pointer to store the generated path, if needed

    // Call MoveTo function of the AIController to instruct the character to move
    FPathFollowingRequestResult Result = MoveTo(MoveRequest, &OutPath);

    //EPathFollowingRequestResult::Type Result = AIController->MoveToLocation(TargetLocation, 50.0f, false, true);

    //// Get the path following component from the AIController
    UPathFollowingComponent* PathFollowingComp = GetPathFollowingComponent();

    if (PathFollowingComp)
    {
        // Check the result of the movement request
        if (Result.Code == EPathFollowingRequestResult::RequestSuccessful)
        {
            // Bind your OnMoveCompleted function to the OnRequestFinished delegate
            PathFollowingComp->OnRequestFinished.Clear();
            PathFollowingComp->OnRequestFinished.AddUObject(this, &ACatchDogAIController::OnMoveCompleted);

            // The request was successful, the character will move towards the destination
        }
        else
        {
            // Handle any failure in the movement request
            //GEngine->AddOnScreenDebugMessage(-1, 10.0f, FColor::Red, FString::Printf(TEXT("ERROR!!! NOT MOVING TO TARGET!")));
        }
    }
}

// Somewhere in your code, where you want to use the function
FVector ACatchDogAIController::GetRandomPointInRadius()
{
    FVector Origin = GetPawn()->GetActorLocation(); // Get the location from where you want to find a random point

    UNavigationSystemV1* NavSystem = UNavigationSystemV1::GetCurrent(GetWorld()); // Get the Navigation System instance

    FVector ReturnVector = { 0.0f, 0.0f, 0.0f };

    if (NavSystem)
    {
        FNavLocation RandomPoint;

        if (NavSystem->GetRandomReachablePointInRadius(Origin, RadiusToFindRandomTarget, RandomPoint)) // Set the radius within which you want to find a random point GetRadiusToFindRandomTarget()
        {
            // 'RandomPoint' now contains the found reachable point
            ReturnVector = RandomPoint.Location;

        }
        else
        {
            //GEngine->AddOnScreenDebugMessage(-1, 10.0f, FColor::Red, FString::Printf(TEXT("ERROR: DIDNT GET RANDOM POINT IN RADIUS!")));
        }
    }

    return ReturnVector;
}

FVector ACatchDogAIController::GetRandomPointInRadiusWithOrigin(FVector Origin, float RadiusToPoint)
{
    UNavigationSystemV1* NavSystem = UNavigationSystemV1::GetCurrent(GetWorld()); // Get the Navigation System instance

    FVector ReturnVector = Origin;

    if (NavSystem)
    {
        FNavLocation RandomPoint;

        if (NavSystem->GetRandomReachablePointInRadius(Origin, RadiusToPoint, RandomPoint)) // Set the radius within which you want to find a random point GetRadiusToFindRandomTarget()
        {
            // 'RandomPoint' now contains the found reachable point
            ReturnVector = RandomPoint.Location;
        }
        else
        {
            UE_LOG(LogTemp, Error, TEXT("NavSystem->GetRandomReachablePointInRadius did not work (AIController)"));
        }
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("NavSystem is not correct"));
    }

    return ReturnVector;
}

void ACatchDogAIController::OnMoveCompleted(FAIRequestID RequestID, const FPathFollowingResult& Result)
{

    if (Result.IsSuccess())
    {
        // Set delay to move again
        DelayToSpawnAfterPossess = 1.0f;

        // Start Start Moving Timer
        GetWorld()->GetTimerManager().SetTimer(
            SpawnDelayTimer, // handle to cancel timer at a later time
            this, // the owning object
            &ACatchDogAIController::StartMoving, // function to call on elapsed
            DelayToSpawnAfterPossess, // float delay until elapsed
            false); // looping

    }
    else if (Result.IsFailure())
    {
        //GEngine->AddOnScreenDebugMessage(-1, 10.0f, FColor::Purple, FString::Printf(TEXT("FAILURE!!! NOT REACHED THE TARGET!")));
    }

    else if (Result.IsInterrupted())
    {
        //GEngine->AddOnScreenDebugMessage(-1, 10.0f, FColor::Yellow, FString::Printf(TEXT("INTERRUPTED!!! NOT REACHED THE TARGET!")));
    }
}