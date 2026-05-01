// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "SnakeGameModeBase.generated.h"

class ASnakeGridwalkerPawn;
class AFoodActor;
class AGridManagerActor;
class USnakeSettingsDataAsset;
class UGridSettingsDataAsset;
class ASnakeGameState;

class UUserWidget;


/**
 *
 */
UCLASS()
class ASnakeGameModeBase : public AGameModeBase
{
	GENERATED_BODY()

public:
	ASnakeGameModeBase();

	virtual void BeginPlay() override;


	// --- Information getters ---

	UFUNCTION(BlueprintPure)
	bool IsFoodAtCell(const FIntPoint& Cell) const;

	UFUNCTION(BlueprintPure)
	TArray<FIntPoint> GetAllSnakeOccupiedCells() const;

	UFUNCTION(BlueprintPure)
	bool AnySnakeOnThisCell(const FIntPoint& Cell) const;

	// --- Logic setters --- 

	UFUNCTION(BlueprintCallable, Category = "Snake|flow")
	void StartPlayingRun();

	UFUNCTION(BlueprintCallable, Category = "Snake|flow")
	void RestartRun();

	UFUNCTION(BlueprintCallable, Category = "Snake|flow")
	void RespawnFruit_Temp();

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Snake|UI")
	TSubclassOf<UUserWidget> MainMenuWidgetClass;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Snake|UI")
	TSubclassOf<UUserWidget> OutroWidgetClass;

	UPROPERTY()
	TObjectPtr<UUserWidget> MainMenuWidgetInstance;

	UPROPERTY()
	TObjectPtr<UUserWidget> OutroWidgetInstance;

private:
	void CacheGridManager();
	void FindOrSpawnGridManager();

	bool IsCellFreeForGameplay(const FIntPoint& Cell) const;
	bool TryFindRandomFreeCell(FIntPoint& OutCell);
	bool TryFindRandomFreeCell_Flexible(FIntPoint& OutCell, const TArray<FIntPoint>& ForbiddenCells,
	                                    FRandomStream& RandomStream) const;

	void SpawnSnake();
	void SpawnFruit_Destructive(const FIntPoint& Cell);

	// UI 
	void ShowMainMenuWidget();
	void ShowOutroWidget();
	void HideMenuWidgets();

	void SetMenuInputMode();
	void SetGameplayInputMode();

	// Game 
	void StartGameLoop();
	void StopGameLoop();
	//void PauseGameLoop();

	UFUNCTION()
	void HandleFruitConsumed(AFoodActor* Food, AActor* ConsumerActor);

	UFUNCTION()
	void HandleSnakeDeath(ASnakeGridwalkerPawn* DeadSnake);


	// Setup
	UPROPERTY(EditDefaultsOnly, Category = "Snake|SnakePawn",
		meta=(AllowPrivateAccess="true"))
	TSubclassOf<ASnakeGridwalkerPawn> SnakePawnClass;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Snake|SnakePawn",
		meta = (AllowPrivateAccess = "true"))
	TObjectPtr<USnakeSettingsDataAsset> SnakeStartupSettingsPreset;

	UPROPERTY(EditAnywhere, Category = "Snake|FoodActor",
		meta = (AllowPrivateAccess = "true"))
	TSubclassOf<AFoodActor> FoodActorClass;

	UPROPERTY(EditDefaultsOnly, Category = "Snake|Grid",
		meta=(AllowPrivateAccess="true"))
	TSubclassOf<AGridManagerActor> GridManagerClass;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Snake|Grid",
		meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UGridSettingsDataAsset> GridStartupSettingsPreset;

	UPROPERTY(EditDefaultsOnly, Category = "Snake|Grid",
		meta=(AllowPrivateAccess="true"))
	FIntPoint SnakeSpawnCell = FIntPoint(0, 0);

	// Runtime set
	UPROPERTY(Transient, VisibleInstanceOnly, Category = "Snake|Runtime",
		meta=(AllowPrivateAccess="true"))
	TObjectPtr<ASnakeGridwalkerPawn> SpawnedSnakePawn;

	UPROPERTY(Transient, VisibleInstanceOnly, Category = "Snake|Runtime",
		meta = (AllowPrivateAccess="true"))
	TArray<TObjectPtr<ASnakeGridwalkerPawn>> SpawnedSnakes;

	UPROPERTY(Transient, VisibleInstanceOnly, Category = "Snake|Runtime",
		meta = (AllowPrivateAccess = "true"))
	TObjectPtr<AFoodActor> SpawnedFoodActor;

	UPROPERTY(Transient, VisibleInstanceOnly, Category = "Snake|Runtime",
		meta = (AllowPrivateAccess = "true"))
	TObjectPtr<AGridManagerActor> GridManager;


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
