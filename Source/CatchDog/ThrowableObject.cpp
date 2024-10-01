// Fill out your copyright notice in the Description page of Project Settings.


#include "ThrowableObject.h"
#include "BearTrap.h"
#include "NetTrap.h"
#include "Bone.h"
#include "CatchDogCharacter.h"
#include "CatchDogPlayerState.h"

// Sets default values
AThrowableObject::AThrowableObject()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	// Initialize sphere collider
	SphereCollider = CreateDefaultSubobject<USphereCollider>(TEXT("Sphere Collider"));
	SphereCollider->SetSimulatePhysics(true);
	SphereCollider->SetEnableGravity(true);
	SphereCollider->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics); // Enable collision for queries and physics.
	SphereCollider->SetCollisionResponseToAllChannels(ECR_Block); // Set collision response to block all channels.
	SetRootComponent(SphereCollider);

	ActorMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Actor Mesh"));
	ActorMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	ActorMesh->SetupAttachment(RootComponent);

	bReplicates = true;
	SetReplicateMovement(true);
}

// Called when the game starts or when spawned
void AThrowableObject::BeginPlay()
{
	Super::BeginPlay();

	SphereCollider->OnComponentHit.AddDynamic(this, &AThrowableObject::OnHit);
}

void AThrowableObject::OnHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	FVector ImpactNormal = Hit.ImpactNormal;

	// Calculate the rotation based on the wall's normal
	FQuat RotationQuat = FQuat::FindBetweenNormals(FVector::UpVector, ImpactNormal);
	FRotator SpawnRotation = RotationQuat.Rotator();

	// Spawn actor
	FActorSpawnParameters SpawnInfo;
	SpawnInfo.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	switch (ItemType)
	{
	case EItemType::BearTrap: // Bear Trap slows you down during 5 seconds.

		GetWorld()->SpawnActor<ABearTrap>(BearTrap, GetActorLocation(), SpawnRotation, SpawnInfo);

		break;

	case EItemType::NetTrap: // Net trap completely avoids your movement during 5 seconds.

		GetWorld()->SpawnActor<ANetTrap>(NetTrap, GetActorLocation(), SpawnRotation, SpawnInfo);

		break;

	case EItemType::Bone: // Bones collide with characters and remove points from them.

		// check collision with character
		if (ACatchDogCharacter* CatchDogCharacter = Cast<ACatchDogCharacter>(OtherActor))
		{
			if (!CatchDogCharacter->GetPlayerState())
			{
				return;
			}

			if (ACatchDogPlayerState* CatchDogPlayerState = Cast<ACatchDogPlayerState>(CatchDogCharacter->GetPlayerState()))
			{
				if (CatchDogPlayerState->GetShield() == false)
				{
					CatchDogPlayerState->PlayerScored(-3);
				}
			}
		}

		break;
	}

	Destroy();
}

void AThrowableObject::SetItemType(EItemType ItemTypeToSpawn)
{
	ItemType = ItemTypeToSpawn;
}



