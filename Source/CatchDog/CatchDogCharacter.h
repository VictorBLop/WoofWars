// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Logging/LogMacros.h"
#include "SphereCollider.h"
#include "PickableActor.h"
#include "SpeedBoost.h"
#include "Shield.h"
#include "CatchDogEnums.h"
#include "CatchDogCharacter.generated.h"

class USpringArmComponent;
class UCameraComponent;
class UInputMappingContext;
class UInputAction;
class ACatchDogPlayerController;
class AThrowableObject;
struct FInputActionValue;

DECLARE_LOG_CATEGORY_EXTERN(LogTemplateCharacter, Log, All);

UCLASS(config=Game)
class ACatchDogCharacter : public ACharacter
{
	GENERATED_BODY()

	/** Camera boom positioning the camera behind the character */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	USpringArmComponent* CameraBoom;

	/** Follow camera */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	UCameraComponent* FollowCamera;

	/** MappingContext */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputMappingContext* DefaultMappingContext;

	/** Jump Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* JumpAction;

	/** Move Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* MoveAction;

	/** Look Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* LookAction;

	/** Throw Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* ThrowAction;

	/** Aim Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* AimAction;

	/** Adjust Speed Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* AdjustSpeedAction;

	/** Pause Menu Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* PauseGameAction = nullptr;

	///** Increase Projectile Speed Action (only for controller) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* IncreaseSpeedControllerAction;

	///** Increase Projectile Speed Action (only for controller) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* DecreaseSpeedControllerAction;

	// Reference to Player Controller
	ACatchDogPlayerController* CatchDogPlayerController = nullptr;

public:
	ACatchDogCharacter();

	void SetPlayerControllerReference(ACatchDogPlayerController* playerController);

	UFUNCTION()
	void ChangeSkeletalMesh(int32 Index);

protected:

	// APawn interface
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	// To add mapping context
	virtual void BeginPlay();

	UFUNCTION(NetMulticast, Reliable)
	void NetMulticast_SetNewSkeletalMesh(int32 index);

	void PossessedBy(AController* NewController);

	/** Called for movement input */
	void Move(const FInputActionValue& Value);

	/** Called for looking input */
	void Look(const FInputActionValue& Value);

	/** Called for throw object input */
	void ThrowObject(const FInputActionValue& Value);

	/** Called for Aim input */
	void Aim(const FInputActionValue& Value);

	void StopAiming(const FInputActionValue& Value);

	void AdjustSpeed(const FInputActionValue& Value);

	void TogglePauseMenu(const FInputActionValue& Value);

	void DecreaseSpeedController(const FInputActionValue& Value);

	void IncreaseSpeedController(const FInputActionValue& Value);

	void PickUp(AActor* PickableActor);

	/** Called for speed boost input */
	void ActivateItem(EItemType ItemType, AActor* PickableActor);

	void DestroyAndRemoveFromArray(AActor* pickableActor);

protected:

	// Sphere Collider
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Mesh, meta = (AllowPrivateAccess = "true"))
	USphereComponent* SphereCatcher = nullptr;

	// Collider related elements
	UFUNCTION()
	void OnBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	void OnEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	// Child Actor References for Projectile

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Projectile", meta = (AllowPrivateAccess = "true"))
	UChildActorComponent* ProjectileReference = nullptr;

	/* Throwable actors */
	UPROPERTY(EditDefaultsOnly, Category = "Projectile Version of the Objects (Throwable)")
	TSubclassOf<AActor> ThrowableObjectClass;

	UPROPERTY(EditDefaultsOnly, Category = "Projectile Version of the Objects (Throwable)")
	TSubclassOf<AActor> BearTrapThrowableClass;

	UPROPERTY(EditDefaultsOnly, Category = "Projectile Version of the Objects (Throwable)")
	TSubclassOf<AActor> NetTrapThrowableClass;

	UPROPERTY(EditDefaultsOnly, Category = "Projectile Version of the Objects (Throwable)")
	TSubclassOf<AActor> BoneThrowableClass;

	/* Spawnable actors */
	UPROPERTY(EditDefaultsOnly, Category = "Spawnable Version of the Objects (Spawnable)")
	TSubclassOf<AActor> BearTrapSpawnableClass;

	UPROPERTY(EditDefaultsOnly, Category = "Spawnable Version of the Objects (Spawnable)")
	TSubclassOf<AActor> NetTrapSpawnableClass;

	UPROPERTY(EditDefaultsOnly, Category = "Spawnable Version of the Objects (Spawnable)")
	TSubclassOf<AActor> BoneSpawnableClass;

	TArray<AActor*> TrajectoryPoints;

	// Spawn(Throw) and Aiming Trajectory

	UPROPERTY(EditAnywhere, Category = "Aim & Throw Animation")
	UAnimMontage* AimMontage = nullptr;

	UPROPERTY(EditAnywhere, Category = "Aim & Throw Animation")
	UAnimMontage* ThrowMontage = nullptr;

	UFUNCTION(BlueprintCallable)
	void SpawnFromAnimation();

	void SpawnThrowObject(); // Spawn actual throwable object

	UPROPERTY(Replicated)
	AThrowableObject* ThrowableActorSpawned = nullptr;

	UFUNCTION(Server, Reliable)
	void Server_RPCSpawnThrowableActor();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	bool bIsAiming = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	bool bThrow = false;

	void SpawnAimingTrajectory();

	void DestroyTrajectory();
	TSubclassOf<AActor> GetSubclassFromInventoryObject();
	TSubclassOf<AActor> GetSpawnableSubclassFromInventoryObject();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Projectile", meta = (AllowPrivateAccess = "true"))
	float ProjectileSpeed = 0.0f;

	// Speed Boost Related Elements

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = PowerUp)
	float SpeedBoostVelocity = 1200.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = PowerUp)
	float TimeIntervalForSpeedBoost = 5.0f;

	void OnTimerEndSpeedBoost();

	/* Speed elements */

	float CurrentMinAnalogWalkSpeed = 0.0f;

	UPROPERTY(ReplicatedUsing = OnRep_CurrentMaxWalkSpeed)
	float CurrentMaxWalkSpeed = 0.0f;

	UFUNCTION()
	void OnRep_CurrentMaxWalkSpeed();

	UFUNCTION(Server,Reliable)
	void ServerRPC_SetNewWalkSpeed();

	void SetNewWalkSpeed();

	// Shield Related elements

	void OnTimerEndShield();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = PowerUp)
	float TimeIntervalForShield = 10.0f;

	/* Replication */

	virtual void GetLifetimeReplicatedProps(TArray< FLifetimeProperty >& OutLifetimeProps) const override;

	/* Sound Replication */

	// Puppy Sounds

	UPROPERTY(ReplicatedUsing = OnRep_PuppyPlaySound)
	bool bPuppyPlaySound = false;

	UFUNCTION()
	void OnRep_PuppyPlaySound();

	void PlayPuppySound();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sounds - Puppy")
	USoundBase* PuppySoundToPlay;

	// Pick Ups and Traps Sounds

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sounds - Pick Up")
	USoundBase* ShieldSoundToPlay;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sounds - Pick Up")
	USoundBase* SpeedBoostSoundToPlay;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sounds - Traps")
	USoundBase* TrapsSoundToPlay;

	UPROPERTY(ReplicatedUsing = OnRep_ShieldPlaySound)
	bool bShieldPlaySound = false;

	UPROPERTY(ReplicatedUsing = OnRep_SpeedBoostPlaySound)
	bool bSpeedBoostPlaySound = false;

	UPROPERTY(ReplicatedUsing = OnRep_TrapPlaySound)
	bool bTrapPlaySound = false;

	UFUNCTION()
	void OnRep_ShieldPlaySound();

	UFUNCTION()
	void OnRep_SpeedBoostPlaySound();

	UFUNCTION()
	void OnRep_TrapPlaySound();

	/* Math Operation */
	UFUNCTION(BlueprintCallable)
	int32 Remainder(int32 Number1, int32 Number2);

public:

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<USkeletalMesh*> CharacterMeshes;

	// Shield Related elements
	bool GetShield();

	void ActivateShield(bool ShieldMode);

	// Inventory related elements
	 void AddItemToInventory(EItemType ItemType);

	 void SetMaxMinWalkSpeed(float MinWalkSpeed, float MaxWalkSpeed);

	 float GetMaxWalkSpeed();

public:
	/** Returns CameraBoom subobject **/
	FORCEINLINE class USpringArmComponent* GetCameraBoom() const { return CameraBoom; }
	/** Returns FollowCamera subobject **/
	FORCEINLINE class UCameraComponent* GetFollowCamera() const { return FollowCamera; }

private:

	/* Handle to manage the timer */
	FTimerHandle ShieldTimerHandle;

	FTimerHandle SpeedBoostTimerHandle;

};

