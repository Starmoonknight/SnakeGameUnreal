// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "SnakeSettingsTypes.h"
#include "SnakeSettingsDataAsset.generated.h"

/**
 * 
 */
UCLASS(BlueprintType)
class SNAKEGAME_API USnakeSettingsDataAsset : public UDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Snake")
	FSnakeStartupSettings StartupSettings;
};
