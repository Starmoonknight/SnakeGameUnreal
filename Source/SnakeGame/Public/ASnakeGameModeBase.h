// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "ASnakeGameModeBase.generated.h"

class AASnakeGridwalkerPawn;
class AAFoodActor;
class AAGridManagerActor;
class USnakeSettingsDataAsset;
class UGridSettingsDataAsset;
class ASnakeGameState;


/**
 *
 */
UCLASS()
class AASnakeGameModeBase : public AGameModeBase
{
	GENERATED_BODY()

public:
	AASnakeGameModeBase();

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
	void FindOrSpawnGridManager();

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


	// Setup
	UPROPERTY(EditDefaultsOnly, Category = "Snake|SnakePawn",
		meta=(AllowPrivateAccess="true"))
	TSubclassOf<AASnakeGridwalkerPawn> SnakePawnClass;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Snake|SnakePawn",
		meta = (AllowPrivateAccess = "true"))
	TObjectPtr<USnakeSettingsDataAsset> SnakeStartupSettingsPreset;

	UPROPERTY(EditAnywhere, Category = "Snake|FoodActor",
		meta = (AllowPrivateAccess = "true"))
	TSubclassOf<AAFoodActor> FoodActorClass;

	UPROPERTY(EditDefaultsOnly, Category = "Snake|Grid",
		meta=(AllowPrivateAccess="true"))
	TSubclassOf<AAGridManagerActor> GridManagerClass;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Snake|Grid",
		meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UGridSettingsDataAsset> GridStartupSettingsPreset;

	UPROPERTY(EditDefaultsOnly, Category = "Snake|Grid",
		meta=(AllowPrivateAccess="true"))
	FIntPoint SnakeSpawnCell = FIntPoint(0, 0);

	// Runtime set
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


	// Runtime values 
	int32 CurrentStageIndex = 0;
	int32 ScoreThisStage = 0;

	FRandomStream DefaultRandomStream;
	FRandomStream FoodRandomStream;

	ASnakeGameState* GetSnakeGameState();
};
