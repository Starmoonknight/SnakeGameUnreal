// Fill out your copyright notice in the Description page of Project Settings.


#include "SnakeGameModeBase.h"
#include "SnakeGameState.h"
#include "SnakeGridwalkerPawn.h"
#include "FoodActor.h"
#include "GridManagerActor.h"

#include "SnakeStageSettingsDataAsset.h"
#include "SnakeSettingsDataAsset.h"
#include "GridSettingsDataAsset.h"

#include "Engine/Engine.h"
#include "GameFramework/PlayerController.h"
#include "Blueprint/UserWidget.h"
#include "Kismet/GameplayStatics.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraSystem.h"
#include "Sound/SoundBase.h"


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


ASnakeGameModeBase::ASnakeGameModeBase()
{
	GameStateClass = ASnakeGameState::StaticClass();
	DefaultPawnClass = nullptr;
}

void ASnakeGameModeBase::BeginPlay()
{
	Super::BeginPlay();

	FoodRandomStream.Initialize(MakeSubsystemSeed(RootSeed, FruitSeedTag));
	DefaultRandomStream.Initialize(MakeSubsystemSeed(RootSeed, DefaultSeedTag));

	if (ASnakeGameState* GS = GetSnakeGameState())
	{
		GS->SetMatchPhase(ESnakeMatchPhase::MainMenu);
	}

	ShowMainMenuWidget();

	//StartPlayingRun();
}

void ASnakeGameModeBase::CacheGridManager()
{
	GridManager = Cast<AGridManagerActor>(UGameplayStatics::GetActorOfClass(this, AGridManagerActor::StaticClass()));
}

void ASnakeGameModeBase::FindOrSpawnGridManager()
{
	if (IsValid(GridManager))
	{
		return;
	}

	CacheGridManager();

	if (IsValid(GridManager))
	{
		return;
	}

	if (!GridManagerClass)
	{
		UE_LOG(LogTemp, Warning,
		       TEXT("SnakeGameMode could not find a placed GridManager and has no GridManagerClass assigned."));
		return;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	GridManager = World->SpawnActor<AGridManagerActor>(
		GridManagerClass,
		FVector::ZeroVector,
		FRotator::ZeroRotator);
}

ASnakeGameState* ASnakeGameModeBase::GetSnakeGameState()
{
	return GetGameState<ASnakeGameState>();
}


void ASnakeGameModeBase::QueueDirectionForSnake(int32 PlayerIndex, EGridDirection Direction)
{
	if (!SpawnedSnakes.IsValidIndex(PlayerIndex))
	{
		return;
	}

	if (!IsValid(SpawnedSnakes[PlayerIndex]))
	{
		return;
	}

	SpawnedSnakes[PlayerIndex]->QueueDirectionInput(Direction);
}

void ASnakeGameModeBase::StartPlayingRun()
{
	HideMenuWidgets();
	HideHUDWidget();
	SetGameplayInputMode();

	//BattleResult = ESnakeBattleResult::None;


	// same with this part, should it be removed or modified? 
	if (ASnakeGameState* GS = GetSnakeGameState())
	{
		GS->ResetScores(ActiveLocalPlayerCount);
		GS->BattleResult = ESnakeBattleResult::None;
		GS->SetMatchPhase(ESnakeMatchPhase::Playing);

		// clear away the old end game message, for the temp placeholder UI but might bee good to keep this as a general cleaner anyway.   
		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(
				2,
				0.01f,
				FColor::Red,
				TEXT("")
			);
			const FString MatchPhaseName =
				StaticEnum<ESnakeMatchPhase>()->GetNameStringByValue(
					static_cast<int64>(GS->MatchPhase)
				);

			GEngine->AddOnScreenDebugMessage(
				1,
				999.0f,
				FColor::Green,
				FString::Printf(TEXT("Score: %d | PHASE: %s"),
				                GS->Score,
				                *MatchPhaseName)
			);
		}
	}


	FindOrSpawnGridManager();

	if (!GridManager)
	{
		UE_LOG(LogTemp, Warning, TEXT("SnakeGameMode could not find or spawn GridManager."));
		return;
	}

	if (GetStageCount() < 3)
	{
		UE_LOG(
			LogTemp,
			Error,
			TEXT("At least three stage presets are required."));

		return;
	}

	EnsureLocalPlayers();
	ShowHUDWidget();
	LoadStage(0);
}

void ASnakeGameModeBase::RestartRun()
{
	StartPlayingRun();
}

void ASnakeGameModeBase::ReturnToMainMenu()
{
	StopGameLoop();
	ClearSpawnedActors();
	HideHUDWidget();

	// Single-player only needs Player 0
	if (APlayerController* SecondPlayer =
		UGameplayStatics::GetPlayerController(this, 1))
	{
		UGameplayStatics::RemovePlayer(SecondPlayer, true);
	}

	if (ASnakeGameState* GS = GetSnakeGameState())
	{
		GS->SetMatchPhase(ESnakeMatchPhase::MainMenu);
	}

	ShowMainMenuWidget();
}


int32 ASnakeGameModeBase::GetFinalScore() const
{
	const ASnakeGameState* GS = GetGameState<ASnakeGameState>();
	return GS ? GS->Score : 0;
}

void ASnakeGameModeBase::StartSinglePlayerRun()
{
	ActiveLocalPlayerCount = 1;
	bUseSharedKeyboardControls = false;

	if (ASnakeGameState* GS = GetSnakeGameState())
	{
		GS->PlayMode = ESnakeGameModeType::SinglePlayer;
	}

	StartPlayingRun();
}

void ASnakeGameModeBase::StartCooperativeRun()
{
	ActiveLocalPlayerCount = 2;
	bUseSharedKeyboardControls = false;

	if (ASnakeGameState* GS = GetSnakeGameState())
	{
		GS->PlayMode = ESnakeGameModeType::Cooperative;
	}

	StartPlayingRun();
}

void ASnakeGameModeBase::StartVersusRun()
{
	ActiveLocalPlayerCount = 2;
	bUseSharedKeyboardControls = false;

	if (ASnakeGameState* GS = GetSnakeGameState())
	{
		GS->PlayMode = ESnakeGameModeType::Versus;
	}

	StartPlayingRun();
}

void ASnakeGameModeBase::StartCooperativeSharedKeyboardRun()
{
	ActiveLocalPlayerCount = 2;
	bUseSharedKeyboardControls = true;

	if (ASnakeGameState* GS = GetSnakeGameState())
	{
		GS->PlayMode = ESnakeGameModeType::Cooperative;
	}

	StartPlayingRun();
}

void ASnakeGameModeBase::StartVersusSharedKeyboardRun()
{
	ActiveLocalPlayerCount = 2;
	bUseSharedKeyboardControls = true;

	if (ASnakeGameState* GS = GetSnakeGameState())
	{
		GS->PlayMode = ESnakeGameModeType::Versus;
	}

	StartPlayingRun();
}


bool ASnakeGameModeBase::IsFoodAtCell(const FIntPoint& Cell) const
{
	return IsValid(SpawnedFoodActor)
		&& SpawnedFoodActor->IsActive()
		&& SpawnedFoodActor->GetFoodGridPosition() == Cell;
}

// TO-DO Note:
// After full gameplay loop exists look for a way to handle multiple fruits existing at same time, 
// timed spawn to a max cap of current active. 
// Then either stop spawning and until a fruit has been eaten, starting new spawn-timer from that point.
// OR, start spawning with SpawnFruit_Destructive that destroys the oldest fruit first. 
// 
// Probably change RespawnFruit_Temp and SpawnFruit_Destructive to be generic functions that can take in a list or object,
// so that it can be re-used with other consumables, both boons and banes. 
void ASnakeGameModeBase::RespawnFruit_Temp()
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

// can only be one fruit at this stage, so destroys previous if still active 
void ASnakeGameModeBase::SpawnFruit_Destructive(const FIntPoint& Cell)
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
		SpawnedFoodActor->OnFruitConsumed.RemoveDynamic(this, &ASnakeGameModeBase::HandleFruitConsumed);

		SpawnedFoodActor->Destroy();
		SpawnedFoodActor = nullptr;
	}

	FVector SpawnLocation = GridManager->GridToWorld(Cell);

	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = this;

	// will be placed inside floor at first
	SpawnedFoodActor = World->SpawnActor<AFoodActor>(
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

	SpawnedFoodActor->OnFruitConsumed.AddDynamic(this, &ASnakeGameModeBase::HandleFruitConsumed);
}


TArray<FIntPoint> ASnakeGameModeBase::GetAllSnakeOccupiedCells() const
{
	TArray<FIntPoint> OccupiedCells;

	for (const ASnakeGridwalkerPawn* Snake : SpawnedSnakes)
	{
		if (!IsValid(Snake))
		{
			continue;
		}

		OccupiedCells.Add(Snake->GetHeadCellPosition());
		OccupiedCells.Append(Snake->GetBodyCellPositions());
	}

	return OccupiedCells;
}

bool ASnakeGameModeBase::AnySnakeOnThisCell(const FIntPoint& Cell) const
{
	for (const ASnakeGridwalkerPawn* Snake : SpawnedSnakes)
	{
		if (IsValid(Snake) && Snake->IsSnakeAtCell(Cell))
		{
			return true;
		}
	}

	return false;
}

bool ASnakeGameModeBase::AnyOtherSnakeOnThisCell(const FIntPoint& Cell, const ASnakeGridwalkerPawn* IgnoredSnake) const
{
	for (const ASnakeGridwalkerPawn* Snake : SpawnedSnakes)
	{
		if (IsValid(Snake) && Snake != IgnoredSnake &&
			Snake->IsAlive() && Snake->IsSnakeAtCell(Cell))
		{
			return true;
		}
	}

	return false;
}

bool ASnakeGameModeBase::IsCellFreeForGameplay(const FIntPoint& Cell) const
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

bool ASnakeGameModeBase::TryFindRandomFreeCell(FIntPoint& OutCell)
{
	TArray<FIntPoint> ForbiddenCells;

	ForbiddenCells.Append(GetAllSnakeOccupiedCells());

	if (IsValid(SpawnedFoodActor))
	{
		ForbiddenCells.Add(SpawnedFoodActor->GetFoodGridPosition());
	}

	return TryFindRandomFreeCell_Flexible(OutCell, ForbiddenCells, DefaultRandomStream);
}

bool ASnakeGameModeBase::TryFindRandomFreeCell_Flexible(
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

void ASnakeGameModeBase::EnsureLocalPlayers()
{
	ActiveLocalPlayerCount = FMath::Clamp(ActiveLocalPlayerCount, 1, 2);

	APlayerController* SecondPlayer = UGameplayStatics::GetPlayerController(this, 1);

	// when only single player 
	if (ActiveLocalPlayerCount == 1)
	{
		if (SecondPlayer)
		{
			UGameplayStatics::RemovePlayer(SecondPlayer, true);
		}

		return;
	}

	if (!SecondPlayer)
	{
		UGameplayStatics::CreatePlayer(this, -1, true);
	}

	if (!UGameplayStatics::GetPlayerController(this, 1))
	{
		UE_LOG(LogTemp, Error, TEXT("Could not create the second local player."));
	}
}


void ASnakeGameModeBase::SpawnSnakes()
{
	SpawnedSnakes.Reset();

	for (int32 PlayerIndex = 0; PlayerIndex < ActiveLocalPlayerCount; ++PlayerIndex)
	{
		SpawnSnakeForPlayer(PlayerIndex, GetSpawnCellForPlayer(PlayerIndex));
	}
}

void ASnakeGameModeBase::SpawnSnakeForPlayer(int32 PlayerIndex, const FIntPoint& RequestedSpawnCell)
{
	if (!SnakePawnClass)
	{
		UE_LOG(LogTemp, Error, TEXT("Cannot spawn player %d snake: missing class."),
		       PlayerIndex);
		return;
	}

	if (!GridManager)
	{
		UE_LOG(LogTemp, Error, TEXT("Cannot spawn player %d snake: missing grid."),
		       PlayerIndex);
		return;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	FIntPoint ResolvedSpawnCell = RequestedSpawnCell;

	const bool bRequestedCellInvalid =
		!GridManager->IsInBounds(ResolvedSpawnCell) ||
		GridManager->IsCellBlockedByBoard(ResolvedSpawnCell) ||
		AnySnakeOnThisCell(ResolvedSpawnCell);

	if (bRequestedCellInvalid)
	{
		if (!TryFindRandomFreeCell(ResolvedSpawnCell))
		{
			UE_LOG(LogTemp, Error, TEXT("No valid spawn cell for player %d."),
			       PlayerIndex);

			return;
		}
	}

	const FVector SpawnLocation = GridManager->GridToWorld(ResolvedSpawnCell);
	const FTransform SpawnTransform(FRotator::ZeroRotator, SpawnLocation);

	ASnakeGridwalkerPawn* NewSnake = World->SpawnActorDeferred<ASnakeGridwalkerPawn>(
		SnakePawnClass, SpawnTransform,
		nullptr, nullptr,
		ESpawnActorCollisionHandlingMethod::AlwaysSpawn);

	if (!NewSnake)
	{
		return;
	}

	NewSnake->ConfigureForGrid(GridManager, ResolvedSpawnCell);

	// set this stages modified start-values before finishing spawning 
	if (const USnakeStageSettingsDataAsset* StagePreset = GetStagePreset(CurrentStageIndex))
	{
		NewSnake->SetStartupSettingsPreset(StagePreset->SnakeSettingsPreset);
	}

	// complete deferred spawning, lifecycle functions such as BeginPlay() runs   
	UGameplayStatics::FinishSpawningActor(NewSnake, SpawnTransform);

	// is this the safe pattern to avoid accidental double-binding
	NewSnake->OnSnakeDied.RemoveDynamic(this, &ASnakeGameModeBase::HandleSnakeDeath);
	NewSnake->OnSnakeDied.AddDynamic(this, &ASnakeGameModeBase::HandleSnakeDeath);

	SpawnedSnakes.Add(NewSnake);


	APlayerController* PlayerController = UGameplayStatics::GetPlayerController(this, PlayerIndex);
	if (PlayerController)
	{
		PlayerController->Possess(NewSnake);
		UE_LOG(LogTemp, Warning, TEXT("PlayerController possessed SnakePawn."));
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("No PlayerController for player %d."),
		       PlayerIndex);
	}
}

int32 ASnakeGameModeBase::GetPlayerIndexForSnake(const ASnakeGridwalkerPawn* Snake) const
{
	for (int32 Index = 0; Index < SpawnedSnakes.Num(); ++Index)
	{
		if (SpawnedSnakes[Index].Get() == Snake)
		{
			return Index;
		}
	}

	return INDEX_NONE;
}

FIntPoint ASnakeGameModeBase::GetSpawnCellForPlayer(int32 PlayerIndex) const
{
	if (!GridManager)
	{
		return FIntPoint::ZeroValue;
	}

	// find a way to spawn player 2 on opposite side of grid, mirrored cell and movement direction, so they move towards each other from different corners 
	// Say player 1 is bottom left corner moving up, then player 2 is in top right corner moving down. 

	const int32 SpawnX = FMath::Clamp(2, 0, GridManager->GetWidth() - 1);

	const int32 SpawnY =
		PlayerIndex == 0
			? FMath::Clamp(2, 0, GridManager->GetHeight() - 1)
			: FMath::Clamp(GridManager->GetHeight() - 3, 0, GridManager->GetHeight() - 1);

	return FIntPoint(SpawnX, SpawnY);
}

void ASnakeGameModeBase::ClearSpawnedActors()
{
	if (IsValid(SpawnedFoodActor))
	{
		SpawnedFoodActor->OnFruitConsumed.RemoveDynamic(this, &ASnakeGameModeBase::HandleFruitConsumed);

		SpawnedFoodActor->Destroy();
	}

	SpawnedFoodActor = nullptr;

	for (ASnakeGridwalkerPawn* Snake : SpawnedSnakes)
	{
		if (!IsValid(Snake))
		{
			continue;
		}

		Snake->OnSnakeDied.RemoveDynamic(
			this,
			&ASnakeGameModeBase::HandleSnakeDeath);

		Snake->Destroy();
	}

	SpawnedSnakes.Reset();
}

int32 ASnakeGameModeBase::GetStageCount() const
{
	return StagePresets.Num();
}

void ASnakeGameModeBase::CompleteStage()
{
	if (StageCompleteSound)
	{
		UGameplayStatics::PlaySound2D(this, StageCompleteSound);
	}

	if (StageCompleteEffect && GridManager)
	{
		const FIntPoint CentreCell(
			GridManager->GetWidth() / 2,
			GridManager->GetHeight() / 2);

		UNiagaraFunctionLibrary::SpawnSystemAtLocation(this, StageCompleteEffect,
		                                               GridManager->GridToWorld(CentreCell));
	}

	const int32 NextStageIndex = CurrentStageIndex + 1;

	if (NextStageIndex < GetStageCount())
	{
		LoadStage(NextStageIndex);
	}
	else
	{
		CompleteRun();
	}
}

const USnakeStageSettingsDataAsset* ASnakeGameModeBase::GetStagePreset(int32 StageIndex) const
{
	if (!StagePresets.IsValidIndex(StageIndex))
	{
		return nullptr;
	}

	return StagePresets[StageIndex];
}

void ASnakeGameModeBase::LoadStage(int32 StageIndex)
{
	const USnakeStageSettingsDataAsset* StagePreset = GetStagePreset(StageIndex);

	if (!StagePreset)
	{
		UE_LOG(
			LogTemp,
			Error,
			TEXT("Cannot load stage %d: invalid stage index."),
			StageIndex);

		return;
	}

	if (!StagePreset->GridSettingsPreset)
	{
		UE_LOG(
			LogTemp,
			Error,
			TEXT("Stage %d has no Grid Settings Preset."),
			StageIndex);

		return;
	}

	if (!StagePreset->SnakeSettingsPreset)
	{
		UE_LOG(
			LogTemp,
			Error,
			TEXT("Stage %d has no Snake Settings Preset."),
			StageIndex);

		return;
	}

	if (!GridManager)
	{
		UE_LOG(
			LogTemp,
			Error,
			TEXT("Cannot load stage: Grid Manager is missing."));

		return;
	}

	StopGameLoop();
	ClearSpawnedActors();

	CurrentStageIndex = StageIndex;
	ScoreThisStage = 0;

	GridManager->ApplyRuntimeSettings(StagePreset->GridSettingsPreset->GridSettings);

	if (ASnakeGameState* GS = GetSnakeGameState())
	{
		GS->CurrentStageIndex = StageIndex;
		GS->FoodEatenThisStage = 0;
		GS->PointsGainedThisStage = 0;
		GS->PointsToClearStage = FMath::Max(StagePreset->StageSettings.FoodToClearStage, 1);
	}

	SpawnSnakes();
	RespawnFruit_Temp();
	StartGameLoop();
}

void ASnakeGameModeBase::CompleteRun()
{
	StopGameLoop();
	HideHUDWidget();

	if (ASnakeGameState* GS = GetSnakeGameState())
	{
		if (GS->PlayMode == ESnakeGameModeType::Versus)
		{
			const int32 Player0Score = GS->GetPlayerScore(0);
			const int32 Player1Score = GS->GetPlayerScore(1);

			if (Player0Score > Player1Score)
			{
				GS->BattleResult = ESnakeBattleResult::Player0Won;
			}
			else if (Player1Score > Player0Score)
			{
				GS->BattleResult = ESnakeBattleResult::Player1Won;
			}
			else
			{
				GS->BattleResult = ESnakeBattleResult::Draw;
			}
		}

		GS->SetMatchPhase(ESnakeMatchPhase::Outro);
	}

	ShowOutroWidget();
}


void ASnakeGameModeBase::ShowMainMenuWidget()
{
	HideMenuWidgets();

	if (!MainMenuWidgetClass)
	{
		UE_LOG(LogTemp, Warning, TEXT("MainMenuWidgetClass is not assigned."));
		return;
	}

	APlayerController* PC = GetWorld()->GetFirstPlayerController();
	if (!PC)
	{
		return;
	}

	MainMenuWidgetInstance = CreateWidget<UUserWidget>(PC, MainMenuWidgetClass);

	if (MainMenuWidgetInstance)
	{
		MainMenuWidgetInstance->AddToViewport();
		SetMenuInputMode();
	}
}

void ASnakeGameModeBase::ShowOutroWidget()
{
	HideMenuWidgets();

	if (!OutroWidgetClass)
	{
		UE_LOG(LogTemp, Warning, TEXT("OutroWidgetClass is not assigned."));
		return;
	}

	APlayerController* PC = GetWorld()->GetFirstPlayerController();
	if (!PC)
	{
		return;
	}

	OutroWidgetInstance = CreateWidget<UUserWidget>(PC, OutroWidgetClass);

	if (OutroWidgetInstance)
	{
		OutroWidgetInstance->AddToViewport();
		SetMenuInputMode();
	}
}

void ASnakeGameModeBase::ShowHUDWidget()
{
	HideHUDWidget();

	if (!HUDWidgetClass)
	{
		UE_LOG(LogTemp, Warning, TEXT("HUDWidgetClass is not assigned."));
		return;
	}

	APlayerController* PC = UGameplayStatics::GetPlayerController(this, 0);

	if (!PC)
	{
		return;
	}

	HUDWidgetInstance = CreateWidget<UUserWidget>(PC, HUDWidgetClass);

	if (HUDWidgetInstance)
	{
		// Z-order to 10 keep it above the game viewport
		HUDWidgetInstance->AddToViewport(10);
	}
}

void ASnakeGameModeBase::HideMenuWidgets()
{
	if (MainMenuWidgetInstance)
	{
		MainMenuWidgetInstance->RemoveFromParent();
		MainMenuWidgetInstance = nullptr;
	}

	if (OutroWidgetInstance)
	{
		OutroWidgetInstance->RemoveFromParent();
		OutroWidgetInstance = nullptr;
	}
}

void ASnakeGameModeBase::HideHUDWidget()
{
	if (HUDWidgetInstance)
	{
		HUDWidgetInstance->RemoveFromParent();
		HUDWidgetInstance = nullptr;
	}
}

void ASnakeGameModeBase::SetMenuInputMode()
{
	APlayerController* PC = GetWorld()->GetFirstPlayerController();
	if (!PC)
	{
		return;
	}

	PC->bShowMouseCursor = true;

	FInputModeUIOnly InputMode;
	PC->SetInputMode(InputMode);
}

void ASnakeGameModeBase::SetGameplayInputMode()
{
	APlayerController* PC = GetWorld()->GetFirstPlayerController();
	if (!PC)
	{
		return;
	}

	PC->bShowMouseCursor = false;

	FInputModeGameOnly InputMode;
	PC->SetInputMode(InputMode);
}


void ASnakeGameModeBase::StartGameLoop()
{
	if (SpawnedSnakes.IsEmpty())
	{
		UE_LOG(LogTemp, Warning, TEXT("Cannot start game loop: no snake."));

		return;
	}

	for (ASnakeGridwalkerPawn* Snake : SpawnedSnakes)
	{
		if (IsValid(Snake))
		{
			Snake->ResetSnake();
			Snake->StartMovement();
		}
	}
}

void ASnakeGameModeBase::StopGameLoop()
{
	for (ASnakeGridwalkerPawn* Snake : SpawnedSnakes)
	{
		if (IsValid(Snake))
		{
			Snake->StopMovement();
		}
	}
}


void ASnakeGameModeBase::HandleFruitConsumed(AFoodActor* Food, AActor* ConsumerActor)
{
	// Defensive guard note:
	// This delegate should only be bound to CurrentFood, but this prevents stale,
	// duplicate, or wrong-instance events from affecting this grid.
	if (Food != SpawnedFoodActor)
	{
		return;
	}

	// obtain score value from fruit
	const int32 ScoreValue = Food->GetScoreValue();
	const ASnakeGridwalkerPawn* ConsumingSnake = Cast<ASnakeGridwalkerPawn>(ConsumerActor);
	const int32 ScoringPlayerIndex = GetPlayerIndexForSnake(ConsumingSnake);

	// play food consumed effects
	const FVector FeedbackLocation = Food->GetActorLocation();

	if (FoodConsumedSound)
	{
		UGameplayStatics::PlaySound2D(this, FoodConsumedSound);
	}

	if (FoodConsumedEffect)
	{
		UNiagaraFunctionLibrary::SpawnSystemAtLocation(this, FoodConsumedEffect, FeedbackLocation);
	}

	ASnakeGameState* GS = GetSnakeGameState();

	if (GS)
	{
		GS->AddScoreForPlayer(ScoringPlayerIndex, ScoreValue);
		++GS->FoodEatenThisStage;
		GS->PointsGainedThisStage += ScoreValue;

		// temp text to debug while setting up propper HUD-UI
		if (GEngine)
		{
			const FString MatchPhaseName =
				StaticEnum<ESnakeMatchPhase>()->GetNameStringByValue(
					static_cast<int64>(GS->MatchPhase)
				);

			GEngine->AddOnScreenDebugMessage(
				1,
				999.0f,
				FColor::Green,
				FString::Printf(TEXT("Score: %d | PHASE: %s"),
				                GS->Score,
				                *MatchPhaseName));
		}
	}

	ScoreThisStage += ScoreValue;

	SpawnedFoodActor = nullptr;

	const bool bStageComplete =
		GS &&
		GS->FoodEatenThisStage >= GS->PointsToClearStage;

	if (bStageComplete)
	{
		CompleteStage();

		return;
	}

	RespawnFruit_Temp();
}

void ASnakeGameModeBase::HandleSnakeDeath(ASnakeGridwalkerPawn* DeadSnake)
{
	if (!SpawnedSnakes.Contains(DeadSnake))
	{
		return;
	}

	StopGameLoop();
	HideHUDWidget();

	if (SnakeDeathSound)
	{
		UGameplayStatics::PlaySound2D(this, SnakeDeathSound);
	}

	if (SnakeDeathEffect)
	{
		UNiagaraFunctionLibrary::SpawnSystemAtLocation(this,
		                                               SnakeDeathEffect,
		                                               DeadSnake->GetActorLocation());
	}

	if (ASnakeGameState* GS = GetSnakeGameState())
	{
		const int32 DeadPlayerIndex = GetPlayerIndexForSnake(DeadSnake);

		if (GS->PlayMode == ESnakeGameModeType::Versus)
		{
			// Switch to something that takes scores into consideration. Maybe a victory evaluation function. 
			// Currently, the surviving snake wins, and if both clear all levels then score decides winner 
			switch (DeadPlayerIndex)
			{
			case 0:
				GS->BattleResult = ESnakeBattleResult::Player1Won;
				break;

			case 1:
				GS->BattleResult = ESnakeBattleResult::Player0Won;
				break;

			default:
				GS->BattleResult = ESnakeBattleResult::Draw;
				break;
			}
		}

		GS->SetMatchPhase(ESnakeMatchPhase::Outro);

		// temp text  to debug while setting up propper end screen 
		if (GEngine)
		{
			const FString MatchPhaseName =
				StaticEnum<ESnakeMatchPhase>()->GetNameStringByValue(
					static_cast<int64>(GS->MatchPhase)
				);

			GEngine->AddOnScreenDebugMessage(
				2,
				999.0f,
				FColor::Red,
				FString::Printf(TEXT("GAME OVER - Final Score: %d | PHASE: %s"),
				                GS->Score,
				                *MatchPhaseName)
			);
		}
	}

	ShowOutroWidget();
}
