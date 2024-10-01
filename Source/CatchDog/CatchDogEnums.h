// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

/**
 * class CATCHDOG_API CatchDogEnums
{
public:
	CatchDogEnums();
	~CatchDogEnums();
};
 */

UENUM(BlueprintType)
enum class EItemType : uint8
{
    Shield UMETA(DisplayName = "Shield"),
    SpeedBoost UMETA(DisplayName = "SpeedBoost"),
    BearTrap UMETA(DisplayName = "BearTrap"),
    NetTrap UMETA(DisplayName = "NetTrap"),
    Bone UMETA(DisplayName = "Bone"),
    Puppy UMETA(DisplayName = "Puppy"),
    None UMETA(DisplayName = "None")
};


UENUM(BlueprintType)
enum class EPickableActorsTypes : uint8
{
    Shield UMETA(DisplayName = "Shield"),
    SpeedBoost UMETA(DisplayName = "SpeedBoost"),
    BearTrap UMETA(DisplayName = "BearTrap"),
    NetTrap UMETA(DisplayName = "NetTrap"),
    Bone UMETA(DisplayName = "Bone"),
};