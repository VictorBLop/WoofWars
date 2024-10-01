// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "BoxCollider.h"
#include "Trap.generated.h"

class ACatchDogCharacter;

UCLASS()
class CATCHDOG_API ATrap : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ATrap();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Mesh, meta = (AllowPrivateAccess = "true"))
	UStaticMeshComponent* ActorMesh = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Mesh, meta = (AllowPrivateAccess = "true"))
	UBoxCollider* BoxCollider = nullptr;

	UFUNCTION()
	virtual void OnBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
	
	UFUNCTION()
	virtual void OnEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	virtual void OnTimerEnd();

	FTimerHandle TrapTimerHandle;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = PowerUp)
	float ModifiedCharacterVelocity = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = PowerUp)
	float TimeIntervalForTrap = 5.0f;

	ACatchDogCharacter* CatchDogCharacter = nullptr; // Reference to Character

};
