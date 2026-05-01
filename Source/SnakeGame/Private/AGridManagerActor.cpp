// Fill out your copyright notice in the Description page of Project Settings.


#include "AGridManagerActor.h"

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

	// remove soon, game manager will handle calling the InitializeCells
	InitializeCells();
}

// Called every frame
void AAGridManagerActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

FVector AAGridManagerActor::GetSnakeSpawnPoint() const
{
	if (!IsValid(SnakeSpawnPoint))
	{
		return FVector::ZeroVector;
	}

	return SnakeSpawnPoint->GetActorLocation();
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

void AAGridManagerActor::InitializeGridForGameplay()
{
	InitializeCells();
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
