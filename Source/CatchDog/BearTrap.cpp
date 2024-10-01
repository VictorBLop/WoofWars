// Fill out your copyright notice in the Description page of Project Settings.


#include "BearTrap.h"
#include "CatchDogCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Engine/Engine.h"
#include "Net/UnrealNetwork.h"

ABearTrap::ABearTrap()
{
	PrimaryActorTick.bCanEverTick = false;
}

void ABearTrap::BeginPlay()
{
	Super::BeginPlay();

	BoxCollider->OnComponentBeginOverlap.AddDynamic(this, &ThisClass::OnBeginOverlap);
	BoxCollider->OnComponentEndOverlap.AddDynamic(this, &ThisClass::OnEndOverlap);
}

void ABearTrap::OnBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	// begin overlap
	
		if (OtherActor->IsA<ACatchDogCharacter>())
		{
			CatchDogCharacter = Cast<ACatchDogCharacter>(OtherActor); // cast to ACatchDogCharacter

			if (!CatchDogCharacter)
			{
				return;
			}

			if (CatchDogCharacter->GetShield() == false) // Shield deactivated
			{
				if (HasAuthority())
				{
					CatchDogCharacter->SetMaxMinWalkSpeed(ModifiedCharacterVelocity, ModifiedCharacterVelocity); // set new character velocity			

					// Start timer
					GetWorld()->GetTimerManager().SetTimer(
						TrapTimerHandle, // handle to cancel timer at a later time
						this, // the owning object
						&ABearTrap::OnTimerEnd, // function to call on elapsed
						TimeIntervalForTrap, // float delay until elapsed
						false); // looping
				}

				// instead of hiding the object, just move under the map and that would solve the issue.
				SetActorLocation(GetActorLocation() + FVector(0.0f, 0.0f, -500.0f), false);

			}

		}
	
}

void ABearTrap::OnEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	// end overlap
}

void ABearTrap::OnTimerEnd()
{
	if (CatchDogCharacter->GetMaxWalkSpeed() < 500.0f)
	{
		CatchDogCharacter->SetMaxMinWalkSpeed(20.0f, 500.0f); // set new character velocity
	}

	CatchDogCharacter = nullptr;

	Destroy(); // Destroy after effect has passed
}
