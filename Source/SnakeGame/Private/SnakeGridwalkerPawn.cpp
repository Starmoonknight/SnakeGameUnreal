// Fill out your copyright notice in the Description page of Project Settings.

//SnakeGridwalkerPawn.cpp
#include "SnakeGridwalkerPawn.h"
#include "SnakeSettingsDataAsset.h"
#include "SnakeGameModeBase.h"

#include "GridManagerActor.h"
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
#include "SnakeGameModeBase.h"
#include "Kismet/GameplayStatics.h"

//#include "Kismet/KismetMathLibrary.h"


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

	float DirectionToYaw(const EGridDirection Direction)
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
}


// Sets default values
ASnakeGridwalkerPawn::ASnakeGridwalkerPawn()
{
	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	AutoPossessPlayer = EAutoReceiveInput::Player0;

	// Collision Sphere is the root object
	CollisionSphere = CreateDefaultSubobject<USphereComponent>(TEXT("Collision"));
	SetRootComponent(CollisionSphere);
	CollisionSphere->SetSphereRadius(45.f);
	CollisionSphere->SetCollisionProfileName(TEXT("Pawn"));
	CollisionSphere->SetGenerateOverlapEvents(true);

	// attach the VisualMesh to the sphere
	SnakeHeadMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("SnakeHeadMesh"));
	SnakeHeadMesh->SetupAttachment(RootComponent);
	SnakeHeadMesh->SetRelativeLocation(FVector::ZeroVector);
	SnakeHeadMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	SnakeHeadMesh->SetSimulatePhysics(false);

	VisualSegmentMesh = CreateDefaultSubobject<UInstancedStaticMeshComponent>(TEXT("VisualSegmentMesh"));
	VisualSegmentMesh->SetupAttachment(RootComponent);
	// trying to fix flashing lights when snake moves with these below, temp:
	VisualSegmentMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	VisualSegmentMesh->SetMobility(EComponentMobility::Movable);
	VisualSegmentMesh->SetCastShadow(false);

	// attach the SpringArm to the sphere and set position settings
	SpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArm"));
	SpringArm->SetupAttachment(RootComponent);
	SpringArm->TargetArmLength = 700.f;
	SpringArm->bUsePawnControlRotation = false;
	SpringArm->SetUsingAbsoluteRotation(true);
	SpringArm->SetRelativeRotation(FRotator(-55.0f, 0.0f, 0.0f));
	SpringArm->bDoCollisionTest = false;
	SpringArm->bInheritPitch = false; // are these three still needed? This, 
	SpringArm->bInheritRoll = false; // this,
	SpringArm->bInheritYaw = false; // and this.
	SpringArm->bEnableCameraLag = true;
	SpringArm->CameraLagSpeed = 2.0F;
	SpringArm->CameraLagMaxDistance = 900.0F;

	// attach the camera to the SpringArm
	Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	Camera->SetupAttachment(SpringArm, USpringArmComponent::SocketName); // new, use of SocketName word
	// trying to fix flashing lights when snake moves with these below, temp:
	Camera->PostProcessSettings.bOverride_MotionBlurAmount = true;
	Camera->PostProcessSettings.MotionBlurAmount = 0.0f;

	bUseControllerRotationYaw = false; // new
}

void ASnakeGridwalkerPawn::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);

	ApplyVisualAssets();
}

// Called when the game starts or when spawned
void ASnakeGridwalkerPawn::BeginPlay()
{
	Super::BeginPlay();

	if (SnakeHeadMesh)
	{
		HeadMoveVisualRestRelativeLocation = SnakeHeadMesh->GetRelativeLocation();
	}

	// snake needs to be spawned by deferred spawning since it uses a reference set in that interaction here
	ResetSnake();
	SetupInputMapping();
}

void ASnakeGridwalkerPawn::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);

	SetupInputMapping();
}

// Called every frame
void ASnakeGridwalkerPawn::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (!bIsAlive)
	{
		return;
	}

	UpdateHeadTurnVisual(DeltaTime);
	UpdateHeadMoveVisual(DeltaTime);

	if (!bMovementActive || bMovementPaused)
	{
		return;
	}

	StepAccumulator += DeltaTime;

	// using while instead of if case should make the movement work even if laggy and low fps? 
	while (bIsAlive && StepAccumulator >= StepInterval)
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
			                          &ASnakeGridwalkerPawn::Input_OnMove);
			EnhancedInput->BindAction(MoveAction, ETriggerEvent::Completed, this,
			                          &ASnakeGridwalkerPawn::Input_OnMove);
			EnhancedInput->BindAction(MoveAction, ETriggerEvent::Canceled, this,
			                          &ASnakeGridwalkerPawn::Input_OnMove);
		}

		if (TestGrowthAction)
		{
			EnhancedInput->BindAction(TestGrowthAction, ETriggerEvent::Started, this,
			                          &ASnakeGridwalkerPawn::Input_OnGrowPressed);
		}

		if (TestResetAction)
		{
			EnhancedInput->BindAction(TestResetAction, ETriggerEvent::Started, this,
			                          &ASnakeGridwalkerPawn::Input_OnResetPressed);
		}
	}
}

void ASnakeGridwalkerPawn::ConfigureForGrid(AGridManagerActor* InGridManager, const FIntPoint& InSpawnCell)
{
	GridManager = InGridManager;
	SpawnCell = InSpawnCell;
	GridCellHeadPosition = InSpawnCell;
}

bool ASnakeGridwalkerPawn::TryGetBodyCellPositionByIndex(int32 SegmentIndex, FIntPoint& OutCell) const
{
	if (!BodyCells.IsValidIndex(SegmentIndex))
	{
		return false;
	}

	OutCell = BodyCells[SegmentIndex];
	return true;
}

bool ASnakeGridwalkerPawn::TryFindBodyIndexAtCell(const FIntPoint& Cell, int32& OutSegmentIndex) const
{
	// .IndexOfByKey returns first match, fine for snake where each body cell should be unique 
	OutSegmentIndex = BodyCells.IndexOfByKey(Cell);
	return OutSegmentIndex != INDEX_NONE;
}

void ASnakeGridwalkerPawn::StartMovement()
{
	if (!bIsAlive)
	{
		return;
	}

	bMovementActive = true;
	bMovementPaused = false;
	StepAccumulator = 0.0f;
}

void ASnakeGridwalkerPawn::StopMovement()
{
	bMovementActive = false;
	//bMovementPaused = false;
	StepAccumulator = 0.0f;
}

void ASnakeGridwalkerPawn::SetMovementPaused(bool bPaused)
{
	bMovementPaused = bPaused;
}

void ASnakeGridwalkerPawn::ResetSnake()
{
	if (!GridManager)
	{
		UE_LOG(LogTemp, Warning, TEXT("Cannot reset snake: no GridManager assigned."));
		bIsAlive = false;
		return;
	}

	bIsAlive = true;
	bMovementActive = false;
	bMovementPaused = false;

	StepAccumulator = 0.0f;
	TurnRotationElapsed = 0.0f;
	IsTurning = false;

	ApplyStartupSettings(GetResolvedStartupSettings());

	BodyCells.Reset();

	RawMoveInput = FVector2D::ZeroVector;

	GridCellHeadPosition = SpawnCell;

	TurnStartYaw = DirectionToYaw(CurrentDirection);
	TurnTargetYaw = TurnStartYaw;

	SetActorLocation(GetHeadWorldLocationForCell(GridCellHeadPosition));

	if (SnakeHeadMesh)
	{
		SnakeHeadMesh->SetRelativeRotation(FRotator(0.0f, TurnTargetYaw, 0.0f));
		SnakeHeadMesh->SetRelativeLocation(HeadMoveVisualRestRelativeLocation);
	}

	HeadMoveVisualElapsed = 0.0f;
	bIsHeadMovingVisually = false;
	HeadMoveVisualStartRelativeLocation = HeadMoveVisualRestRelativeLocation;

	if (VisualSegmentMesh)
	{
		VisualSegmentMesh->ClearInstances();
	}
}

void ASnakeGridwalkerPawn::RequestGrowth(const int32 Amount)
{
	if (Amount <= 0)
	{
		return;
	}

	PendingGrowth += Amount;
}

void ASnakeGridwalkerPawn::ApplyVisualAssets()
{
	if (SnakeHeadMesh && HeadMeshAsset)
	{
		SnakeHeadMesh->SetStaticMesh(HeadMeshAsset);
		if (HeadMaterialAsset)
		{
			SnakeHeadMesh->SetMaterial(0, HeadMaterialAsset);
		}
	}

	if (VisualSegmentMesh && SegmentMeshAsset)
	{
		VisualSegmentMesh->SetStaticMesh(SegmentMeshAsset);
		if (SegmentMaterialAsset)
		{
			VisualSegmentMesh->SetMaterial(0, SegmentMaterialAsset);
		}
	}
}

void ASnakeGridwalkerPawn::SetupInputMapping()
{
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

void ASnakeGridwalkerPawn::ApplyStartupSettings(const FSnakeStartupSettings& Settings)
{
	StepInterval = FMath::Max(Settings.StepInterval, 0.01f);
	TurnDuration = FMath::Max(Settings.TurnDuration, 0.01f);

	CurrentDirection = Settings.StartingDirection;
	PendingNextDirection = EGridDirection::None;

	PendingGrowth = Settings.InitialPendingGrowth;
}

const FSnakeStartupSettings& ASnakeGridwalkerPawn::GetResolvedStartupSettings() const
{
	return StartupSettingsPreset
		       ? StartupSettingsPreset->StartupSettings
		       : FallbackStartupSettings;
}


FTransform ASnakeGridwalkerPawn::MakeBodyInstanceLocalTransform(const FIntPoint& BodyCell) const
{
	if (!GridManager)
	{
		// should appear at the sneak head root as fallback
		UE_LOG(LogTemp, Warning, TEXT("Snake has no GridManager assigned."));
		return FTransform::Identity;
	}

	const FVector BodyWorldLocation = GetBodyWorldLocationForCell(BodyCell);

	// take this world-space position and convert it into the snake actor's local space
	const FVector LocalLocation = GetActorTransform().InverseTransformPosition(BodyWorldLocation);

	return FTransform(FRotator::ZeroRotator, LocalLocation, FVector::OneVector);
}

EGridDirection ASnakeGridwalkerPawn::ResolveDirectionFromInput(const FVector2D& Input)
{
	// no input early out
	if (Input.IsNearlyZero())
	{
		return EGridDirection::None;
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

void ASnakeGridwalkerPawn::Input_OnMove(const FInputActionValue& Value)
{
	RawMoveInput = Value.Get<FVector2D>();

	// store active inputs only 
	const EGridDirection InputDirection = ResolveDirectionFromInput(RawMoveInput);
	if (InputDirection != EGridDirection::None)
	{
		PendingNextDirection = InputDirection;
	}
}

void ASnakeGridwalkerPawn::Input_OnGrowPressed()
{
	RequestGrowth();
}

void ASnakeGridwalkerPawn::Input_OnResetPressed()
{
	UWorld* World = GetWorld();
	if (World)
	{
		if (ASnakeGameModeBase* SnakeGameMode = World->GetAuthGameMode<ASnakeGameModeBase>())
		{
			SnakeGameMode->RestartRun();
		}
	}
}

float ASnakeGridwalkerPawn::GetHeadPlacementHalfHeight() const
{
	if (!CollisionSphere)
	{
		return 0.0f;
	}

	return CollisionSphere->GetScaledSphereRadius();
}

float ASnakeGridwalkerPawn::GetBodyPlacementHalfHeight() const
{
	return GetHeadPlacementHalfHeight(); // Temp since the segments are still only visuals
}

FVector ASnakeGridwalkerPawn::GetHeadWorldLocationForCell(const FIntPoint& Cell) const
{
	if (!GridManager)
	{
		return GetActorLocation();
	}

	FVector GridLocation = GridManager->GridToWorld(Cell);
	GridLocation.Z += GetHeadPlacementHalfHeight();
	return GridLocation;
}

FVector ASnakeGridwalkerPawn::GetBodyWorldLocationForCell(const FIntPoint& Cell) const
{
	return GetHeadWorldLocationForCell(Cell); // Temp since the segments are still only visuals
}

void ASnakeGridwalkerPawn::StartHeadTurnVisual(EGridDirection NewDirection)
{
	TurnTargetYaw = DirectionToYaw(NewDirection);
	TurnStartYaw = SnakeHeadMesh->GetRelativeRotation().Yaw;

	TurnRotationElapsed = 0.0f;
	IsTurning = true;
}

void ASnakeGridwalkerPawn::UpdateHeadTurnVisual(float DeltaTime)
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

	SnakeHeadMesh->SetRelativeRotation(FRotator(0.0f, NewYaw, 0.0f));

	// stop rotation at target value, even if it would have overshoot. 
	if (Alpha >= 1.0f) // TurnRotationElapsed >= TurnDuration
	{
		SnakeHeadMesh->SetRelativeRotation(FRotator(0.0f, TurnTargetYaw, 0.0f));
		IsTurning = false;
		TurnRotationElapsed = 0.0f;
	}

	// Look into: 
	//UKismetMathLibrary::FInterpTo();
	//UKismetMathLibrary::RInterpTo();
}

void ASnakeGridwalkerPawn::StartHeadMoveVisual(const FVector& PreviousHeadWorldLocation)
{
	if (!SnakeHeadMesh)
	{
		return;
	}

	HeadMoveVisualStartRelativeLocation =
		GetActorTransform().InverseTransformPosition(PreviousHeadWorldLocation);

	HeadMoveVisualElapsed = 0.0f;
	bIsHeadMovingVisually = true;

	SnakeHeadMesh->SetRelativeLocation(HeadMoveVisualStartRelativeLocation);
}

void ASnakeGridwalkerPawn::UpdateHeadMoveVisual(float DeltaTime)
{
	if (!bIsHeadMovingVisually || !SnakeHeadMesh)
	{
		return;
	}

	HeadMoveVisualElapsed += DeltaTime;

	const float RawAlpha = FMath::Clamp(HeadMoveVisualElapsed / StepInterval, 0.0f, 1.0f);

	// SmoothAlpha makes head starts slower, speeds up, then slows down. 
	// Note: Test how it feels compared to steady speed later.
	const float SmoothAlpha = RawAlpha * RawAlpha * (3.0f - 2.0f * RawAlpha);

	const FVector NewRelativeLocation = FMath::Lerp(
		HeadMoveVisualStartRelativeLocation,
		HeadMoveVisualRestRelativeLocation,
		SmoothAlpha);

	SnakeHeadMesh->SetRelativeLocation(NewRelativeLocation);

	if (RawAlpha >= 1.0f)
	{
		SnakeHeadMesh->SetRelativeLocation(HeadMoveVisualRestRelativeLocation);
		HeadMoveVisualElapsed = 0.0f;
		bIsHeadMovingVisually = false;
	}
}

void ASnakeGridwalkerPawn::AddBodyVisualSegment(const FIntPoint& BodyCell)
{
	if (!VisualSegmentMesh)
	{
		return;
	}

	VisualSegmentMesh->AddInstance(MakeBodyInstanceLocalTransform(BodyCell));
}

void ASnakeGridwalkerPawn::UpdateBodyVisualTransforms()
{
	if (!VisualSegmentMesh)
	{
		return;
	}

	const int32 VisualSegmentCount = VisualSegmentMesh->GetInstanceCount();
	const int32 LogicCount = BodyCells.Num();
	const int32 CountToUpdate = FMath::Min(VisualSegmentCount, LogicCount);

	for (int32 Index = 0; Index < CountToUpdate; ++Index)
	{
		const FTransform NewTransform = MakeBodyInstanceLocalTransform(BodyCells[Index]);
		const bool bIsLastUpdate = (Index == CountToUpdate - 1);

		VisualSegmentMesh->UpdateInstanceTransform(
			Index,
			NewTransform,
			false,
			bIsLastUpdate,
			true);
	}
}

void ASnakeGridwalkerPawn::SyncBodyVisuals()
{
	if (!VisualSegmentMesh)
	{
		return;
	}

	int32 VisualSegmentCount = VisualSegmentMesh->GetInstanceCount();
	const int32 LogicCount = BodyCells.Num();

	while (VisualSegmentCount < LogicCount)
	{
		AddBodyVisualSegment(BodyCells[VisualSegmentCount]);
		++VisualSegmentCount;
	}

	while (VisualSegmentCount > LogicCount)
	{
		VisualSegmentMesh->RemoveInstance(VisualSegmentCount - 1);
		--VisualSegmentCount;
	}

	UpdateBodyVisualTransforms();
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

EGridDirection ASnakeGridwalkerPawn::DetermineDesiredDirection() const
{
	// Choose which direction this step should try to use.
	if (PendingNextDirection != EGridDirection::None &&
		!Reversing(CurrentDirection, PendingNextDirection))
	{
		return PendingNextDirection;
	}

	return CurrentDirection;
}

FIntPoint ASnakeGridwalkerPawn::PeekNextHeadCell(const EGridDirection Direction) const
{
	return GridCellHeadPosition + GridDelta(Direction);
}


bool ASnakeGridwalkerPawn::CheckForCollision(const FIntPoint& NextHeadCell)
{
	// check for grid obstacle / out of bounds
	if (GridManager->IsCellBlockedByBoard(NextHeadCell))
	{
		HandleDeath();
		return true;
	}

	// can only enter tail cell if it is going to be free by next step,
	// if there is no planned growth, then the absolute last tail segment will not be here after movement has been applied
	const bool bCanMoveIntoCurrentTailspace =
		BodyCells.Num() > 0 &&
		NextHeadCell == BodyCells.Last() &&
		PendingGrowth <= 0;

	// check for self collision
	if (BodyCells.Contains(NextHeadCell) && !bCanMoveIntoCurrentTailspace)
	{
		HandleDeath();
		return true;
	}

	return false;
}

void ASnakeGridwalkerPawn::HandleDeath()
{
	if (!bIsAlive)
	{
		UE_LOG(LogTemp, Warning, TEXT("Snake triggered HandleDeath() after death "));
		return;
	}

	bIsAlive = false;
	OnSnakeDied.Broadcast(this);
}

void ASnakeGridwalkerPawn::UpdateHeadWorldLocation(const FIntPoint& NextHeadCell)
{
	if (!GridManager)
	{
		UE_LOG(LogTemp, Warning, TEXT("Cannot reset snake: no GridManager assigned."));
		bIsAlive = false;
		return;
	}

	const FVector PreviousHeadWorldLocation = GetActorLocation();
	const FVector NewHeadWorldLocation = GetHeadWorldLocationForCell(NextHeadCell);

	SetActorLocation(NewHeadWorldLocation);
	StartHeadMoveVisual(PreviousHeadWorldLocation);
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
	if (!GridManager)
	{
		UE_LOG(LogTemp, Warning, TEXT("Cannot advance snake: no GridManager assigned."));
		bIsAlive = false;
		return;
	}

	/*
	// maybe check for death state here instead of the early out return below, to let snake walk into walls visually 
	if (!bIsAlive)
	{
		return;
	}
	*/

	const EGridDirection DesiredDirection = DetermineDesiredDirection();
	const FIntPoint NextHeadCell = PeekNextHeadCell(DesiredDirection);

	// consume input after registering it 
	PendingNextDirection = EGridDirection::None;

	if (CheckForCollision(NextHeadCell))
	{
		return;
	}

	const bool bDirectionChanged = CurrentDirection != DesiredDirection;
	const FIntPoint PreviousHeadCell = GridCellHeadPosition;
	const FIntPoint PreviousTailCell = BodyCells.Num() > 0 ? BodyCells.Last() : GridCellHeadPosition;

	// --- Commit gameplay truth / Logic state, ---
	CurrentDirection = DesiredDirection;
	GridCellHeadPosition = NextHeadCell;

	AdvanceBodySegments(PreviousHeadCell);
	const bool bGrewThisStep = TryConsumeGrowth();
	if (bGrewThisStep)
	{
		BodyCells.Add(PreviousTailCell);
	}

	// --- Update Visual / World-Sync after all logic ---
	if (bDirectionChanged)
	{
		StartHeadTurnVisual(DesiredDirection);
	}
	UpdateHeadWorldLocation(NextHeadCell);

	if (bGrewThisStep)
	{
		AddBodyVisualSegment(PreviousTailCell);
	}
	UpdateBodyVisualTransforms();
}
