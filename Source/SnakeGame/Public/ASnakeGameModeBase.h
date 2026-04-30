// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include  "SnakeSettingsTypes.h"
#include  "GridSettingsTypes.h"

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "ASnakeGameModeBase.generated.h"

class AASnakeGridwalkerPawn;
class AFoodActor;
class AGridManager;
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

	UFUNCTION(BlueprintCallable, Category = Snake)
	void StartPlayingRun();

	UFUNCTION(BlueprintCallable, Category = Snake)
	void RestartRun();

	AASnakeGridwalkerPawn* SpawnSnakeForSlot(int32 SlotIndex, const FIntPoint& SpawnCell);
	void SpawnSnake();

private:
	UPROPERTY(EditDefaultsOnly, Category = "Snake")
	TSubclassOf<AASnakeGridwalkerPawn> SnakePawnClass;

	UPROPERTY(EditDefaultsOnly, Category = "Snake")
	TSubclassOf<AAFoodActor> FoodActorClass;

	UPROPERTY()
	TObjectPtr<AASnakeGridwalkerPawn> SpawnedSnakePawn;

	UPROPERTY()
	TObjectPtr<AAFoodActor> SpawnedFoodActor;

	UPROPERTY()
	TObjectPtr<AGridManager> GridManager;


	int32 CurrentStageIndex = 0;
	int32 PointsThisStage = 0;
	
	
};
