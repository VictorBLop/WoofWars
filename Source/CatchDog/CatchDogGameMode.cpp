// Fill out your copyright notice in the Description page of Project Settings.


#include "CatchDogGameMode.h"
#include "CatchDogCharacter.h"
#include "UObject/ConstructorHelpers.h"
#include "CatchDogGameState.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "CatchDogPlayerController.h"
#include "NavigationData.h"

ACatchDogGameMode::ACatchDogGameMode()
{
	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnBPClass(TEXT("/Game/ThirdPerson/Blueprints/BP_ThirdPersonCharacter"));
	if (PlayerPawnBPClass.Class != NULL)
	{
		DefaultPawnClass = PlayerPawnBPClass.Class;
	}
}

void ACatchDogGameMode::BeginPlay()
{
	Super::BeginPlay();

	CatchDogGameState = GetGameState<ACatchDogGameState>();
}

void ACatchDogGameMode::CatchDogRestartGame()
{
	RestartGame();
}


void ACatchDogGameMode::CatchDogQuitLevel(APlayerController* controller)
{
	if (HasAuthority())
		UKismetSystemLibrary::QuitGame(this, controller, EQuitPreference::Quit, false);
}

void ACatchDogGameMode::GameOver()
{
	// Show end screen for players
	
}

TSubclassOf<APickableActor> ACatchDogGameMode::GetSubclassOfPickableActor(EPickableActorsTypes PickableActorType)
{
	switch (PickableActorType)
	{
	case EPickableActorsTypes::Shield:

		return ShieldClass;

	case EPickableActorsTypes::SpeedBoost:

		return SpeedBoostClass;

	case EPickableActorsTypes::BearTrap:

		return BearTrapPickableClass;

	case EPickableActorsTypes::NetTrap:

		return NetTrapPickableClass;

	case EPickableActorsTypes::Bone:

		return BonePickableClass;

	default:

		return nullptr;
		// Handle default case, if needed
	}
}

TSubclassOf<APickableActor> ACatchDogGameMode::GetSubclassOfPickableActorWithInt(int32 index)
{
	switch (index)
	{
	case 0:

		return ShieldClass;

	case 1:

		return SpeedBoostClass;

	case 2:

		return BearTrapPickableClass;

	case 3:

		return NetTrapPickableClass;

	case 4:

		return BonePickableClass;

	default:

		return nullptr;
		// Handle default case, if needed
	}
}

// Function to get a random element from the enum
EPickableActorsTypes ACatchDogGameMode::GetRandomEnumValue()
{
	// List of enum values in an array
	TArray<EPickableActorsTypes> AllPickableTypes = {
		EPickableActorsTypes::Shield,
		EPickableActorsTypes::SpeedBoost,
		EPickableActorsTypes::BearTrap,
		EPickableActorsTypes::NetTrap,
		EPickableActorsTypes::Bone
	};

	// Generate a random index within the array range
	int32 RandomIndex = FMath::RandRange(0, AllPickableTypes.Num() - 1);

	// Return the enum value at the random index from the array
	return AllPickableTypes[RandomIndex];
}


void ACatchDogGameMode::SpawnPowerUps()
{
	//// Create and assign the AIController to the Puppy
	if (HasAuthority())
	{
		FVector RandomLocation = FVector(0.0f, 0.0f, 0.0f);

		if (GetWorld())
		{
			ACatchDogAIController* CatchDogAIController = GetWorld()->SpawnActor<ACatchDogAIController>(ACatchDogAIController::StaticClass(), FVector::ZeroVector, FRotator(0.0f, 0.0f, 0.0f));

			// Create ZoneOriginsArray, which will be an array to store the Zone1OriginsArray or Zone2OriginsArray
			TArray<FVector> ZoneOriginsArray;

			bIsZone1OrZone2 = !bIsZone1OrZone2;

			if (bIsZone1OrZone2)
			{
				ZoneOriginsArray = Zone1OriginPoints;
			}
			else
			{
				ZoneOriginsArray = Zone2OriginPoints;
			}

			// Spawn Power-Ups!
			for(int index = 0; index < ZoneOriginsArray.Num(); index++)
			{
				for (int i = 0; i < PickableActorsArraySize; i++)
				{
					// Spawn actor
					FActorSpawnParameters SpawnInfo;
					SpawnInfo.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::DontSpawnIfColliding;

					// Since there are 5 different power-ups adn traps, the list of them has indexes from 0 to 4. That's why the index used to get the subclass of pickable actor
					// is i%5, it will always be a value between 0 and 4. This is used to spawn the same amount of each type of power-up/trap.
					TSubclassOf<APickableActor> SubclassOfPickableActor = GetSubclassOfPickableActorWithInt(i%5); 

					if (SubclassOfPickableActor)
					{
						if (CatchDogAIController)
						{
							RandomLocation = CatchDogAIController->GetRandomPointInRadiusWithOrigin(ZoneOriginsArray[index], ZoneMapRadius) + FVector(0.0f, 0.0f, 60.0f); // Implement in Z so that it does not get stuck in the ground
						}

						//UE_LOG(LogTemp, Warning, TEXT("(Zone %i) Power-Up (%i) : %f, %f, %f"), index, i, RandomLocation.X, RandomLocation.Y, RandomLocation.Z);

						APickableActor* PickableActor = GetWorld()->SpawnActor<APickableActor>(
							SubclassOfPickableActor, // Subclass of Pickable Actor
							RandomLocation, // Random point reachable
							FRotator(0.0f, 0.0f, 0.0f), // Rotation
							SpawnInfo); // Spawn Parameters

						if (PickableActor)
						{
							TPickableActorsArray.Add(PickableActor);
						}

					}
				}
			}
		}
	}
}

void ACatchDogGameMode::DestroyPowerUps()
{
	if (HasAuthority())
	{
		// Destroy Power-Ups!
		for (APickableActor* PickableActor : TPickableActorsArray)
		{
			if (PickableActor && PickableActor->GetWorld()) //&& !PickableActor->IsPendingKill())
			{
				PickableActor->Destroy();
			}
			else
			{
				UE_LOG(LogTemp, Error, TEXT("PickableActor is not ValidLowLevel"));
			}
		}

		TPickableActorsArray.Empty(); // Clear the array after destroying the actors
	}

}


void ACatchDogGameMode::SpawnPuppies()
{
	if (HasAuthority())
	{
		// Create ZoneOriginsArray, which will be an array to store the Zone1OriginsArray or Zone2OriginsArray
		TArray<FVector> ZoneOriginsArray;

		if (bIsZone1OrZone2)
		{
			ZoneOriginsArray = Zone1OriginPoints;
			GEngine->AddOnScreenDebugMessage(-1, 20.0f, FColor::Purple, FString::Printf(TEXT("ZONE 1")));
			UE_LOG(LogTemp, Error, TEXT("ZONE 1"));


		}
		else
		{
			ZoneOriginsArray = Zone2OriginPoints;
			GEngine->AddOnScreenDebugMessage(-1, 20.0f, FColor::Purple, FString::Printf(TEXT("ZONE 2")));
			UE_LOG(LogTemp, Warning, TEXT("ZONE 2"));

		}

		// Spawn Puppies!
		for (int index = 0; index < ZoneOriginsArray.Num(); index++)
		{
			for (int i = 0; i < PuppiesArraySize; i++)
			{
				// Spawn actor
				FActorSpawnParameters SpawnInfo;
				SpawnInfo.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

				if (PuppyClass)
				{
					FVector RandomLocation = Origin;

					//// Create and assign the AIController to the Puppy
					if (GetWorld())
					{
						ACatchDogAIController* CatchDogAIController = GetWorld()->SpawnActor<ACatchDogAIController>(ACatchDogAIController::StaticClass(), FVector::ZeroVector, FRotator(0.0f, 0.0f, 0.0f));

						if (CatchDogAIController)
						{
							RandomLocation = CatchDogAIController->GetRandomPointInRadiusWithOrigin(ZoneOriginsArray[index], ZoneMapRadius); // Generate reachable random point with AIController

							if (RandomLocation == ZoneOriginsArray[index])
							{
								UE_LOG(LogTemp, Error, TEXT("Something did not work!!!!! RandomPointWithOrigin did not work. (GameMode)"));
							}

							UE_LOG(LogTemp, Warning, TEXT("(Zone %i) - Puppy (%i) : %f, %f, %f (GameMode)"), index, i, RandomLocation.X, RandomLocation.Y, RandomLocation.Z);
						}

						// Spawn Puppy in position received previously
						APuppy* PuppyActor = GetWorld()->SpawnActor<APuppy>(
							PuppyClass,
							RandomLocation,
							FRotator(0.0f, 0.0f, 0.0f),
							SpawnInfo);

						if (PuppyActor)
						{
							// Check if the AIController was successfully spawned
							if (CatchDogAIController)
							{
								// Possess the Puppy with the AIController
								CatchDogAIController->Possess(PuppyActor);
								PuppyActor->AIControllerReference = CatchDogAIController;
							}
							else
							{
								UE_LOG(LogTemp, Error, TEXT("CatchDogAIController is nullptr"));
							}

							// Activate the movement component
							PuppyActor->GetCharacterMovement()->Activate();

							TPuppiesArray.Add(PuppyActor);
							TPuppiesAIControllersArray.Add(CatchDogAIController);
						}
						else
						{
							UE_LOG(LogTemp, Error, TEXT("Puppy is nullptr"));
						}
					}
					else
					{
						UE_LOG(LogTemp, Error, TEXT("GetWorld is nullptr"));
					}
				}
			}
		}
	}
}


void ACatchDogGameMode::DestroyPuppiesAndControllers()
{
	if (HasAuthority())
	{
		// Destroy Puppies!
		for (APuppy* Puppy : TPuppiesArray)
		{
			if (Puppy && Puppy->GetWorld())
			{
				Puppy->Destroy();
			}
			else
			{
				UE_LOG(LogTemp, Error, TEXT("Puppy is not ValidLowLevel"));
			}
		}

		TPuppiesArray.Empty(); // Clear the array after destroying the actors

		// Destroy AI Controllers
		for (ACatchDogAIController* AIController : TPuppiesAIControllersArray)
		{
			if (AIController && AIController->GetWorld()) 
			{
				AIController->Destroy();
			}
			else
			{
				UE_LOG(LogTemp, Error, TEXT("AIController is not ValidLowLevel"));
			}
		}

		TPuppiesAIControllersArray.Empty(); // Clear the array after destroying the actors

	}

}