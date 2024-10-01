// Fill out your copyright notice in the Description page of Project Settings.


#include "PickableActor.h"
#include "Components/TextBlock.h"
#include "CatchDogCharacter.h"
#include "Engine/Engine.h"
#include "Net/UnrealNetwork.h"

// Sets default values
APickableActor::APickableActor()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	ActorMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Actor Mesh"));
	ActorMesh->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	ActorMesh->SetCollisionObjectType(ECollisionChannel::ECC_WorldStatic);
	ActorMesh->SetMobility(EComponentMobility::Static);
	SetRootComponent(ActorMesh);

	bReplicates = true;
}

// Called when the game starts or when spawned
void APickableActor::BeginPlay()
{
	Super::BeginPlay();

}

