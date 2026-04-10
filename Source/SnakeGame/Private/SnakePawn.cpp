// Fill out your copyright notice in the Description page of Project Settings.

// SnakePawn.cpp
#include "SnakePawn.h"

#include "Camera/CameraComponent.h"
#include "Components/InputComponent.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/SpringArmComponent.h"
#include "InputAction.h"
#include "InputMappingContext.h"

// Sets default values
ASnakePawn::ASnakePawn()
{
	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	// Collision Sphere is the root object
	CollisionSphere = CreateDefaultSubobject<USphereComponent>(TEXT("Collision"));
	SetRootComponent(CollisionSphere);
	CollisionSphere->InitSphereRadius(50.f);

	// attach the VisualMesh to the sphere
	VisualMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("VisualMesh"));
	VisualMesh->SetupAttachment(CollisionSphere);

	// attach the SpringArm to the sphere and set position settings
	SpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArm"));
	SpringArm->SetupAttachment(CollisionSphere);
	SpringArm->TargetArmLength = 700.f;
	SpringArm->SetRelativeRotation(FRotator(-55.0f, 0.0f, 0.0f));
	SpringArm->bDoCollisionTest = false;
	SpringArm->bInheritPitch = false;
	SpringArm->bInheritRoll = false;
	SpringArm->bInheritYaw = true;

	// look into enable lagg for smooth spring arm turn

	// attach the camera to the SpringArm
	Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	Camera->SetupAttachment(SpringArm);

	AutoPossessPlayer = EAutoReceiveInput::Player0;
}

// Called when the game starts or when spawned
void ASnakePawn::BeginPlay()
{
	Super::BeginPlay();


	// should set up PlayerController and Input mappings
	if (APlayerController* PlayerController = Cast<APlayerController>(GetController()))
	{
		if (ULocalPlayer* LocalPlayer = PlayerController->GetLocalPlayer())
		{
			if (UEnhancedInputLocalPlayerSubsystem* InputSubsystem = LocalPlayer->GetSubsystem<
				UEnhancedInputLocalPlayerSubsystem>())
			{
				if (DefaultMappingContext)
				{
					InputSubsystem->AddMappingContext(DefaultMappingContext, 0);
				}
			}
		}
	}
}

// Called every frame
void ASnakePawn::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	UpdateRotationFromInput(DeltaTime);
	MoveTick(DeltaTime);
}

// Called to bind functionality to input
void ASnakePawn::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	if (UEnhancedInputComponent* EnhancedInput = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		/* NOTE: could use BindActionInstanceLambda to call the methods. 
		// Example:
		EnhancedInput->BindActionInstanceLambda(
		TurnLeftAction,
		ETriggerEvent::Started,
		[this](const FInputActionInstance&)
		{
			TurnLeft(true);
		});
		*/

		if (TurnLeftAction)
		{
			EnhancedInput->BindAction(TurnLeftAction, ETriggerEvent::Started, this, &ASnakePawn::OnTurnLeftStarted);
			EnhancedInput->BindAction(TurnLeftAction, ETriggerEvent::Completed, this, &ASnakePawn::OnTurnLeftReleased);
			EnhancedInput->BindAction(TurnLeftAction, ETriggerEvent::Canceled, this, &ASnakePawn::OnTurnLeftReleased);
		}

		if (TurnRightAction)
		{
			EnhancedInput->BindAction(TurnRightAction, ETriggerEvent::Started, this, &ASnakePawn::OnTurnRightStarted);
			EnhancedInput->BindAction(TurnRightAction, ETriggerEvent::Completed, this,
			                          &ASnakePawn::OnTurnRightReleased);
			EnhancedInput->BindAction(TurnRightAction, ETriggerEvent::Canceled, this, &ASnakePawn::OnTurnRightReleased);
		}
	}
}


void ASnakePawn::OnTurnLeftStarted(const FInputActionValue& Value)
{
	bLeftHeld = true;
	LeftPressOrder = ++PressSequenceCounter;
}

void ASnakePawn::OnTurnLeftReleased(const FInputActionValue& Value)
{
	bLeftHeld = false;
	LeftPressOrder = INDEX_NONE;
	// not needed but cleaner? Also means press order is only meaningful while the key is held 
}

void ASnakePawn::OnTurnRightStarted(const FInputActionValue& Value)
{
	bRightHeld = true;
	RightPressOrder = ++PressSequenceCounter;
}

void ASnakePawn::OnTurnRightReleased(const FInputActionValue& Value)
{
	bRightHeld = false;
	RightPressOrder = INDEX_NONE;
}

float ASnakePawn::ResolveConflictDirection(const ETurnPriorityMode& PriorityMode) const
{
	switch (PriorityMode)
	{
	case ETurnPriorityMode::FirstPress:
		return (LeftPressOrder < RightPressOrder) ? -1.0f : 1.0f;

	case ETurnPriorityMode::LastPress:
		return (LeftPressOrder > RightPressOrder) ? -1.0f : 1.0f;

	case ETurnPriorityMode::ExclusiveInput:
	default:
		return 0.0f;
	}
}

float ASnakePawn::GetTurnDirection() const
{
	if (!bLeftHeld && !bRightHeld)
	{
		return 0.0f;
	}

	if (bLeftHeld && !bRightHeld)
	{
		return -1.0f;
	}

	if (!bLeftHeld && bRightHeld)
	{
		return 1.0f;
	}

	// If both buttons are held down at same time 
	return ResolveConflictDirection(TurnPriorityMode);
}

void ASnakePawn::UpdateRotationFromInput(float DeltaTime)
{
	const float TurnDirection = GetTurnDirection();
	if (TurnDirection == 0.0f)
	{
		return;
	}

	const float YawDelta = TurnDirection * BaseTurnRateDegPerSec * DeltaTime;
	AddActorLocalRotation(FRotator(0.0f, YawDelta, 0.0f));
}

void ASnakePawn::MoveTick(float DeltaTime)
{
	const FVector Delta = GetActorForwardVector() * BaseMoveSpeed * DeltaTime;
	AddActorWorldOffset(Delta, true);
}
