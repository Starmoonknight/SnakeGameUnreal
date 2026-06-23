// Fill out your copyright notice in the Description page of Project Settings.


#include "GridManagerActor.h"

#include "Components/SceneComponent.h"
#include "Components/StaticMeshComponent.h"
#include  "Components/InstancedStaticMeshComponent.h"


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
AGridManagerActor::AGridManagerActor()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
	SetRootComponent(SceneRoot);

	FloorVisuals = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("FloorMesh"));
	FloorVisuals->SetupAttachment(SceneRoot);
	FloorVisuals->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	FloorVisuals->SetSimulatePhysics(false);

	// Boarder walls 
	NorthWallVisual = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("NorthWallMesh"));

	SouthWallVisual = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("SouthWallMesh"));

	EastWallVisual = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("EastWallMesh"));

	WestWallVisual = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("WestWallMesh"));

	// Settings for the boarder walls 
	UStaticMeshComponent* BoundaryWalls[] =
	{
		NorthWallVisual,
		SouthWallVisual,
		EastWallVisual,
		WestWallVisual
	};

	for (UStaticMeshComponent* Wall : BoundaryWalls)
	{
		Wall->SetupAttachment(SceneRoot);
		Wall->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
		Wall->SetCollisionObjectType(ECC_WorldStatic);
		Wall->SetCollisionResponseToAllChannels(ECR_Ignore);
		Wall->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
		Wall->SetGenerateOverlapEvents(true);
		Wall->SetSimulatePhysics(false);
		Wall->ComponentTags.Add(TEXT("SnakeWall"));
	}


	// internal Walls
	WallInstances = CreateDefaultSubobject<UInstancedStaticMeshComponent>(TEXT("GeneratedWallInstances"));

	WallInstances->SetupAttachment(SceneRoot);
	WallInstances->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	WallInstances->SetCollisionObjectType(ECC_WorldStatic);
	WallInstances->SetCollisionResponseToAllChannels(ECR_Ignore);
	WallInstances->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	WallInstances->SetGenerateOverlapEvents(true);
	WallInstances->ComponentTags.Add(TEXT("SnakeWall"));
}

void AGridManagerActor::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);

	SetupGridVisuals_Stretchy();
}

// Called when the game starts or when spawned
void AGridManagerActor::BeginPlay()
{
	Super::BeginPlay();

	// remove soon, game manager will handle calling the InitializeCells
	InitializeCells();
}

// Called every frame
void AGridManagerActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

FIntPoint AGridManagerActor::GetSnakeSpawnWorldLocation() const
{
	if (!IsValid(SnakeSpawnPoint))
	{
		return FIntPoint::ZeroValue;
	}

	return WorldToGrid(SnakeSpawnPoint->GetActorLocation());
}

bool AGridManagerActor::IsInBounds(const FIntPoint& Cell) const
{
	return (Cell.X >= 0 && Cell.X < GridDimensions.X)
		&& (Cell.Y >= 0 && Cell.Y < GridDimensions.Y);
}

FVector AGridManagerActor::GridToWorld(const FIntPoint& Cell) const
{
	return FVector(
		GridOrigin.X + (Cell.X * CellSize),
		GridOrigin.Y + (Cell.Y * CellSize),
		GridWorldZ);
}

FIntPoint AGridManagerActor::WorldToGrid(const FVector& WorldLocation) const
{
	const float LocalX = (WorldLocation.X - GridOrigin.X) / CellSize;
	const float LocalY = (WorldLocation.Y - GridOrigin.Y) / CellSize;

	// RoundToInt picks the nearest grid cell, FloorToInt always rounds downwards
	return FIntPoint(
		FMath::RoundToInt(LocalX),
		FMath::RoundToInt(LocalY));
}

bool AGridManagerActor::IsCellBlockedByBoard(const FIntPoint& Cell) const
{
	if (!IsInBounds(Cell))
	{
		return true;
	}

	return Cells[FlatIndex(Cell)] == EGridCellType::Blocked;
}

void AGridManagerActor::InitializeGridForGameplay()
{
	InitializeCells();
	GenerateInternalWalls();
}

void AGridManagerActor::ApplyRuntimeSettings(const FGridStartupSettings& Settings)
{
	GridDimensions.X = FMath::Max(Settings.GridDimensions.X, 3);
	GridDimensions.Y = FMath::Max(Settings.GridDimensions.Y, 3);

	GridOrigin = Settings.GridOrigin;
	GridWorldZ = Settings.GridWorldZ;
	CellSize = FMath::Max(Settings.CellSize, 1);
	InternalWallSpacing = FMath::Max(Settings.InternalWallSpacing, 0);
	RootSeed = Settings.RootSeed;

	SetupGridVisuals_Stretchy();
	InitializeGridForGameplay();
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


int32 AGridManagerActor::FlatIndex(const FIntPoint& Cell) const
{
	check(GridDimensions.X > 0);
	check(IsInBounds(Cell));

	return Cell.Y * GridDimensions.X + Cell.X;
}

FIntPoint AGridManagerActor::IndexToCellCoord(const int32 Index) const
{
	check(GridDimensions.X > 0);
	check(GridDimensions.Y > 0);
	check(Index >= 0);
	check(Index < GridDimensions.X * GridDimensions.Y);

	const int32 X = Index % GridDimensions.X;
	const int32 Y = Index / GridDimensions.X;

	return FIntPoint(X, Y);
}

UStaticMesh* AGridManagerActor::GetWallMeshToUse() const
{
	if (bHasFancyWalls && WallMeshAsset)
	{
		return WallMeshAsset;
	}

	return FallbackCubeMesh;
}

UStaticMesh* AGridManagerActor::GetInnerWallMeshToUse() const
{
	if (bHasFancyWalls && InnerWallMeshAsset)
	{
		return InnerWallMeshAsset;
	}

	return FallbackCubeMesh;
}

UStaticMesh* AGridManagerActor::GetFloorMeshToUse() const
{
	if (bHasFancyFloor && FloorMeshAsset)
	{
		return FloorMeshAsset;
	}

	return FallbackPlaneMesh;
}


void AGridManagerActor::InitializeCells()
{
	const int32 CellCount = GridDimensions.X * GridDimensions.Y;
	Cells.SetNum(CellCount);

	for (EGridCellType& CellType : Cells)
	{
		CellType = EGridCellType::Empty;
	}
}

void AGridManagerActor::GenerateInternalWalls()
{
	if (!WallInstances)
	{
		return;
	}

	WallInstances->ClearInstances();
	WallInstances->SetStaticMesh(GetInnerWallMeshToUse());

	if (InternalWallSpacing < 2)
	{
		return;
	}

	const float MeshScale = static_cast<float>(CellSize) / 100.0f;

	// Generate one vertical wall stripe every InternalWallSpacing columns
	for (int32 X = InternalWallSpacing; X < GridDimensions.X - 1; X += InternalWallSpacing)
	{
		// Should leave two free cells near the top and bottom of the playable grid
		for (int32 Y = 2; Y < GridDimensions.Y - 2; ++Y)
		{
			// Alternate openings between one-third and two-thirds of the grid height,
			// should make it so openings don't line up on same row and keep every generated stripe navigable
			const int32 OpeningY =
				((X / InternalWallSpacing) % 2 == 0)
					? GridDimensions.Y / 3
					: (GridDimensions.Y * 2) / 3;

			// Skip the openings + one neighboring cell on each side, should make three cell wide gaps 
			if (FMath::Abs(Y - OpeningY) <= 1)
			{
				continue;
			}

			// ALSO NEED TO ADD SO WALLS DON'T SPAWN ON SNAEK 

			const FIntPoint Cell(X, Y);

			// update so game knows the cell-tile is blocked for game logic 
			Cells[FlatIndex(Cell)] = EGridCellType::Blocked;

			// get the values for where to spawn the wall visuals 
			FVector WorldLocation = GridToWorld(Cell);
			WorldLocation.Z += CellSize * 0.5f;

			const FVector LocalLocation = GetActorTransform().InverseTransformPosition(WorldLocation);

			WallInstances->AddInstance(FTransform(FRotator::ZeroRotator, LocalLocation, FVector(MeshScale)));
		}
	}
}

void AGridManagerActor::BuildTiledFloor()
{
	// way to make checkered material look here. 
}

void AGridManagerActor::SetupGridVisuals_Stretchy()
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
