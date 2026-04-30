// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include  "GridSettingsTypes.h"
#include  "GridSettingsDataAsset.generated.h"

/**
 * 
 */
UCLASS(BlueprintType)
class SNAKEGAME_API UGridSettingsDataAsset : public UDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Grid")
	FGridStartupSettings GridSettings;
};
