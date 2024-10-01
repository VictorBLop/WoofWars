// Copyright Epic Games, Inc. All Rights Reserved.

#include "CatchDogCharacter.h"
#include "Engine/LocalPlayer.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/Controller.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputActionValue.h"
#include "CatchDogPlayerController.h"
#include "BearTrapPickable.h"
#include "NetTrapPickable.h"
#include "Bone.h"
#include "ThrowableObject.h"
#include "Kismet/KismetMathLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "SphereCollider.h"
#include "Puppy.h"
#include "Engine/Engine.h"
#include "Net/UnrealNetwork.h"
#include "CatchDogHUD.h"
#include "CatchDogGameMode.h"

DEFINE_LOG_CATEGORY(LogTemplateCharacter);

//////////////////////////////////////////////////////////////////////////
// ACatchDogCharacter

ACatchDogCharacter::ACatchDogCharacter()
{
	PrimaryActorTick.bCanEverTick = true; //Debug

	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);
		
	// Don't rotate when the controller rotates. Let that just affect the camera.
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	// Configure character movement
	GetCharacterMovement()->bOrientRotationToMovement = true; // Character moves in the direction of input...	
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 500.0f, 0.0f); // ...at this rotation rate

	// Note: For faster iteration times these variables, and many more, can be tweaked in the Character Blueprint
	// instead of recompiling to adjust them
	GetCharacterMovement()->JumpZVelocity = 700.f;
	GetCharacterMovement()->AirControl = 0.35f;
	GetCharacterMovement()->MaxWalkSpeed = 500.f;
	GetCharacterMovement()->MinAnalogWalkSpeed = 20.f;
	GetCharacterMovement()->BrakingDecelerationWalking = 2000.f;
	GetCharacterMovement()->BrakingDecelerationFalling = 1500.0f;

	// Create a camera boom (pulls in towards the player if there is a collision)
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 400.0f; // The camera follows at this distance behind the character	
	CameraBoom->bUsePawnControlRotation = true; // Rotate the arm based on the controller

	// Create a follow camera
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName); // Attach the camera to the end of the boom and let the boom adjust to match the controller orientation
	FollowCamera->bUsePawnControlRotation = false; // Camera does not rotate relative to arm

	// Initialize sphere collider
	SphereCatcher = CreateDefaultSubobject<USphereCollider>(TEXT("Sphere Catcher"));
	SphereCatcher->SetupAttachment(RootComponent);

	ProjectileReference = CreateDefaultSubobject<UChildActorComponent>(TEXT("Projectile Reference"));
	ProjectileReference->AddRelativeLocation(FVector(80, 0.0f, 70.0f));
	ProjectileReference->SetChildActorClass(ThrowableObjectClass);
	ProjectileReference->SetupAttachment(RootComponent);
	ProjectileSpeed = 800.0f;

	// Note: The skeletal mesh and anim blueprint references on the Mesh component (inherited from Character) 
	// are set in the derived blueprint asset named ThirdPersonCharacter (to avoid direct content references in C++)
}

void ACatchDogCharacter::SetPlayerControllerReference(ACatchDogPlayerController* playerController)
{
	CatchDogPlayerController = playerController;
}


void ACatchDogCharacter::BeginPlay()
{
	// Call the base class  
	Super::BeginPlay();

	//Add Input Mapping Context
	if(ACatchDogPlayerController* playerController = Cast<ACatchDogPlayerController>(Controller))
	{
		CatchDogPlayerController = playerController;

		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(CatchDogPlayerController->GetLocalPlayer()))
		{
			Subsystem->AddMappingContext(DefaultMappingContext, 0);
		}
	}
	
	SphereCatcher->OnComponentBeginOverlap.AddDynamic(this, &ThisClass::OnBeginOverlap);
	SphereCatcher->OnComponentEndOverlap.AddDynamic(this, &ThisClass::OnEndOverlap);

	// Initialize current max and min walk speed
	CurrentMaxWalkSpeed = GetCharacterMovement()->MaxWalkSpeed;
	CurrentMinAnalogWalkSpeed = GetCharacterMovement()->MinAnalogWalkSpeed;
}

void ACatchDogCharacter::ChangeSkeletalMesh(int32 Index)
{
	if (!CharacterMeshes.IsEmpty())
	{
		GetMesh()->SetSkeletalMesh(CharacterMeshes[Index]);
	}

	NetMulticast_SetNewSkeletalMesh(Index);
}
void ACatchDogCharacter::NetMulticast_SetNewSkeletalMesh_Implementation(int32 index)
{
	if (!CharacterMeshes.IsEmpty())
	{
		GetMesh()->SetSkeletalMesh(CharacterMeshes[index]);
	}
}

void ACatchDogCharacter::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);
}

//////////////////////////////////////////////////////////////////////////
// Input

void ACatchDogCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	// Set up action bindings
	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent)) {
		
		// Jumping
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Started, this, &ACharacter::Jump);
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Completed, this, &ACharacter::StopJumping);

		// Moving
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &ACatchDogCharacter::Move);

		// Looking
		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &ACatchDogCharacter::Look);

		//Aim (Trigger)
		EnhancedInputComponent->BindAction(AimAction, ETriggerEvent::Triggered, this, &ACatchDogCharacter::Aim);

		//Aim (Hold)
		EnhancedInputComponent->BindAction(AimAction, ETriggerEvent::Ongoing, this, &ACatchDogCharacter::Aim);

		//Aim (Completed)
		EnhancedInputComponent->BindAction(AimAction, ETriggerEvent::Completed, this, &ACatchDogCharacter::StopAiming);

		//Throw (Fix Camera)
		EnhancedInputComponent->BindAction(ThrowAction, ETriggerEvent::Triggered, this, &ACatchDogCharacter::ThrowObject);

		// Adjust Speed
		EnhancedInputComponent->BindAction(AdjustSpeedAction, ETriggerEvent::Triggered, this, &ACatchDogCharacter::AdjustSpeed);

		// Pause Game
		EnhancedInputComponent->BindAction(PauseGameAction, ETriggerEvent::Triggered, this, &ACatchDogCharacter::TogglePauseMenu);

		// Decrease Speed (Projectile)
		EnhancedInputComponent->BindAction(DecreaseSpeedControllerAction, ETriggerEvent::Triggered, this, &ACatchDogCharacter::DecreaseSpeedController);

		// Increase Speed (Projectile)
		EnhancedInputComponent->BindAction(IncreaseSpeedControllerAction, ETriggerEvent::Triggered, this, &ACatchDogCharacter::IncreaseSpeedController);

	}
	else
	{
		UE_LOG(LogTemplateCharacter, Error, TEXT("'%s' Failed to find an Enhanced Input component! This template is built to use the Enhanced Input system. If you intend to use the legacy system, then you will need to update this C++ file."), *GetNameSafe(this));
	}
}

void ACatchDogCharacter::Move(const FInputActionValue& Value)
{
	// input is a Vector2D
	FVector2D MovementVector = Value.Get<FVector2D>();

	if (Controller != nullptr)
	{
		// find out which way is forward
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);

		// get forward vector
		const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
	
		// get right vector 
		const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

		// add movement 
		AddMovementInput(ForwardDirection, MovementVector.Y);
		AddMovementInput(RightDirection, MovementVector.X);
	}

	DestroyTrajectory();
}

void ACatchDogCharacter::Look(const FInputActionValue& Value)
{
	// input is a Vector2D
	FVector2D LookAxisVector = Value.Get<FVector2D>();

	if (Controller != nullptr)
	{
		// add yaw and pitch input to controller
		AddControllerYawInput(LookAxisVector.X);
		AddControllerPitchInput(LookAxisVector.Y);
	}
}

void ACatchDogCharacter::ThrowObject(const FInputActionValue& Value)
{
	if (!CatchDogPlayerController)
	{
		return;
	}

	// Check if there is any item to throw
	if (CatchDogPlayerController->GetInventoryNumberOfItems() == 1)
	{
		bThrow = true;

		if (ThrowMontage)
		{
			StopAnimMontage(AimMontage);
			PlayAnimMontage(ThrowMontage, 1.0f, TEXT("ThrowMontage"));
		}

		SpawnFromAnimation();
	}
}

void ACatchDogCharacter::SpawnFromAnimation()
{
	// Spawn actor in projectileReference position, with same rotation as character
	if (HasAuthority())
	{
		SpawnThrowObject(); // Spawn Object from the server
	}
	else
	{
		Server_RPCSpawnThrowableActor(); // Call RPC to spawn object from the server
	}
}

void ACatchDogCharacter::Server_RPCSpawnThrowableActor_Implementation()
{
	SpawnThrowObject();
}

void ACatchDogCharacter::SpawnThrowObject()
{
	// Spawn actor
	FActorSpawnParameters SpawnInfo;
	SpawnInfo.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::DontSpawnIfColliding;

	if (GetSubclassFromInventoryObject() == nullptr)
	{
		return;
	}

	ThrowableActorSpawned = GetWorld()->SpawnActor<AThrowableObject>(GetSubclassFromInventoryObject(), ProjectileReference->GetComponentLocation(), GetActorRotation(), SpawnInfo);

	// Check if the actor was spawned successfully
	if (ThrowableActorSpawned)
	{
		// get unit vector from character to object
		FVector UnitVectorFromCharacterToObject = UKismetMathLibrary::GetDirectionUnitVector(GetActorLocation(), ProjectileReference->GetComponentToWorld().GetLocation());

		// add linear speed
		USphereComponent* ThrowSphereCollider = ThrowableActorSpawned->GetSphereCollider();
		if (ThrowSphereCollider)
		{
			// Check if the SphereCatcher component exists
			ThrowSphereCollider->SetSimulatePhysics(true); // Ensure physics simulation is enabled
			ThrowSphereCollider->SetPhysicsLinearVelocity(ProjectileSpeed * UnitVectorFromCharacterToObject + GetActorForwardVector() * GetCharacterMovement()->GetLastUpdateVelocity().Size2D(), true);

			ThrowableActorSpawned->SetItemType(CatchDogPlayerController->GetItemTypeInInventory());

			if(CatchDogPlayerController)
			{
				CatchDogPlayerController->RemoveItemFromInventory();

				bThrow = false;
				bIsAiming = false;

				DestroyTrajectory();
			}
		}
	}
}

void ACatchDogCharacter::Aim(const FInputActionValue& Value)
{
	if (!CatchDogPlayerController)
	{
		return;
	}

	if (CatchDogPlayerController->GetInventoryNumberOfItems() == 1)
	{
		SpawnAimingTrajectory();

		bIsAiming = true;

		if(AimMontage)
		{
			PlayAnimMontage(AimMontage, 1.0f, TEXT("AnimMontage"));
		}
	} 
	else
	{
		StopAnimMontage(AimMontage);
	}
	// No items to throw or aim with.
	
	
}

void ACatchDogCharacter::SpawnAimingTrajectory()
{
	if (!bIsAiming)
	{
		StopAnimMontage(AimMontage);
		return;
	}

	// get unit vector from character to object
	FVector UnitVectorFromCharacterToObject = UKismetMathLibrary::GetDirectionUnitVector(GetActorLocation(), ProjectileReference->GetComponentToWorld().GetLocation());

	// Set up prediction parameters
	FPredictProjectilePathParams PathParams;
	PathParams.StartLocation = ProjectileReference->GetComponentToWorld().GetLocation(); // Set the start location of the projectile
	PathParams.LaunchVelocity = UnitVectorFromCharacterToObject * ProjectileSpeed; // Set the initial launch velocity
	PathParams.MaxSimTime = 3.0f; // max simulation time (0.1f, since it is being drawn every frame)
	PathParams.bTraceWithChannel = true;
	PathParams.TraceChannel = ECollisionChannel::ECC_WorldStatic;
	PathParams.SimFrequency = ProjectileSpeed / 20.0f; // Simulation frequency
	PathParams.DrawDebugType = EDrawDebugTrace::None; // Draw only one frame 
	PathParams.ProjectileRadius = 0.0f; // try line instead of debug meshes

	/* Check trace with collisions */
	PathParams.bTraceWithCollision = true;

	FPredictProjectilePathResult PathResult;

	// Predict the projectile path
	bool bHit = UGameplayStatics::PredictProjectilePath(this, PathParams, PathResult);

	int index = 0;

	if (TrajectoryPoints.IsEmpty())
	{
		// Spawn object in the whole trajectory
		for (FPredictProjectilePathPointData PointInPath : PathResult.PathData)
		{
			FActorSpawnParameters SpawnParams;
			SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::DontSpawnIfColliding;

			// Spawn actor from server and add actors to TrajectoryPoints array
			// The value 12 is to spawn a static mesh every 12 points, due to the huge amount of points in a trajectory (can be more than 240,
			// and spawning 240 static meshes is too performance-wise costly. 
			if (ThrowableObjectClass && (index % 6 == 0))
			{
				AActor* TrajectoryMesh = GetWorld()->SpawnActor<AActor>(ThrowableObjectClass, PointInPath.Location, GetActorRotation(), SpawnParams);

				TrajectoryPoints.Add(TrajectoryMesh);
			}

			index++;
		}
		// end of for loop

		if (bHit) // Spawn Actor at the end of the trajectory
		{
			FActorSpawnParameters SpawnParams;
			SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

			AActor* CollisionPoint = GetWorld()->SpawnActor<AActor>(GetSpawnableSubclassFromInventoryObject(), PathResult.HitResult.ImpactPoint, GetActorRotation(), SpawnParams);

			TrajectoryPoints.Add(CollisionPoint);
		}
	}
	// TrajectoryPoints is not empty, therefore it is not needed to spawn all over it again.

}

void ACatchDogCharacter::DestroyTrajectory()
{
	if (!TrajectoryPoints.IsEmpty())
	{
		for (AActor* ActorInTrajectory : TrajectoryPoints)
			if (ActorInTrajectory)
			{
				ActorInTrajectory->Destroy();
			}

		TrajectoryPoints.Empty();
	}
}


void ACatchDogCharacter::StopAiming(const FInputActionValue& Value)
{
	StopAnimMontage(AimMontage);

	DestroyTrajectory();

	bIsAiming = false;
}



void ACatchDogCharacter::AdjustSpeed(const FInputActionValue& Value)
{
	float MouseWheelAxis = Value.Get<float>();

	if (MouseWheelAxis < 0.0f && ProjectileSpeed >= 200.0f)
	{
		ProjectileSpeed -= 50.0f;
	}
	else if (MouseWheelAxis > 0.0f && ProjectileSpeed <= 1500.0f)
	{
		ProjectileSpeed += 50.0f;
	}

	DestroyTrajectory();
}

void ACatchDogCharacter::TogglePauseMenu(const FInputActionValue& Value)
{
	// do something
	if (!IsLocallyControlled() || !GetWorld())
	{
		return;
	}

	if (ACatchDogPlayerController* playerController = Cast<ACatchDogPlayerController>(GetController()))
	{
		if (!playerController->IsLocalController())
		{
			return;
		}

		if (ACatchDogHUD* CatchDogHUD = Cast<ACatchDogHUD>(playerController->MyHUD))
		{
			CatchDogHUD->TogglePauseMenu();
		}
	}

}

void ACatchDogCharacter::DecreaseSpeedController(const FInputActionValue& Value)
{
	float ControllerInput = Value.Get<float>();

	if (ControllerInput > 0.0f && ProjectileSpeed >= 200.0f)
	{
		ProjectileSpeed -= 50.0f;
	}

	DestroyTrajectory();
}

void ACatchDogCharacter::IncreaseSpeedController(const FInputActionValue& Value)
{
	float ControllerInput = Value.Get<float>();

	if (ControllerInput > 0.0f && ProjectileSpeed <= 1500.0f)
	{
		ProjectileSpeed += 50.0f;
	}

	DestroyTrajectory();
}

//////////////////////////////////////////////////////////////////////////

// PICK UP AND ACTIVATE ITEM

void ACatchDogCharacter::PickUp(AActor* PickableActor)
{
	EItemType ItemType;

	// pick up action
	if (PickableActor)
	{
		if (PickableActor->IsA<ASpeedBoost>()) // Speed Boost
		{
			ItemType = EItemType::SpeedBoost;
		}
		else if (PickableActor->IsA<AShield>()) // Shield
		{
			ItemType = EItemType::Shield;
		}
		else if (PickableActor->IsA<ABearTrapPickable>()) // Bear Trap Pickable
		{
			ItemType = EItemType::BearTrap;
		}
		else if (PickableActor->IsA<ANetTrapPickable>()) // Net Trap Pickable
		{
			ItemType = EItemType::NetTrap;
		}
		else if (PickableActor->IsA<ABone>()) // Bone
		{
			ItemType = EItemType::Bone;
		}
		else // None
		{
			ItemType = EItemType::None;
		}

		ActivateItem(ItemType, PickableActor);
	}
	
}

void ACatchDogCharacter::ActivateItem(EItemType ItemType, AActor* PickableActor)
{
	if(CatchDogPlayerController)
	{
		int32 PrevInventoryNumberOfItems = CatchDogPlayerController->GetInventoryNumberOfItems();

		switch (ItemType)
		{
			// Shield Boost Item
		case EItemType::Shield:

			ActivateShield(true); // Activate shield

			GetWorld()->GetTimerManager().SetTimer(
				ShieldTimerHandle, // handle to cancel timer at a later time
				this, // the owning object
				&ACatchDogCharacter::OnTimerEndShield, // function to call on elapsed
				TimeIntervalForShield, // float delay until elapsed
				false); // looping

			bShieldPlaySound = !bShieldPlaySound;
			OnRep_ShieldPlaySound();
			DestroyAndRemoveFromArray(PickableActor);
			
			break;

			// Speed Boost Item
		case EItemType::SpeedBoost:

			// Set new max velocity
			SetMaxMinWalkSpeed(CurrentMinAnalogWalkSpeed, SpeedBoostVelocity);
			
			// Start timer
			GetWorld()->GetTimerManager().SetTimer(
				SpeedBoostTimerHandle, // handle to cancel timer at a later time
				this, // the owning object
				&ACatchDogCharacter::OnTimerEndSpeedBoost, // function to call on elapsed
				TimeIntervalForSpeedBoost, // float delay until elapsed
				false); // looping

			bSpeedBoostPlaySound = !bSpeedBoostPlaySound;
			OnRep_SpeedBoostPlaySound();
			DestroyAndRemoveFromArray(PickableActor);
		
			CatchDogPlayerController->PlayerScored(-1);

			break;

			// Bear Trap Item
		case EItemType::BearTrap:

			AddItemToInventory(EItemType::BearTrap);

			break;

			// Net Trap Item
		case EItemType::NetTrap:

			AddItemToInventory(EItemType::NetTrap);

			break;

			// Bone Item
		case EItemType::Bone:

			AddItemToInventory(EItemType::Bone);

			break;

			// None Item
		case EItemType::None:

			break;
		}


		// IF the number of items has increased, it means the player had not items before picking it up, so let's destroy it.
		if (CatchDogPlayerController->GetInventoryNumberOfItems() != PrevInventoryNumberOfItems)
		{
			bTrapPlaySound = !bTrapPlaySound;
			OnRep_TrapPlaySound();
			DestroyAndRemoveFromArray(PickableActor);
		}

	}
}

void ACatchDogCharacter::DestroyAndRemoveFromArray(AActor* pickableActor)
{
	if(!pickableActor) // check nullptr
	{
		return;
	}

	if (ACatchDogGameMode* CatchDogGameMode = Cast<ACatchDogGameMode>(UGameplayStatics::GetGameMode(GetWorld())))
	{
		CatchDogGameMode->TPickableActorsArray.Remove(Cast<APickableActor>(pickableActor)); // Remove Puppy
		pickableActor->Destroy();
	}
}

void ACatchDogCharacter::OnTimerEndSpeedBoost()
{
	// Set new max velocity
	if (CurrentMaxWalkSpeed >= 500.0f)
	{
		SetMaxMinWalkSpeed(CurrentMinAnalogWalkSpeed, 500.0f);
	}
}



void ACatchDogCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ThisClass, ThrowableActorSpawned);
	DOREPLIFETIME(ThisClass, CurrentMaxWalkSpeed);
	DOREPLIFETIME(ThisClass, bPuppyPlaySound);
	DOREPLIFETIME(ThisClass, bShieldPlaySound);
	DOREPLIFETIME(ThisClass, bSpeedBoostPlaySound);
	DOREPLIFETIME(ThisClass, bTrapPlaySound);
}

// CHANGING CHARACTER SPEED FROM TRAPS

void ACatchDogCharacter::SetMaxMinWalkSpeed(float MinWalkSpeed, float MaxWalkSpeed)
{
	CurrentMaxWalkSpeed = MaxWalkSpeed;
	CurrentMinAnalogWalkSpeed = MinWalkSpeed;

	if (HasAuthority())
	{
		OnRep_CurrentMaxWalkSpeed();
	}
	else
	{
		ServerRPC_SetNewWalkSpeed();
	}
}

void ACatchDogCharacter::OnRep_CurrentMaxWalkSpeed()
{
	SetNewWalkSpeed();
}

void ACatchDogCharacter::ServerRPC_SetNewWalkSpeed_Implementation()
{
	SetNewWalkSpeed();
}

void ACatchDogCharacter::SetNewWalkSpeed()
{
	GetCharacterMovement()->MaxWalkSpeed = CurrentMaxWalkSpeed;
	GetCharacterMovement()->MinAnalogWalkSpeed = CurrentMinAnalogWalkSpeed;
}


float ACatchDogCharacter::GetMaxWalkSpeed()
{
	return CurrentMaxWalkSpeed;
}

// SHIELD AND INVENTORY

void ACatchDogCharacter::OnTimerEndShield()
{
	// Reset walk speed
	if (CatchDogPlayerController)
	{
		CatchDogPlayerController->ActivateShield(false);
	}
}

bool ACatchDogCharacter::GetShield()
{
	if (CatchDogPlayerController)
	{
		return CatchDogPlayerController->GetShield();
	}
	else
	{
		return false;
	}
}

void ACatchDogCharacter::ActivateShield(bool ShieldMode)
{
	if (CatchDogPlayerController)
	{
		CatchDogPlayerController->ActivateShield(ShieldMode);
	}	
}

void ACatchDogCharacter::AddItemToInventory(EItemType ItemType)
{
	if (CatchDogPlayerController)
	{
		CatchDogPlayerController->AddItemToInventory(ItemType);
	}
	
}

TSubclassOf<AActor> ACatchDogCharacter::GetSubclassFromInventoryObject()
{
	if (!CatchDogPlayerController)
	{
		return nullptr;
	}

	EItemType ItemType = CatchDogPlayerController->GetItemTypeInInventory();

	switch (ItemType)
	{
	case EItemType::BearTrap: // Object in Inventory is Bear Trap

		return BearTrapThrowableClass;
		break;

	case EItemType::NetTrap: // Object in Inventory is Net Trap

		return NetTrapThrowableClass;
		break;

	case EItemType::Bone: // Object in Inventory is Bone

		return BoneThrowableClass;
		break;

	default: // Object in Inventory is None

		return nullptr;
		break;

	}
}

TSubclassOf<AActor> ACatchDogCharacter::GetSpawnableSubclassFromInventoryObject()
{
	if (!CatchDogPlayerController)
	{
		return nullptr;
	}

	EItemType ItemType = CatchDogPlayerController->GetItemTypeInInventory();

	switch (ItemType)
	{
	case EItemType::BearTrap: // Object in Inventory is Bear Trap

		return BearTrapSpawnableClass;
		break;

	case EItemType::NetTrap: // Object in Inventory is Net Trap

		return NetTrapSpawnableClass;
		break;

	case EItemType::Bone: // Object in Inventory is Bone

		return BoneSpawnableClass;
		break;

	default: // Object in Inventory is None

		return nullptr;
		break;

	}
}


// OVERLAP FUNCTIONS

void ACatchDogCharacter::OnBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if(HasAuthority()) // If not Server, just escape the function.
	{
		// If this point is reached, the Actor HasAuthority() = true (It's the Server).
		if (!OtherActor)
		{
			return;
		}

		if (OtherActor->IsA<APickableActor>()) // Pickable Actor (Power-ups and traps)
		{
			CatchDogPlayerController = Cast<ACatchDogPlayerController>(GetController());

			if (CatchDogPlayerController)
			{
				PickUp(OtherActor);
			}
		}
		else if (OtherActor->IsA<APuppy>()) // Puppies
		{
			CatchDogPlayerController = Cast<ACatchDogPlayerController>(GetController());

			if (CatchDogPlayerController)
			{
				if (ACatchDogGameMode* CatchDogGameMode = Cast<ACatchDogGameMode>(UGameplayStatics::GetGameMode(GetWorld())))
				{
					CatchDogGameMode->TPuppiesArray.Remove(Cast<APuppy>(OtherActor)); // Remove Puppy

					ACatchDogAIController* AIController = Cast<APuppy>(OtherActor)->AIControllerReference;
					CatchDogGameMode->TPuppiesAIControllersArray.Remove(AIController); // Remove AI Controller which possessed Puppy

					if (OtherActor && AIController)
					{
						AIController->Destroy();
						OtherActor->Destroy();

						//Play Sound for client
						bPuppyPlaySound = !bPuppyPlaySound;
						OnRep_PuppyPlaySound();
					}
				}
				
				CatchDogPlayerController->PlayerScored(1);
			}

		}
	}
	
}


void ACatchDogCharacter::OnRep_PuppyPlaySound()
{
	PlayPuppySound();
}

void ACatchDogCharacter::OnRep_ShieldPlaySound()
{
	if (ShieldSoundToPlay)
	{
		UGameplayStatics::PlaySoundAtLocation(this, ShieldSoundToPlay, GetActorLocation(), 1.0f, 1.0f, 0.0f);
	}
}

void ACatchDogCharacter::OnRep_SpeedBoostPlaySound()
{
	if (SpeedBoostSoundToPlay)
	{
		UGameplayStatics::PlaySoundAtLocation(this, SpeedBoostSoundToPlay, GetActorLocation(), 1.0f, 1.0f, 0.0f);
	}
}

void ACatchDogCharacter::OnRep_TrapPlaySound()
{
	if (TrapsSoundToPlay)
	{
		UGameplayStatics::PlaySoundAtLocation(this, TrapsSoundToPlay, GetActorLocation(), 1.0f, 1.0f, 0.0f);
	}
}
int32 ACatchDogCharacter::Remainder(int32 Number1, int32 Number2)
{
	return Number1%Number2;
}
void ACatchDogCharacter::PlayPuppySound()
{
	if (PuppySoundToPlay)
	{
		UGameplayStatics::PlaySoundAtLocation(this, PuppySoundToPlay, GetActorLocation(), 1.0f, 1.0f, 0.0f);
	}
}

void ACatchDogCharacter::OnEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	// do something here
}
