// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "GridDirectionTypes.h"

#include "CoreMinimal.h"
#include  "SnakeSettingsTypes.generated.h"

/**
 * 
 */
USTRUCT(BlueprintType)
struct FSnakeStartupSettings
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Snake|Movement",
		meta = (ClampMin = "0.01", UIMin = "0.01"))
	float StepInterval = 0.8f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Snake|Movement",
		meta = (ClampMin = "0.01", UIMin = "0.01"))
	float TurnDuration = 0.15f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Snake|Movement")
	EGridDirection StartingDirection = EGridDirection::Up;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Snake|Growth",
		meta = (ClampMin = "0", UIMin = "0"))
	int32 InitialPendingGrowth = 0;
};
