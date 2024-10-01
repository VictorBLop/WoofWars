// Fill out your copyright notice in the Description page of Project Settings.


#include "SlipperySurface.h"
#include "CatchDogCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"

ASlipperySurface::ASlipperySurface()
{
	ActorMesh->SetVisibility(ShowPlane);

	BoxCollider->SetRelativeScale3D({ PlaneSize, PlaneSize, 1.0f });
}

void ASlipperySurface::BeginPlay()
{
	Super::BeginPlay();

	BoxCollider->OnComponentBeginOverlap.AddDynamic(this, &ThisClass::OnBeginOverlap);
	BoxCollider->OnComponentEndOverlap.AddDynamic(this, &ThisClass::OnEndOverlap);
}

void ASlipperySurface::OnBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (OtherActor->IsA<ACatchDogCharacter>())
	{
		CatchDogCharacter = Cast<ACatchDogCharacter>(OtherActor);

		if (CatchDogCharacter)
		{
			CatchDogCharacter->GetCharacterMovement()->GroundFriction = GroundFriction;
			CatchDogCharacter->GetCharacterMovement()->BrakingDecelerationWalking = BrakingDecelerationWalking;
		}
	}
}

void ASlipperySurface::OnEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (OtherActor->IsA<ACatchDogCharacter>())
	{
		if (CatchDogCharacter)
		{
			CatchDogCharacter->GetCharacterMovement()->GroundFriction = 8.0f;
			CatchDogCharacter->GetCharacterMovement()->BrakingDecelerationWalking = 2000.0f;
		}

		CatchDogCharacter = nullptr;
	}
}