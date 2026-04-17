// Fill out your copyright notice in the Description page of Project Settings.

//SnakeGridwalkerPawn.cpp
#include "SnakeGridwalkerPawn.h"

#include "Camera/CameraComponent.h"
#include "Components/InputComponent.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/SpringArmComponent.h"
#include "InputAction.h"
#include "InputMappingContext.h"


namespace
{
	FIntPoint GridDelta(const EGridDirection Direction)
	{
		switch (Direction)
		{
		case EGridDirection::Up: return FIntPoint(1, 0);
		case EGridDirection::Down: return FIntPoint(-1, 0);
		case EGridDirection::Right: return FIntPoint(0, 1);
		case EGridDirection::Left: return FIntPoint(0, -1);
		case EGridDirection::None:
		default:
			return FIntPoint::ZeroValue;
		}
	}

	bool Reversing(const EGridDirection CurrentDirection, const EGridDirection NextDirection)
	{
		return
			(CurrentDirection == EGridDirection::Up && NextDirection == EGridDirection::Down) ||
			(CurrentDirection == EGridDirection::Down && NextDirection == EGridDirection::Up) ||
			(CurrentDirection == EGridDirection::Right && NextDirection == EGridDirection::Left) ||
			(CurrentDirection == EGridDirection::Left && NextDirection == EGridDirection::Right);
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

	VisualSegmentMesh = CreateDefaultSubobject<UInstancedStaticMeshComponent>(TEXT("VisualSegmentMesh"));
	VisualSegmentMesh->SetupAttachment(CollisionSphere);

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

void ASnakeGridwalkerPawn::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);

	if (VisualMesh)
	{
		VisualMesh->SetStaticMesh(HeadMeshAsset);
		if (HeadMeshAsset)
		{
			VisualMesh->SetMaterial(0, HeadMaterialAsset);
		}
	}

	if (VisualSegmentMesh)
	{
		VisualSegmentMesh->SetStaticMesh(SegmentMeshAsset);
		if (SegmentMeshAsset)
		{
			VisualSegmentMesh->SetMaterial(0, SegmentMaterialAsset);
		}
	}
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


	UpdateTurnVisual(DeltaTime);

	StepAccumulator += DeltaTime;
	if (StepAccumulator >= StepInterval)
	{
		StepAccumulator -= StepInterval;
		AdvanceSnakeOneStep();
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
			EnhancedInput->BindAction(MoveAction, ETriggerEvent::Triggered, this,
			                          &ASnakeGridwalkerPawn::OnMoveInput);
			EnhancedInput->BindAction(MoveAction, ETriggerEvent::Completed, this,
			                          &ASnakeGridwalkerPawn::OnMoveInput);
			EnhancedInput->BindAction(MoveAction, ETriggerEvent::Canceled, this,
			                          &ASnakeGridwalkerPawn::OnMoveInput);
		}

		if (TestGrowthAction)
		{
			EnhancedInput->BindAction(TestGrowthAction, ETriggerEvent::Started, this,
			                          &ASnakeGridwalkerPawn::OnGrowPressed);
		}
	}
}

void ASnakeGridwalkerPawn::RequestGrowth(int32 Amount)
{
	PendingGrowth += Amount;
}

void ASnakeGridwalkerPawn::OnGrowPressed(const FInputActionValue& Value)
{
	RequestGrowth();
}

EGridDirection ASnakeGridwalkerPawn::ResolveDirectionFromInput(const FVector2D& Input)
{
	// no input early out
	if (Input.IsNearlyZero())
	{
		return EGridDirection::None; // Note that this is currently overwriting older input? 
	}

	// Vertical or Horizontal movement biggest, then direction on that axis 
	if (FMath::Abs(Input.X) > FMath::Abs(Input.Y)) // X axis = forward/backward
	{
		return Input.X > 0.0f ? EGridDirection::Up : EGridDirection::Down;
	}
	if (FMath::Abs(Input.Y) > FMath::Abs(Input.X)) // Y axis = right/left
	{
		return Input.Y > 0.0f ? EGridDirection::Right : EGridDirection::Left;
	}

	// A-W or S-D tie case: falls through to not be counted 
	return EGridDirection::None;
}

void ASnakeGridwalkerPawn::OnMoveInput(const FInputActionValue& Value)
{
	RawMoveInput = Value.Get<FVector2D>();

	// store active inputs only 
	const EGridDirection InputDirection = ResolveDirectionFromInput(RawMoveInput);
	if (InputDirection != EGridDirection::None)
	{
		PendingNextDirection = InputDirection;
	}
}

bool ASnakeGridwalkerPawn::TryConsumeGrowth()
{
	if (PendingGrowth <= 0)
	{
		return false;
	}


	--PendingGrowth;
	return true;
}

FVector ASnakeGridwalkerPawn::CellToWorld(const FIntPoint Cell) const
{
	return FVector(
		Cell.X * CellSize,
		Cell.Y * CellSize,
		GetActorLocation().Z);
}

float ASnakeGridwalkerPawn::DirectionToYaw(EGridDirection Direction)
{
	switch (Direction)
	{
	case EGridDirection::Up:
		return 0.0f; // +X forward
	case EGridDirection::Right:
		return 90.0f; // +Y right
	case EGridDirection::Down:
		return 180.0f; // -X back
	case EGridDirection::Left:
		return -90.0f; // -Y left
	default:
		return 0.0f;
	}
}

void ASnakeGridwalkerPawn::StartTurnVisual(EGridDirection NewDirection)
{
	TurnTargetYaw = DirectionToYaw(NewDirection);
	TurnStartYaw = VisualMesh->GetRelativeRotation().Yaw;

	TurnRotationElapsed = 0.0f;
	IsTurning = true;
}

void ASnakeGridwalkerPawn::UpdateTurnVisual(float DeltaTime)
{
	if (!IsTurning)
	{
		return;
	}

	TurnRotationElapsed += DeltaTime;

	// Alpha = Clamp(TurnElapsed / TurnDuration, 0, 1);  Clamping makes the time become a normalized progress value. 
	// Percentage of completion, say:  
	// TurnElapsed = 0.3f;
	// TurnDuration = 0.6f; 
	// 0.3 / 0.6 = 0.5    -> Alpha = 0.5    -> means 50% completed 
	const float Alpha = FMath::Clamp(TurnRotationElapsed / TurnDuration, 0.0f, 1.0f);
	const float DeltaYaw = FMath::FindDeltaAngleDegrees(TurnStartYaw, TurnTargetYaw);
	const float NewYaw = TurnStartYaw + (DeltaYaw * Alpha);

	VisualMesh->SetRelativeRotation(FRotator(0.0f, NewYaw, 0.0f));

	// stop rotation at target value, even if it would have overshoot. 
	if (Alpha >= 1.0f) // TurnRotationElapsed >= TurnDuration
	{
		VisualMesh->SetRelativeRotation(FRotator(0.0f, TurnTargetYaw, 0.0f));
		IsTurning = false;
		TurnRotationElapsed = 0.0f;
	}
}

void ASnakeGridwalkerPawn::ApplyPendingDirection()
{
	// Assign heading direction, and update rotation if direction changed
	if (PendingNextDirection != EGridDirection::None &&
		!Reversing(CurrentDirection, PendingNextDirection))
	{
		if (CurrentDirection != PendingNextDirection)
		{
			StartTurnVisual(PendingNextDirection);
		}
		CurrentDirection = PendingNextDirection;
	}

	// consume active direction-change input
	PendingNextDirection = EGridDirection::None;
}


FIntPoint ASnakeGridwalkerPawn::PeekNextHeadCell() const
{
	return GridPosition + GridDelta(CurrentDirection);
}

void ASnakeGridwalkerPawn::UpdateHeadWorldLocation(const FIntPoint& NextHeadCell)
{
	SetActorLocation(CellToWorld(NextHeadCell));
}

void ASnakeGridwalkerPawn::AdvanceBodySegments(FIntPoint VacatedCell)
{
	if (BodyCells.IsEmpty())
	{
		return;
	}

	for (int32 i = 0; i < BodyCells.Num(); ++i)
	{
		const FIntPoint OldSegmentCell = BodyCells[i];
		BodyCells[i] = VacatedCell;
		VacatedCell = OldSegmentCell;
	}
}

void ASnakeGridwalkerPawn::AdvanceSnakeOneStep()
{
	ApplyPendingDirection();

	const FIntPoint NextHeadCell = PeekNextHeadCell();

	// later:
	// if (!CanEnterCell(NextHeadCell)) { handle death/block; return; }

	const FIntPoint PreviousHeadCell = GridPosition;
	const FIntPoint PreviousTailCell = BodyCells.Num() > 0 ? BodyCells.Last() : GridPosition;

	// --- Update logic state, ---
	GridPosition = NextHeadCell;
	AdvanceBodySegments(PreviousHeadCell);

	if (TryConsumeGrowth())
	{
		BodyCells.Add(PreviousTailCell);
	}

	// --- Update Visual / World-Sync after all logic ---
	UpdateHeadWorldLocation(NextHeadCell);
}
