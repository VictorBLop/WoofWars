// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Trap.h"
#include "SlipperySurface.generated.h"

/**
 * 
 */
UCLASS()
class CATCHDOG_API ASlipperySurface : public ATrap
{
	GENERATED_BODY()

public:

	ASlipperySurface();

	virtual void BeginPlay() override;

	virtual void OnBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult) override;

	virtual void OnEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex) override;

protected:

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Slippery Surface")
	float BrakingDecelerationWalking = 2000.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Slippery Surface")
	float GroundFriction = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Slippery Surface")
	bool ShowPlane = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Slippery Surface")
	float PlaneSize = 0.0f;
	
};
