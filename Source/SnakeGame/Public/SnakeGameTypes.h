#pragma once

#include "CoreMinimal.h"
#include "SnakeGameTypes.generated.h"


UENUM(BlueprintType)
enum class ESnakeGameModeType : uint8
{
	SinglePlayer UMETA(DisplayName = "Single Player"),
	Cooperative UMETA(DisplayName = "Cooperative"),
	Versus UMETA(DisplayName = "Versus")
};


UENUM(BlueprintType)
enum class ESnakeBattleResult : uint8
{
	None,
	Player0Won UMETA(DisplayName = "Player 1 Wins"),
	Player1Won UMETA(DisplayName = "Player 2 Wins"),
	Draw UMETA(DisplayName = "Draw")
};


UENUM(BlueprintType)
enum class ESnakeRunEndReason : uint8
{
	None UMETA(DisplayName = "None"),
	SnakeDied UMETA(DisplayName = "Snake Died"),
	AllStagesCleared UMETA(DisplayName = "All Stages Cleared")
};
