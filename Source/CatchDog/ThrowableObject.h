// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SphereCollider.h"
#include "CatchDogEnums.h"
#include "ThrowableObject.generated.h"

UCLASS()
class CATCHDOG_API AThrowableObject : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AThrowableObject();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	// Sphere Collider
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Mesh, meta = (AllowPrivateAccess = "true"))
	USphereComponent* SphereCollider = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Mesh, meta = (AllowPrivateAccess = "true"))
	UStaticMeshComponent* ActorMesh = nullptr;

	UPROPERTY(EditDefaultsOnly, Category = "Traps To Spawn On Collision")
	TSubclassOf<AActor> BearTrap;

	UPROPERTY(EditDefaultsOnly, Category = "Traps To Spawn On Collision")
	TSubclassOf<AActor> NetTrap;

	EItemType ItemType = EItemType::None;

public:	
	void SetItemType(EItemType ItemTypeToSpawn);

	USphereComponent* GetSphereCollider() const { return SphereCollider; }

	UFUNCTION()
	void OnHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);

};
