// Fill out your copyright notice in the Description page of Project Settings.

#pragma once


#include "CoreMinimal.h"
#include "GameFramework/GameStateBase.h"
#include "SnakeGameState.generated.h"

UENUM(BlueprintType)
enum class ESnakeMatchPhase : uint8
{
	MainMenu UMETA(DisplayName = "Main Menu"),
	Playing UMETA(DisplayName = "Playing"),
	Outro UMETA(DisplayName = "Outro"),
	None UMETA(DisplayName = "None"),
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(
	FOnPhaseChangedSignature,
	ESnakeMatchPhase, NewPhase);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(
	FOnScoreChangedSignature,
	int32, NewScore);


/**
 * 
 */
UCLASS()
class ASnakeGameState : public AGameStateBase
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

	UPROPERTY(BlueprintAssignable, Category = "Snake")
	FOnPhaseChangedSignature OnPhaseChanged;

	UPROPERTY(BlueprintAssignable, Category = "Snake")
	FOnScoreChangedSignature OnScoreChanged;

	UPROPERTY(BlueprintReadOnly, Category = "Snake")
	ESnakeMatchPhase MatchPhase = ESnakeMatchPhase::None;

	void AddScore(int32 NewScore)
	{
		Score += NewScore;
		OnScoreChanged.Broadcast(Score);
	}

	void SetMatchPhase(ESnakeMatchPhase NewPhase)
	{
		MatchPhase = NewPhase;
		OnPhaseChanged.Broadcast(NewPhase);
	}
};
