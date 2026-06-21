// SnakeStageSettingsDataAsset.h
#pragma once


#include "CoreMinimal.h"
#include "Engine/DataAsset.h"

#include "GridSettingsDataAsset.h"
#include "SnakeSettingsTypes.h"
#include "SnakeSettingsDataAsset.h"
#include "SnakeStageSettingsDataAsset.generated.h"

/**
 * 
 */
UCLASS(BlueprintType)
class SNAKEGAME_API USnakeStageSettingsDataAsset : public UDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "SnakeStage")
	FSnakeStageSettings StageSettings;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "SnakeStage")
	TObjectPtr<UGridSettingsDataAsset> GridSettingsPreset;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "SnakeStage")
	TObjectPtr<USnakeSettingsDataAsset> SnakeSettingsPreset;
};
