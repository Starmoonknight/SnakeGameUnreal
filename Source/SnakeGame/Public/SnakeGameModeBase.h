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
class USnakeStageSettingsDataAsset;
class ASnakeGameState;

class UUserWidget;
class USoundBase;
class UNiagaraSystem;


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

	bool AnyOtherSnakeOnThisCell(const FIntPoint& Cell, const ASnakeGridwalkerPawn* IgnoredSnake) const;

	UFUNCTION(BlueprintPure, Category = "Snake|Flow")
	int32 GetFinalScore() const;

	UFUNCTION(BlueprintPure, Category = "Snake|Flow")
	int32 GetCurrentStageNumber() const { return CurrentStageIndex + 1; }

	// --- Logic setters --- 
	UFUNCTION(BlueprintCallable, Category = "Snake|Flow")
	void StartSinglePlayerRun();

	UFUNCTION(BlueprintCallable, Category = "Snake|Flow")
	void StartCooperativeRun();

	UFUNCTION(BlueprintCallable, Category = "Snake|Flow")
	void StartVersusRun();

	UFUNCTION(BlueprintCallable, Category = "Snake|flow")
	void StartPlayingRun();

	UFUNCTION(BlueprintCallable, Category = "Snake|flow")
	void RestartRun();

	UFUNCTION(BlueprintCallable, Category = "Snake|flow")
	void ReturnToMainMenu();

	UFUNCTION(BlueprintCallable, Category = "Snake|flow")
	void RespawnFruit_Temp();

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Snake|Feedback")
	TObjectPtr<USoundBase> FoodConsumedSound;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Snake|Feedback")
	TObjectPtr<USoundBase> SnakeDeathSound;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Snake|Feedback")
	TObjectPtr<USoundBase> StageCompleteSound;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Snake|Feedback")
	TObjectPtr<UNiagaraSystem> FoodConsumedEffect;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Snake|Feedback")
	TObjectPtr<UNiagaraSystem> SnakeDeathEffect;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Snake|Feedback")
	TObjectPtr<UNiagaraSystem> StageCompleteEffect;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Snake|UI")
	TSubclassOf<UUserWidget> MainMenuWidgetClass;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Snake|UI")
	TSubclassOf<UUserWidget> OutroWidgetClass;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Snake|UI")
	TSubclassOf<UUserWidget> HUDWidgetClass;

	UPROPERTY()
	TObjectPtr<UUserWidget> MainMenuWidgetInstance;

	UPROPERTY()
	TObjectPtr<UUserWidget> OutroWidgetInstance;

	UPROPERTY()
	TObjectPtr<UUserWidget> HUDWidgetInstance;

private:
	void CacheGridManager();
	void FindOrSpawnGridManager();

	bool IsCellFreeForGameplay(const FIntPoint& Cell) const;
	bool TryFindRandomFreeCell(FIntPoint& OutCell);
	bool TryFindRandomFreeCell_Flexible(FIntPoint& OutCell, const TArray<FIntPoint>& ForbiddenCells,
	                                    FRandomStream& RandomStream) const;

	void EnsureLocalPlayers();
	void SpawnSnakes();
	void SpawnSnakeForPlayer(int32 PlayerIndex, const FIntPoint& RequestedSpawnCell);

	FIntPoint GetSpawnCellForPlayer(int32 PlayerIndex) const;
	void ClearSpawnedActors();

	void SpawnFruit_Destructive(const FIntPoint& Cell);

	void LoadStage(int32 StageIndex);
	const USnakeStageSettingsDataAsset* GetStagePreset(int32 StageIndex) const;
	int32 GetStageCount() const;
	void CompleteStage();
	void CompleteRun();

	// UI 
	void ShowMainMenuWidget();
	void ShowOutroWidget();
	void ShowHUDWidget();
	void HideMenuWidgets();
	void HideHUDWidget();

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
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Snake|Stages",
		meta = (AllowPrivateAccess = "true"))
	TArray<TObjectPtr<USnakeStageSettingsDataAsset>> StagePresets;

	UPROPERTY(EditDefaultsOnly, Category = "Snake|SnakePawn",
		meta=(AllowPrivateAccess="true"))
	TSubclassOf<ASnakeGridwalkerPawn> SnakePawnClass;

	UPROPERTY(EditAnywhere, Category = "Snake|FoodActor",
		meta = (AllowPrivateAccess = "true"))
	TSubclassOf<AFoodActor> FoodActorClass;

	UPROPERTY(EditDefaultsOnly, Category = "Snake|Grid",
		meta=(AllowPrivateAccess="true"))
	TSubclassOf<AGridManagerActor> GridManagerClass;

	UPROPERTY(EditDefaultsOnly, Category = "Snake|Grid",
		meta=(AllowPrivateAccess="true"))
	FIntPoint SnakeSpawnCell = FIntPoint(0, 0);

	// Runtime set
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Snake|Runtime",
		meta = (AllowPrivateAccess = "true"))
	int32 ActiveLocalPlayerCount = 2;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Snake|Runtime",
		meta = (AllowPrivateAccess = "true"))
	bool bUseSharedKeyboardControls = false;

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
