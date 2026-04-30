// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include  "SnakeSettingsTypes.h"
#include  "GridSettingsTypes.h"

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "ASnakeGameModeBase.generated.h"

class AASnakeGridwalkerPawn;
class AAFoodActor;
class AAGridManagerActor;
class ASnakeGameState;


/**
 *
 */
UCLASS()
class AASnakeGameModeBase : public AGameModeBase
{
	GENERATED_BODY()

public:
	virtual void BeginPlay() override;


	// --- Information getters ---

	UFUNCTION(BlueprintPure, Category = "Grid|Food")
	bool IsFoodAtCell(const FIntPoint& Cell) const;

	TArray<FIntPoint> GetAllSnakeOccupiedCells() const;
	bool AnySnakeOnThisCell(const FIntPoint& Cell) const;

	// --- Logic setters --- 

	UFUNCTION(BlueprintCallable, Category = "Snake")
	void StartPlayingRun();

	UFUNCTION(BlueprintCallable, Category = "Snake")
	void RestartRun();

	UFUNCTION(BlueprintCallable, Category = "Grid|Food")
	void RespawnFruit_Temp();

private:
	void CacheGridManager();

	bool IsCellFreeForGameplay(const FIntPoint& Cell) const;
	bool TryFindRandomFreeCell(FIntPoint& OutCell);
	bool TryFindRandomFreeCell_Flexible(FIntPoint& OutCell, const TArray<FIntPoint>& ForbiddenCells,
	                                    FRandomStream& RandomStream) const;

	void SpawnSnake();
	void SpawnFruit_Destructive(const FIntPoint& Cell);

	// Game 
	void StartGameLoop();
	void StopGameLoop();
	//void PauseGameLoop();

	UFUNCTION()
	void HandleFruitConsumed(AAFoodActor* Food, AActor* ConsumerActor);

	UFUNCTION()
	void HandleSnakeDeath(AASnakeGridwalkerPawn* DeadSnake);


	UPROPERTY(EditInstanceOnly, Category = "Snake|Reference",
		meta=(AllowPrivateAccess="true"))
	TObjectPtr<AActor> SnakeSpawnPoint;

	UPROPERTY(EditInstanceOnly, Category = "Snake|Reference",
		meta=(AllowPrivateAccess="true"))
	TSubclassOf<AASnakeGridwalkerPawn> SnakePawnClass;

	UPROPERTY(EditAnywhere, Category = "Snake|Reference",
		meta = (AllowPrivateAccess = "true"))
	TSubclassOf<AAFoodActor> FoodActorClass;

	UPROPERTY(Transient, VisibleInstanceOnly, Category = "Snake|Runtime",
		meta=(AllowPrivateAccess="true"))
	TObjectPtr<AASnakeGridwalkerPawn> SpawnedSnakePawn;

	UPROPERTY(Transient, VisibleInstanceOnly, Category = "Snake|Runtime",
		meta = (AllowPrivateAccess="true"))
	TArray<TObjectPtr<AASnakeGridwalkerPawn>> SpawnedSnakes;

	UPROPERTY(Transient, VisibleInstanceOnly, Category = "Snake|Runtime",
		meta = (AllowPrivateAccess = "true"))
	TObjectPtr<AAFoodActor> SpawnedFoodActor;

	UPROPERTY(Transient, VisibleInstanceOnly, Category = "Snake|Runtime",
		meta = (AllowPrivateAccess = "true"))
	TObjectPtr<AAGridManagerActor> GridManager;


	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Grid|Settings",
		meta = (AllowPrivateAccess = "true"))
	int32 RootSeed = 12345;


	int32 CurrentStageIndex = 0;
	int32 ScoreThisStage = 0;

	FRandomStream DefaultRandomStream;
	FRandomStream FoodRandomStream;

	ASnakeGameState* GetSnakeGameState();
};
