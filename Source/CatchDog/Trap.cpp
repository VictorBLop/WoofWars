// Fill out your copyright notice in the Description page of Project Settings.


#include "Trap.h"
#include "Engine/Engine.h"
#include "Net/UnrealNetwork.h"

// Sets default values
ATrap::ATrap()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	ActorMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Actor Mesh"));
	ActorMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	ActorMesh->SetMobility(EComponentMobility::Movable);
	SetRootComponent(ActorMesh);

	// Initialize sphere collider
	BoxCollider = CreateDefaultSubobject<UBoxCollider>(TEXT("Box Collider"));
	BoxCollider->SetupAttachment(RootComponent);
	BoxCollider->SetCollisionObjectType(ECollisionChannel::ECC_Pawn); // Or use ECC_WorldDynamic or ECC_Pawn, etc.
	BoxCollider->SetCollisionResponseToChannel(ECollisionChannel::ECC_WorldStatic, ECollisionResponse::ECR_Ignore);
	BoxCollider->SetCollisionResponseToChannel(ECollisionChannel::ECC_WorldDynamic, ECollisionResponse::ECR_Ignore);

	bReplicates = true;
}

// Called when the game starts or when spawned
void ATrap::BeginPlay()
{
	Super::BeginPlay();
	
}

void ATrap::OnBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	//OnOverlap
}

void ATrap::OnEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	//OnEndOverlap
}

void ATrap::OnTimerEnd()
{
	// do stuff when timer ends
}
