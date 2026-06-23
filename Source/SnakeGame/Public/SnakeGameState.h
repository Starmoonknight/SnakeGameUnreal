// Fill out your copyright notice in the Description page of Project Settings.

#pragma once


#include "CoreMinimal.h"
#include "GameFramework/GameStateBase.h"
#include "SnakeGameTypes.h"
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

	UPROPERTY(BlueprintReadOnly, Category = "Snake|Score")
	TArray<int32> PlayerScores;

	UPROPERTY(BlueprintReadOnly, Category = "Snake|Score")
	int32 CurrentStageIndex = 0;

	UPROPERTY(BlueprintReadOnly, Category = "Snake|Score")
	int32 TotalStagesAmount = 0;

	UPROPERTY(BlueprintReadOnly, Category = "Snake|Score")
	int32 FoodEatenThisStage = 0;

	UPROPERTY(BlueprintReadOnly, Category = "Snake|Score")
	int32 PointsGainedThisStage = 0;

	UPROPERTY(BlueprintReadOnly, Category = "Snake|Score")
	int32 PointsToClearStage = 0;

	UPROPERTY(BlueprintAssignable, Category = "Snake|Score")
	FOnScoreChangedSignature OnScoreChanged;

	UPROPERTY(BlueprintAssignable, Category = "Snake")
	FOnPhaseChangedSignature OnPhaseChanged;

	UPROPERTY(BlueprintReadOnly, Category = "Snake")
	ESnakeMatchPhase MatchPhase = ESnakeMatchPhase::None;

	UPROPERTY(BlueprintReadOnly, Category = "Snake")
	ESnakeGameModeType PlayMode = ESnakeGameModeType::SinglePlayer;

	UPROPERTY(BlueprintReadOnly, Category = "Snake|Battle")
	ESnakeBattleResult BattleResult = ESnakeBattleResult::None;

	UPROPERTY(BlueprintReadOnly, Category = "Snake|Result")
	ESnakeRunEndReason RunEndReason = ESnakeRunEndReason::None;


	UFUNCTION(BlueprintPure, Category = "Snake|Score")
	int32 GetPlayerScore(const int32 PlayerIndex) const
	{
		return PlayerScores.IsValidIndex(PlayerIndex)
			       ? PlayerScores[PlayerIndex]
			       : 0;
	}

	UFUNCTION(BlueprintPure, Category = "Snake|Score")
	TArray<int32> GetAllPlayerScores(const int32 PlayerCount) const
	{
		TArray<int32> OutScores;
		OutScores.Init(0, FMath::Max(PlayerCount, 0));

		for (int32 Index = 0; Index < OutScores.Num(); ++Index)
		{
			if (PlayerScores.IsValidIndex(Index))
			{
				OutScores[Index] = PlayerScores[Index];
			}
		}

		return OutScores;
	}

	void ResetScores(const int32 PlayerCount)
	{
		Score = 0;
		PlayerScores.Init(0, FMath::Max(PlayerCount, 0));
		OnScoreChanged.Broadcast(Score);
	}

	void AddScore(const int32 Amount)
	{
		Score += Amount;
		OnScoreChanged.Broadcast(Score);
	}

	void AddScoreForPlayer(const int32 PlayerIndex, const int32 Amount)
	{
		if (PlayerScores.IsValidIndex(PlayerIndex))
		{
			PlayerScores[PlayerIndex] += Amount;
		}

		Score += Amount;
		OnScoreChanged.Broadcast(Score);
	}


	void SetMatchPhase(ESnakeMatchPhase NewPhase)
	{
		MatchPhase = NewPhase;
		OnPhaseChanged.Broadcast(NewPhase);
	}
};
