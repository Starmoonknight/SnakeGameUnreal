// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GridDirectionTypes.generated.h"


/**
 * 
 */
USTRUCT(BlueprintType)
struct FGridSettings
{
	GENERATED_BODY()
	UPROPERTY(EditAnywhere, Category = "Grid|Settings",
		meta = (AllowPrivateAccess = "true", ClampMin = "1"))
	FIntPoint GridDimensions = FIntPoint(40, 40);

	UPROPERTY(EditAnywhere, Category = "Grid|Settings",
		meta = (AllowPrivateAccess = "true"))
	FVector2D GridOrigin = FVector2D::ZeroVector;

	UPROPERTY(EditAnywhere, Category = "Grid|Settings",
		meta = (AllowPrivateAccess = "true"))
	float GridWorldZ = 0.f;

	UPROPERTY(EditAnywhere, Category = "Grid|Settings",
		meta = (AllowPrivateAccess = "true", ClampMin = "1"))
	int32 CellSize = 100;

	UPROPERTY(EditAnywhere, Category = "Grid|Settings",
		meta = (AllowPrivateAccess = "true"))
	int32 RootSeed = 12345;
};
