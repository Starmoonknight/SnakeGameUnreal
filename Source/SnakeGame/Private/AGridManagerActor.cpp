// Fill out your copyright notice in the Description page of Project Settings.


#include "AGridManagerActor.h"
#include "ASnakeGridwalkerPawn.h"
#include "AFoodActor.h"

#include "Components/SceneComponent.h"
#include "Components/StaticMeshComponent.h"
#include  "Components/InstancedStaticMeshComponent.h"


namespace
{
	int32 FoFlatIndex(const FIntPoint& Cell, const FIntPoint& GridDimensions)
	{
		return Cell.Y * GridDimensions.X + Cell.X;
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

	RebuildGridVisuals();
}

// Called when the game starts or when spawned
void AAGridManagerActor::BeginPlay()
{
	Super::BeginPlay();

	RandomStream.Initialize(RandomSeed);
	InitializeCells();
	RespawnFruit_Temp();
}

// Called every frame
void AAGridManagerActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
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

FVector AAGridManagerActor::CellToWorld(const FIntPoint Cell) const
{
	return FVector(
		GridOrigin.X + (Cell.X * CellSize),
		GridOrigin.Y + (Cell.Y * CellSize),
		GridWorldZ);
}


FVector2D AAGridManagerActor::GetBoardWorldSize() const
{
	const FVector2D BoardWorldSize(
		GridDimensions.X * CellSize,
		GridDimensions.Y * CellSize);

	return BoardWorldSize;
}

FVector AAGridManagerActor::GetBoardWorldCenter() const
{
	const FVector BoardCenter(
		GridOrigin.X + ((GridDimensions.X - 1) * CellSize * 0.5f),
		GridOrigin.Y + ((GridDimensions.Y - 1) * CellSize * 0.5f),
		GridWorldZ);

	return BoardCenter;
}

void AAGridManagerActor::CalculateBoardSize()
{
	const float BoardWidth = GridDimensions.X * CellSize;
	const float BoardHeight = GridDimensions.Y * CellSize;
	const float HalfCell = CellSize / 2.0f;
	const float WallHeight = CellSize;
	const float WallThickness = CellSize;
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
}

void AAGridManagerActor::RebuildGridVisuals()
{
	FVector2D BoardWorldSize = GetBoardWorldSize();
	FVector BoardWorldCenter = GetBoardWorldCenter();

	FVector
	NorthWallPlacement.Y = BoardWorldCenter.Y + BoardWorldSize.Y * 0.5f + WallThickness * 0.5f;
	NorthWallPlacement.Z = GridWorldZ + WallHeight * 0.5f;
}
