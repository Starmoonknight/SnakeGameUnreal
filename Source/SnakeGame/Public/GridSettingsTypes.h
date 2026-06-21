// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GridSettingsTypes.generated.h"


/**
 * 
 */
USTRUCT(BlueprintType)
struct FGridStartupSettings
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category = "GridSettings",
		meta = (AllowPrivateAccess = "true", ClampMin = "3", UIMin = "3"))
	FIntPoint GridDimensions = FIntPoint(40, 40);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "GridSettings",
		meta = (ClampMin = "0", UIMin = "0"))
	int32 InternalWallSpacing = 0;

	UPROPERTY(EditAnywhere, Category = "GridSettings",
		meta = (AllowPrivateAccess = "true"))
	FVector2D GridOrigin = FVector2D::ZeroVector;

	UPROPERTY(EditAnywhere, Category = "GridSettings",
		meta = (AllowPrivateAccess = "true"))
	float GridWorldZ = 0.f;

	UPROPERTY(EditAnywhere, Category = "GridSettings",
		meta = (AllowPrivateAccess = "true", ClampMin = "1"))
	int32 CellSize = 100;

	UPROPERTY(EditAnywhere, Category = "GridSettings",
		meta = (AllowPrivateAccess = "true"))
	int32 RootSeed = 12345;
};
