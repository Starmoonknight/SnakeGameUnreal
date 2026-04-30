// Fill out your copyright notice in the Description page of Project Settings.

#pragma once


#include "CoreMinimal.h"
#include "GameFramework/GameStateBase.h"
#include "ASnakeGameState.generated.h"

UENUM(BlueprintType)
enum class ESnakeGamePhase : uint8
{
	MainMenu UMETA(DisplayName = "Main Menu"),
	Playing UMETA(DisplayName = "Playing"),
	Outro UMETA(DisplayName = "Outro"),
	None UMETA(DisplayName = "None"),
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(
	FOnPhaseChangedSignature,
	ESnakeGamePhase, NewPhase);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(
	FOnScoreChangedSignature,
	int32, NewScore);


/**
 * 
 */
UCLASS()
class ASnakeGameStatic : public AGameStateBase
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintReadOnly, Category = "Snake")
	int32 Score = 0;

	UPROPERTY(BlueprintReadOnly, Category = "Snake")
	int32 CurrentStageIndex = 0;

	UPROPERTY(BlueprintReadOnly, Category = "Snake")
	int32 FoodEatenThisStage = 0;

	UPROPERTY(BlueprintReadOnly, Category = "Snake")
	int32 PointsGainedThisStage = 0;

	UPROPERTY(BlueprintReadOnly, Category = "Snake")
	int32 PointsToClearStage = 0;

	UPROPERTY(BlueprintReadOnly, Category = "Snake")
	FOnPhaseChangedSignature OnPhaseChanged;

	UPROPERTY(BlueprintReadOnly, Category = "Snake")
	FOnScoreChangedSignature OnScoreChanged;

	UPROPERTY(BlueprintReadOnly, Category = "Snake")
	ESnakeGamePhase MatchPhase = ESnakeGamePhase::None;

	void AddScore(int32 NewScore)
	{
		Score += NewScore;
		OnScoreChanged.Broadcast(Score);
	}

	void SetMatchPhase(ESnakeGamePhase NewPhase)
	{
		MatchPhase = NewPhase;
		OnPhaseChanged.Broadcast(NewPhase);
	}
};
