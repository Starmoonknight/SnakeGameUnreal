// Fill out your copyright notice in the Description page of Project Settings.

//SnakeGridwalkerPawn.cpp
#include "SnakeGridwalkerPawn.h"

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


namespace
{
	FIntPoint GridDelta(const EGridDirection Dir)
	{
		switch (Dir)
		{
		case EGridDirection::UP: return FIntPoint(1.0f, 0.0f);
		case EGridDirection::DOWN: return FIntPoint(-1.0f, 0.0f);
		case EGridDirection::RIGHT: return FIntPoint(0.0f, 1.0f);
		case EGridDirection::LEFT: return FIntPoint(0.0f, -1.0f);
		case EGridDirection::NONE:
		default:
			return FIntPoint::ZeroValue;
		}
	}

	bool Reversing(const EGridDirection CurrentDirection, const EGridDirection NextDirection)
	{
		return
			(CurrentDirection == EGridDirection::UP && NextDirection == EGridDirection::DOWN) ||
			(CurrentDirection == EGridDirection::DOWN && NextDirection == EGridDirection::UP) ||
			(CurrentDirection == EGridDirection::RIGHT && NextDirection == EGridDirection::LEFT) ||
			(CurrentDirection == EGridDirection::LEFT && NextDirection == EGridDirection::RIGHT);
	}
}


// Sets default values
ASnakeGridwalkerPawn::ASnakeGridwalkerPawn()
{
	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	// Collision Sphere is the root object
	CollisionSphere = CreateDefaultSubobject<USphereComponent>(TEXT("Collision"));
	SetRootComponent(CollisionSphere);
	CollisionSphere->SetSphereRadius(50.f);

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
	SpringArm->bInheritYaw = false;

	// attach the camera to the SpringArm
	Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	Camera->SetupAttachment(SpringArm);

	AutoPossessPlayer = EAutoReceiveInput::Player0;
}

// Called when the game starts or when spawned
void ASnakeGridwalkerPawn::BeginPlay()
{
	Super::BeginPlay();

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
void ASnakeGridwalkerPawn::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// need a better time ticker?
	StepAccumulator += DeltaTime;
	if (StepAccumulator >= StepInterval)
	{
		StepAccumulator -= StepInterval;
		StepMove();
	}
}

// Called to bind functionality to input
void ASnakeGridwalkerPawn::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	// should bind actions
	if (UEnhancedInputComponent* EnhancedInput = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		if (MoveAction)
		{
			EnhancedInput->BindAction(MoveAction, ETriggerEvent::Triggered, this, &ASnakeGridwalkerPawn::Move);
			EnhancedInput->BindAction(MoveAction, ETriggerEvent::Completed, this, &ASnakeGridwalkerPawn::Move);
			EnhancedInput->BindAction(MoveAction, ETriggerEvent::Canceled, this, &ASnakeGridwalkerPawn::Move);
		}
	}
}

EGridDirection ASnakeGridwalkerPawn::ResolveDirectionFromInput(const FVector2D& Input) const
{
	// no input early out
	if (Input.IsNearlyZero())
	{
		return EGridDirection::NONE; // Note that this is currently overwriting older input? 
	}

	// Vertical or Horizontal movement biggest, then direction on that axis 
	if (FMath::Abs(Input.X) > FMath::Abs(Input.Y)) // X axis = forward/backward
	{
		return Input.X > 0.0f ? EGridDirection::UP : EGridDirection::DOWN;
	}
	if (FMath::Abs(Input.Y) > FMath::Abs(Input.X)) // Y axis = right/left
	{
		return Input.Y > 0.0f ? EGridDirection::RIGHT : EGridDirection::LEFT;
	}

	// A-W or S-D tie case: falls through to not be counted 
	return EGridDirection::NONE;
}

void ASnakeGridwalkerPawn::Move(const FInputActionValue& Value)
{
	RawMoveInput = Value.Get<FVector2D>();

	// store active inputs only 
	const EGridDirection InputDirection = ResolveDirectionFromInput(RawMoveInput);
	if (InputDirection != EGridDirection::NONE)
	{
		PendingNextDirection = InputDirection;
	}
}

void ASnakeGridwalkerPawn::StepMove()
{
	if (PendingNextDirection != EGridDirection::NONE &&
		!Reversing(CurrentDirection, PendingNextDirection))
	{
		CurrentDirection = PendingNextDirection;
	}

	// consume active input
	PendingNextDirection = EGridDirection::NONE;

	GridPosition += GridDelta(CurrentDirection);

	const FVector WorldLocation(
		GridPosition.X * CellSize,
		GridPosition.Y * CellSize,
		GetActorLocation().Z);

	SetActorLocation(WorldLocation);
}
