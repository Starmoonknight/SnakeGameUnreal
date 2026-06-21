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
	Player0Won,
	Player1Won,
	Draw
};
