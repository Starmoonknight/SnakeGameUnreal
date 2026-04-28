// Fill out your copyright notice in the Description page of Project Settings.


#include "AGridManagerActor.h"
#include "ASnakeGridwalkerPawn.h"
#include "AFoodActor.h"

#include "Components/SceneComponent.h"
#include "Components/StaticMeshComponent.h"
#include  "Components/InstancedStaticMeshComponent.h"
#include "Kismet/GameplayStatics.h"


namespace
{
	struct FBoardBuildData
	{
		FVector2D BoardWorldSize = FVector2D::ZeroVector;
		FVector BoardWorldCenter = FVector::ZeroVector;

		float BoardWidth = 0.f;
		float BoardHeight = 0.f;
		float HalfCell = 0.f;
		float WallHeight = 0.f;
		float WallThickness = 0.f;
	};

	constexpr uint32 FruitSeedTag = 0xF17E1234u; // a random hexadecimal, just need to be distinct 
	//RootSeed + FruitTag -> fruit seed
	//RootSeed + WallTag  -> wall seed
	//RootSeed + EnemyTag -> enemy seed

	// Deterministic scrambling function, takes two numbers and mixes them into a new number
	uint32 MixSeed(uint32 Seed, uint32 Tag)
	{
		Seed ^= Tag + 0x9E3779B9u + (Seed << 6) + (Seed >> 2);
		Seed ^= Seed >> 16;
		Seed *= 0x7FEB352Du;
		Seed ^= Seed >> 15;
		Seed *= 0x846CA68Bu;
		Seed ^= Seed >> 16;
		return Seed;
	}

	int32 MakeSubsystemSeed(int32 RootSeed, uint32 SubsystemTag)
	{
		const uint32 Mixed = MixSeed(static_cast<uint32>(RootSeed), SubsystemTag);

		return static_cast<int32>(Mixed & 0x7FFFFFFFu);
	}


	FBoardBuildData BuildBoardData(
		const FIntPoint& GridDimensions,
		const FVector2D& GridOrigin,
		float CellSize,
		float GridWorldZ)
	{
		FBoardBuildData Data;

		Data.BoardWorldSize = FVector2D(
			GridDimensions.X * CellSize,
			GridDimensions.Y * CellSize);

		Data.BoardWorldCenter = FVector(
			GridOrigin.X + ((GridDimensions.X - 1) * CellSize * 0.5f),
			GridOrigin.Y + ((GridDimensions.Y - 1) * CellSize * 0.5f),
			GridWorldZ);

		Data.BoardWidth = GridDimensions.X * CellSize;
		Data.BoardHeight = GridDimensions.Y * CellSize;
		Data.HalfCell = CellSize / 2.0f;
		Data.WallHeight = CellSize;
		Data.WallThickness = CellSize;

		return Data;
	}
}


// Sets default values
AAGridManagerActor::AAGridManagerActor()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
	SetRootComponent(SceneRoot);

	FloorVisuals = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("FloorMesh"));
	FloorVisuals->SetupAttachment(SceneRoot);
	FloorVisuals->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	FloorVisuals->SetSimulatePhysics(false);

	NorthWallVisual = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("NorthWallMesh"));
	NorthWallVisual->SetupAttachment(SceneRoot);
	NorthWallVisual->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	NorthWallVisual->SetSimulatePhysics(false);

	SouthWallVisual = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("SouthWallMesh"));
	SouthWallVisual->SetupAttachment(SceneRoot);
	SouthWallVisual->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	SouthWallVisual->SetSimulatePhysics(false);

	EastWallVisual = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("EastWallMesh"));
	EastWallVisual->SetupAttachment(SceneRoot);
	EastWallVisual->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	EastWallVisual->SetSimulatePhysics(false);

	WestWallVisual = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("WestWallMesh"));
	WestWallVisual->SetupAttachment(SceneRoot);
	WestWallVisual->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	WestWallVisual->SetSimulatePhysics(false);
}

void AAGridManagerActor::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);

	SetupGridVisuals_Stretchy();
}

// Called when the game starts or when spawned
void AAGridManagerActor::BeginPlay()
{
	Super::BeginPlay();

	FoodRandomStream.Initialize(MakeSubsystemSeed(RootSeed, FruitSeedTag));

	InitializeCells();

	PlaceSnakeOnGrid();
	RespawnFruit_Temp();

	StartGameLoop();
}

// Called every frame
void AAGridManagerActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

bool AAGridManagerActor::IsInBounds(const FIntPoint& Cell) const
{
	return (Cell.X >= 0 && Cell.X < GridDimensions.X)
		&& (Cell.Y >= 0 && Cell.Y < GridDimensions.Y);
}

FVector AAGridManagerActor::CellToWorld(const FIntPoint& Cell) const
{
	return FVector(
		GridOrigin.X + (Cell.X * CellSize),
		GridOrigin.Y + (Cell.Y * CellSize),
		GridWorldZ);
}

FIntPoint AAGridManagerActor::WorldToCell(const FVector& WorldLocation) const
{
	const float LocalX = (WorldLocation.X - GridOrigin.X) / CellSize;
	const float LocalY = (WorldLocation.Y - GridOrigin.Y) / CellSize;

	// RoundToInt picks the nearest grid cell, FloorToInt always rounds downwards
	return FIntPoint(
		FMath::RoundToInt(LocalX),
		FMath::RoundToInt(LocalY));
}

bool AAGridManagerActor::IsCellBlockedByBoard(const FIntPoint& Cell) const
{
	if (!IsInBounds(Cell))
	{
		return true;
	}

	return Cells[FlatIndex(Cell)] == EGridCellType::Blocked;
}

bool AAGridManagerActor::IsFoodAtCell_Temp(const FIntPoint& Cell) const
{
	return IsValid(CurrentFood)
		&& CurrentFood->IsActive()
		&& CurrentFood->GetFoodGridPosition() == Cell;
}


/*
FlatIndex()
IndexToCoord()

These formulas assume:

GridDimensions.X > 0
Cell.X >= 0
Cell.Y >= 0
Cell.X < GridDimensions.X
Cell.Y < GridDimensions.Y
Index >= 0
Index < GridDimensions.X * GridDimensions.Y
*/


void AAGridManagerActor::RespawnFruit_Temp()
{
	FIntPoint NewFoodCell;
	if (!TryFindRandomFreeCell_Temp(NewFoodCell))
	{
		UE_LOG(LogTemp, Warning, TEXT("No valid free cell found for fruit."));
		return;
	}

	SpawnFruitAtCellDestructive_Temp(NewFoodCell);
}

void AAGridManagerActor::HandleFruitConsumed_Temp(AAFoodActor* Food, AActor* ConsumerActor)
{
	if (Food != CurrentFood)
	{
		return;
	}

	CurrentFood = nullptr;
	RespawnFruit_Temp();
}

int32 AAGridManagerActor::FlatIndex(const FIntPoint& Cell) const
{
	check(GridDimensions.X > 0);
	check(IsInBounds(Cell));

	return Cell.Y * GridDimensions.X + Cell.X;
}

FIntPoint AAGridManagerActor::IndexToCellCoord(const int32 Index) const
{
	check(GridDimensions.X > 0);
	check(GridDimensions.Y > 0);
	check(Index >= 0);
	check(Index < GridDimensions.X * GridDimensions.Y);

	const int32 X = Index % GridDimensions.X;
	const int32 Y = Index / GridDimensions.X;

	return FIntPoint(X, Y);
}

UStaticMesh* AAGridManagerActor::GetWallMeshToUse() const
{
	if (bHasFancyWalls && WallMeshAsset)
	{
		return WallMeshAsset;
	}

	return FallbackCubeMesh;
}

UStaticMesh* AAGridManagerActor::GetFloorMeshToUse() const
{
	if (bHasFancyFloor && FloorMeshAsset)
	{
		return FloorMeshAsset;
	}

	return FallbackPlaneMesh;
}

void AAGridManagerActor::PlaceSnakeOnGrid()
{
	if (IsValid(Snake))
	{
		return;
	}

	if (!SnakeClass)
	{
		UE_LOG(LogTemp, Warning, TEXT("GridManager has no SnakeClass assigned."));
		return;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	FIntPoint SpawnCell = FIntPoint::ZeroValue;

	if (SnakeSpawnPoint)
	{
		SpawnCell = WorldToCell(SnakeSpawnPoint->GetActorLocation());
	}

	// need to align to propper cell placement first
	if (!IsInBounds(SpawnCell))
	{
		UE_LOG(LogTemp, Warning, TEXT("Snake spawn location was outside the grid. Falling back to cell 0,0."));
		SpawnCell = FIntPoint::ZeroValue;
	}

	const FVector SpawnLocation = CellToWorld(SpawnCell);
	const FRotator SpawnRotator = FRotator::ZeroRotator;
	const FTransform SpawnTransform(SpawnRotator, SpawnLocation);

	AASnakeGridwalkerPawn* NewSnake =
		World->SpawnActorDeferred<AASnakeGridwalkerPawn>(
			SnakeClass,
			SpawnTransform,
			nullptr,
			nullptr,
			ESpawnActorCollisionHandlingMethod::AlwaysSpawn);

	if (!NewSnake)
	{
		UE_LOG(LogTemp, Warning, TEXT("Failed to spawn snake."));
		return;
	}

	NewSnake->ConfigureForGrid(this, SpawnCell);

	UGameplayStatics::FinishSpawningActor(NewSnake, SpawnTransform);

	Snake = NewSnake;
}

bool AAGridManagerActor::CanPlaceFruitAtCell_Temp(const FIntPoint& Cell) const
{
	if (!IsInBounds(Cell))
	{
		return false;
	}

	const bool bBlocked =
		IsCellBlockedByBoard(Cell)
		|| IsFoodAtCell_Temp(Cell)
		|| (Snake && Snake->IsSnakeAtCell(Cell));

	return !bBlocked;
}

bool AAGridManagerActor::TryFindRandomFreeCell_Temp(FIntPoint& OutCell)
{
	TArray<FIntPoint> CandidateCells;
	CandidateCells.Reserve(GetCellCount());

	for (int32 Y = 0; Y < GridDimensions.Y; Y++)
	{
		for (int32 X = 0; X < GridDimensions.X; X++)
		{
			const FIntPoint Cell(X, Y);

			if (CanPlaceFruitAtCell_Temp(Cell))
			{
				CandidateCells.Add(Cell);
			}
		}
	}

	if (CandidateCells.IsEmpty())
	{
		return false;
	}

	const int32 RandomIndex = FoodRandomStream.RandRange(0, CandidateCells.Num() - 1);
	OutCell = CandidateCells[RandomIndex];

	return true;
}

// can only be one fruit at this stage, so destroys previous if still active 

void AAGridManagerActor::SpawnFruitAtCellDestructive_Temp(const FIntPoint& Cell)
{
	if (!FoodClass)
	{
		UE_LOG(LogTemp, Warning, TEXT("GridManager has no FoodClass assigned."));
		return;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	if (IsValid(CurrentFood))
	{
		CurrentFood->Destroy();
		CurrentFood = nullptr;
	}

	FVector SpawnLocation = CellToWorld(Cell);

	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = this;

	// will be placed inside floor at first
	CurrentFood = World->SpawnActor<AAFoodActor>(
		FoodClass,
		SpawnLocation,
		FRotator::ZeroRotator,
		SpawnParams);

	if (!CurrentFood)
	{
		UE_LOG(LogTemp, Warning, TEXT("Failed to spawn food."));
		return;
	}

	// can only query the spawned actor's component size after it exists.
	SpawnLocation.Z += CurrentFood->GetPlacementHalfHeight();

	CurrentFood->SetFoodGridPosition(Cell, SpawnLocation);
	CurrentFood->SetFoodValues(1, 1);
	CurrentFood->SetActiveStatus(true);

	CurrentFood->OnFruitConsumed.AddDynamic(this, &AAGridManagerActor::HandleFruitConsumed_Temp);
}

void AAGridManagerActor::InitializeCells()
{
	const int32 CellCount = GridDimensions.X * GridDimensions.Y;
	Cells.SetNum(CellCount);

	for (EGridCellType& CellType : Cells)
	{
		CellType = EGridCellType::Empty;
	}
}

void AAGridManagerActor::BuildTiledFloor()
{
	// way to make checkered material look here. 
}

void AAGridManagerActor::SetupGridVisuals_Stretchy()
{
	// gather derived values 
	const FBoardBuildData GridData = BuildBoardData(
		GridDimensions, GridOrigin, CellSize, GridWorldZ);

	UStaticMesh* WallMeshToUse = GetWallMeshToUse();
	UStaticMesh* FloorMeshToUse = GetFloorMeshToUse();

	// select what meshes to use
	FloorVisuals->SetStaticMesh(FloorMeshToUse);
	NorthWallVisual->SetStaticMesh(WallMeshToUse);
	SouthWallVisual->SetStaticMesh(WallMeshToUse);
	EastWallVisual->SetStaticMesh(WallMeshToUse);
	WestWallVisual->SetStaticMesh(WallMeshToUse);

	// scaling floor size with assumption fallback meshes are built so scale 1 = 1 cell (100 units)
	FloorVisuals->SetWorldLocation(GridData.BoardWorldCenter);
	FloorVisuals->SetWorldScale3D(FVector(
		GridDimensions.X,
		GridDimensions.Y,
		1.0f));

	// same size assumption for wall meshes, just scale to fit 
	// place wall objects  

	// North Wall: Y = CenterY + HalfBoardHeight + HalfWallThickness
	const FVector NorthLocation(
		GridData.BoardWorldCenter.X,
		GridData.BoardWorldCenter.Y + (GridData.BoardHeight * 0.5f) + (GridData.WallThickness * 0.5f),
		GridWorldZ + (GridData.WallHeight * 0.5f));

	NorthWallVisual->SetWorldScale3D(FVector(GridDimensions.X, 1.0f, 1.0f));
	NorthWallVisual->SetWorldLocation(NorthLocation);

	// South wall: Y = CenterY - HalfBoardHeight - HalfWallThickness
	const FVector SouthLocation(
		GridData.BoardWorldCenter.X,
		GridData.BoardWorldCenter.Y - (GridData.BoardHeight * 0.5f) - (GridData.WallThickness * 0.5f),
		GridWorldZ + (GridData.WallHeight * 0.5f));

	SouthWallVisual->SetWorldScale3D(FVector(GridDimensions.X, 1.0f, 1.0f));
	SouthWallVisual->SetWorldLocation(SouthLocation);

	// East wall: X = CenterX + HalfBoardWidth + HalfWallThickness
	const FVector EastLocation(
		GridData.BoardWorldCenter.X + (GridData.BoardWidth * 0.5f) + (GridData.WallThickness * 0.5f),
		GridData.BoardWorldCenter.Y,
		GridWorldZ + (GridData.WallHeight * 0.5f));

	EastWallVisual->SetWorldScale3D(FVector(1.0f, GridDimensions.Y, 1.0f));
	EastWallVisual->SetWorldLocation(EastLocation);

	// West wall: X = CenterX - HalfBoardWidth - HalfWallThickness
	const FVector WestLocation(
		GridData.BoardWorldCenter.X - (GridData.BoardWidth * 0.5f) - (GridData.WallThickness * 0.5f),
		GridData.BoardWorldCenter.Y,
		GridWorldZ + (GridData.WallHeight * 0.5f));

	WestWallVisual->SetWorldScale3D(FVector(1.0f, GridDimensions.Y, 1.0f));
	WestWallVisual->SetWorldLocation(WestLocation);
}

void AAGridManagerActor::StartGameLoop()
{
	if (!IsValid(Snake))
	{
		UE_LOG(LogTemp, Warning, TEXT("Cannot start game loop: no snake."));
		return;
	}

	Snake->StartMovement();
}
