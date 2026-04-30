// Fill out your copyright notice in the Description page of Project Settings.


#include "ASnakeGameModeBase.h"
//#include "SnakeGameInstance.h"
#include "ASnakeGameState.h"
#include "ASnakeGridwalkerPawn.h"
#include "AFoodActor.h"
#include "AGridManagerActor.h"
//#include "SnakeGameTypes.h"
#include "ASnakeGameModeBase.h"

#include "SnakeSettingsTypes.h"
#include "GridSettingsTypes.h"

#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"


namespace
{
	constexpr uint32 DefaultSeedTag = 0xA7F3C9B2;
	constexpr uint32 FruitSeedTag = 0xF17E1234u;
	// a random hexadecimal, just need to be distinct 
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
}


void AASnakeGameModeBase::BeginPlay()
{
	Super::BeginPlay();

	FoodRandomStream.Initialize(MakeSubsystemSeed(RootSeed, FruitSeedTag));
	DefaultRandomStream.Initialize(MakeSubsystemSeed(RootSeed, DefaultSeedTag));

	CacheGridManager();

	if (!GridManager)
	{
		UE_LOG(LogTemp, Warning, TEXT("SnakeGameMode could not find GridManager."));
		return;
	}

	GridManager->InitializeGridForGameplay();

	SpawnSnake();
	RespawnFruit_Temp();

	StartGameLoop();
}

void AASnakeGameModeBase::CacheGridManager()
{
	GridManager = Cast<AAGridManagerActor>(UGameplayStatics::GetActorOfClass(this, AAGridManagerActor::StaticClass()));
}

AASnakeGameState* AASnakeGameModeBase::GetSnakeGameState()
{
	return GetGameState<AASnakeGameState>();
}

// IN PROGRESS
void AASnakeGameModeBase::StartPlayingRun()
{
	//BattleResult = ESnakeBattleResult::None;

	if (ASnakeGameState* GS = GetSnakeGameState())
	{
		GS->Score = 0;
		GS->SetMatchPhase(ESnakeMatchPhase::Playing);
	}

	//LoadStage(0);
}

void AASnakeGameModeBase::RestartRun()
{
	StartGameLoop();
}


bool AASnakeGameModeBase::IsFoodAtCell(const FIntPoint& Cell) const
{
	return IsValid(SpawnedFoodActor)
		&& SpawnedFoodActor->IsActive()
		&& SpawnedFoodActor->GetFoodGridPosition() == Cell;
}


void AASnakeGameModeBase::RespawnFruit_Temp()
{
	FIntPoint NewFoodCell;

	TArray<FIntPoint> ForbiddenCells;
	ForbiddenCells.Append(GetAllSnakeOccupiedCells());

	if (!TryFindRandomFreeCell_Flexible(NewFoodCell, ForbiddenCells, FoodRandomStream))
	{
		UE_LOG(LogTemp, Warning, TEXT("No valid free cell found for fruit."));
		return;
	}

	SpawnFruit_Destructive(NewFoodCell);
}


TArray<FIntPoint> AASnakeGameModeBase::GetAllSnakeOccupiedCells() const
{
	TArray<FIntPoint> OccupiedCells;

	if (!IsValid(SpawnedSnakePawn))
	{
		return OccupiedCells;
	}

	OccupiedCells.Add(SpawnedSnakePawn->GetHeadCellPosition());
	OccupiedCells.Append(SpawnedSnakePawn->GetBodyCellPositions());

	return OccupiedCells;

	// for later when SpawnedSnakes gets set: 
	/*
	for (const AASnakeGridwalkerPawn* Snake : SpawnedSnakes)
	{
		if (!Snake)
		{
			continue;
		}

		OccupiedCells.Add(Snake->GetHeadCellPosition());
		OccupiedCells.Append(Snake->GetBodyCellPositions());
	}

	return OccupiedCells;
	*/
}

bool AASnakeGameModeBase::AnySnakeOnThisCell(const FIntPoint& Cell) const
{
	return IsValid(SpawnedSnakePawn)
		&& SpawnedSnakePawn->IsSnakeAtCell(Cell);

	// for later when SpawnedSnakes gets set: 
	/*
	for (const AASnakeGridwalkerPawn* Snake : SpawnedSnakes)
	{
		if (!Snake)
		{
			continue;
		}

		if (Snake->IsSnakeAtCell(Cell))
		{
			return true;
		}
	}

	return false;
	*/
}

bool AASnakeGameModeBase::IsCellFreeForGameplay(const FIntPoint& Cell) const
{
	if (!GridManager || !GridManager->IsInBounds(Cell))
	{
		return false;
	}

	const bool bBlocked =
		GridManager->IsCellBlockedByBoard(Cell)
		|| IsFoodAtCell(Cell)
		|| AnySnakeOnThisCell(Cell);

	return !bBlocked;
}

bool AASnakeGameModeBase::TryFindRandomFreeCell(FIntPoint& OutCell)
{
	TArray<FIntPoint> ForbiddenCells;

	ForbiddenCells.Append(GetAllSnakeOccupiedCells());

	if (IsValid(SpawnedFoodActor))
	{
		ForbiddenCells.Add(SpawnedFoodActor->GetFoodGridPosition());
	}

	return TryFindRandomFreeCell_Flexible(OutCell, ForbiddenCells, DefaultRandomStream);
}

bool AASnakeGameModeBase::TryFindRandomFreeCell_Flexible(
	FIntPoint& OutCell,
	const TArray<FIntPoint>& ForbiddenCells,
	FRandomStream& RandomStream) const
{
	if (!GridManager)
	{
		return false;
	}

	TSet<FIntPoint> ForbiddenSet;
	ForbiddenSet.Reserve(ForbiddenCells.Num());

	for (const FIntPoint& Cell : ForbiddenCells)
	{
		ForbiddenSet.Add(Cell);
	}

	TArray<FIntPoint> CandidateCells;
	CandidateCells.Reserve(GridManager->GetCellCount());

	for (int32 Y = 0; Y < GridManager->GetHeight(); Y++)
	{
		for (int32 X = 0; X < GridManager->GetWidth(); X++)
		{
			const FIntPoint Cell(X, Y);

			if (GridManager->IsCellBlockedByBoard(Cell))
			{
				continue;
			}

			if (ForbiddenSet.Contains(Cell))
			{
				continue;
			}

			CandidateCells.Add(Cell);
		}
	}

	if (CandidateCells.IsEmpty())
	{
		return false;
	}

	const int32 RandomIndex = RandomStream.RandRange(0, CandidateCells.Num() - 1);
	OutCell = CandidateCells[RandomIndex];

	return true;
}


void AASnakeGameModeBase::SpawnSnake()
{
	if (!SnakePawnClass)
	{
		UE_LOG(LogTemp, Warning, TEXT("SnakeGameMode has no SnakeClass assigned."));
		return;
	}

	if (!GridManager)
	{
		UE_LOG(LogTemp, Warning, TEXT("SnakeGameMode has no GridManager assigned."));
		return;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	if (IsValid(SpawnedSnakePawn))
	{
		SpawnedSnakePawn->Destroy();
		SpawnedSnakePawn = nullptr;
	}

	FIntPoint SnakeSpawnCell(GridManager->GetWidth() / 2, GridManager->GetHeight() / 2);
	if (SnakeSpawnPoint)
	{
		SnakeSpawnCell = GridManager->WorldToCell(SnakeSpawnPoint->GetActorLocation());
	}

	// need to align to propper cell placement first
	if (!GridManager->IsInBounds(SnakeSpawnCell))
	{
		UE_LOG(LogTemp, Warning, TEXT("Snake spawn location was outside the grid. Falling back to cell 0,0."));
		SnakeSpawnCell = FIntPoint::ZeroValue;
	}

	const FVector SpawnLocation = GridManager->CellToWorld(SnakeSpawnCell);
	const FRotator SpawnRotator = FRotator::ZeroRotator;
	const FTransform SpawnTransform(SpawnRotator, SpawnLocation);

	AASnakeGridwalkerPawn* NewSnake =
		World->SpawnActorDeferred<AASnakeGridwalkerPawn>(
			SnakePawnClass,
			SpawnTransform,
			nullptr,
			nullptr,
			ESpawnActorCollisionHandlingMethod::AlwaysSpawn);

	if (!NewSnake)
	{
		UE_LOG(LogTemp, Warning, TEXT("Failed to spawn snake."));
		return;
	}

	NewSnake->ConfigureForGrid(GridManager, SnakeSpawnCell);
	UGameplayStatics::FinishSpawningActor(NewSnake, SpawnTransform);

	SpawnedSnakePawn = NewSnake;

	// is this the safe pattern to avoid accidental double-binding
	SpawnedSnakePawn->OnSnakeDied.RemoveDynamic(this, &AASnakeGameModeBase::HandleSnakeDeath);
	SpawnedSnakePawn->OnSnakeDied.AddDynamic(this, &AASnakeGameModeBase::HandleSnakeDeath);

	APlayerController* PlayerController = World->GetFirstPlayerController();
	if (PlayerController)
	{
		PlayerController->Possess(SpawnedSnakePawn);
		UE_LOG(LogTemp, Warning, TEXT("PlayerController possessed SnakePawn."));
	}
}


// can only be one fruit at this stage, so destroys previous if still active 
void AASnakeGameModeBase::SpawnFruit_Destructive(const FIntPoint& Cell)
{
	if (!FoodActorClass)
	{
		UE_LOG(LogTemp, Warning, TEXT("SnakeGameMode has no FoodClass assigned."));
		return;
	}

	if (!GridManager)
	{
		UE_LOG(LogTemp, Warning, TEXT("SnakeGameMode has no GridManager assigned."));
		return;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	if (IsValid(SpawnedFoodActor))
	{
		SpawnedFoodActor->OnFruitConsumed.RemoveDynamic(this, &AASnakeGameModeBase::HandleFruitConsumed);

		SpawnedFoodActor->Destroy();
		SpawnedFoodActor = nullptr;
	}

	FVector SpawnLocation = GridManager->CellToWorld(Cell);

	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = this;

	// will be placed inside floor at first
	SpawnedFoodActor = World->SpawnActor<AAFoodActor>(
		FoodActorClass,
		SpawnLocation,
		FRotator::ZeroRotator,
		SpawnParams);

	if (!SpawnedFoodActor)
	{
		UE_LOG(LogTemp, Warning, TEXT("Failed to spawn food."));
		return;
	}

	// can only query the spawned actor's component size after it exists.
	SpawnLocation.Z += SpawnedFoodActor->GetPlacementHalfHeight();

	SpawnedFoodActor->SetFoodGridPosition(Cell, SpawnLocation);
	SpawnedFoodActor->SetFoodValues(1, 1);
	SpawnedFoodActor->SetActiveStatus(true);

	SpawnedFoodActor->OnFruitConsumed.AddDynamic(this, &AASnakeGameModeBase::HandleFruitConsumed);
}

void AASnakeGameModeBase::StartGameLoop()
{
	if (!IsValid(SpawnedSnakePawn))
	{
		UE_LOG(LogTemp, Warning, TEXT("Cannot start game loop: no snake."));
		return;
	}

	SpawnedSnakePawn->ResetSnake();
	SpawnedSnakePawn->StartMovement();
}

void AASnakeGameModeBase::StopGameLoop()
{
	if (!IsValid(SpawnedSnakePawn))
	{
		UE_LOG(LogTemp, Warning, TEXT("Cannot start game loop: no snake."));
		return;
	}

	SpawnedSnakePawn->StopMovement();
}


void AASnakeGameModeBase::HandleFruitConsumed(AAFoodActor* Food, AActor* ConsumerActor)
{
	// Defensive guard note:
	// This delegate should only be bound to CurrentFood, but this prevents stale,
	// duplicate, or wrong-instance events from affecting this grid.
	if (Food != SpawnedFoodActor)
	{
		return;
	}


	if (ASnakeGameState* GS = GetSnakeGameState())
	{
		GS->AddScore(Food->GetScoreValue());
	}


	ScoreThisStage++;


	SpawnedFoodActor = nullptr;
	RespawnFruit_Temp();
}

void AASnakeGameModeBase::HandleSnakeDeath(AASnakeGridwalkerPawn* DeadSnake)
{
	if (DeadSnake != SpawnedSnakePawn)
	{
		return;
	}

	// VERY TEMP SOLUTION! snake does not die yet, just resets its state to start version for now. 
	RestartRun();
}
