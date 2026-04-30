#pragma once

#include "CoreMinimal.h"
#include "GridDirectionTypes.generated.h"

UENUM(BlueprintType)
enum class EGridDirection : uint8
{
	// used for grid direction inputs 
	// None = no valid input this step, remember to not use as a pause or add more choices 
	None UMETA(DisplayName = "None"),
	Up UMETA(DisplayName = "Up"),
	Down UMETA(DisplayName = "Down"),
	Left UMETA(DisplayName = "Left"),
	Right UMETA(DisplayName = "Right")
};
