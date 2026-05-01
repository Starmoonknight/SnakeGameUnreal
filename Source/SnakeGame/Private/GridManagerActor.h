// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GridManagerActor.generated.h"

class UStaticMesh;
class UStaticMeshComponent;
class UInstancedStaticMeshComponent;
class UMaterialInterface;
class USceneComponent;


// Test to set up a Primary Data Asset, and from that a Data Asset, that can be used in blueprints 
// Not used in game but keeping in code to remember 
// 
// I think the steps was: 
// - Make a blueprint of the Primary Data Asset type and name it PDA_GridSettingsData, 
// - Open the new blueprint -> click plus on Variables -> name the variable "Data" and change the type from bool to GridSettings (since it was visible as a USTRUCT(Blueprintable) type)
// 
// - Then make a new blueprint again and select the PDA_GridSettingsData as parent class to make the DA_TestGrid.
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


UENUM()
enum class EGridCellType : uint8
{
	Empty,
	Blocked,
};


UCLASS()
class AGridManagerActor : public AActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	AGridManagerActor();

protected:
	virtual void OnConstruction(const FTransform& Transform) override;

	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;


	// Helpers
	UFUNCTION(BlueprintPure, Category = "Grid")
	int32 GetWidth() const { return GridDimensions.X; }

	UFUNCTION(BlueprintPure, Category = "Grid")
	int32 GetHeight() const { return GridDimensions.Y; }

	UFUNCTION(BlueprintPure, Category = "Grid")
	int32 GetCellCount() const { return GridDimensions.X * GridDimensions.Y; }

	UFUNCTION(BlueprintPure, Category = "Grid")
	int32 GetCellSize() const { return CellSize; }

	// don't use this 
	UFUNCTION(BlueprintPure, Category = "Grid")
	FIntPoint GetSnakeSpawnWorldLocation() const;

	// use this
	UFUNCTION(BlueprintPure, Category = "Grid")
	FIntPoint GetSnakeSpawnCell() const { return SnakeSpawnCell; }

	UFUNCTION(BlueprintPure, Category = "Grid")
	bool IsInBounds(const FIntPoint& Cell) const;

	UFUNCTION(BlueprintPure, Category = "Grid")
	FVector GridToWorld(const FIntPoint& Cell) const;

	UFUNCTION(BlueprintPure, Category = "Grid") // BlueprintCallable or BlueprintPure, and why? 
	FIntPoint WorldToGrid(const FVector& WorldLocation) const;

	UFUNCTION(BlueprintPure, Category = "Grid")
	bool IsCellBlockedByBoard(const FIntPoint& Cell) const;

	void InitializeGridForGameplay();

private:
	int32 FlatIndex(const FIntPoint& Cell) const;
	FIntPoint IndexToCellCoord(const int32 Index) const;

	UStaticMesh* GetWallMeshToUse() const;
	UStaticMesh* GetFloorMeshToUse() const;

	// board setup 
	void InitializeCells();
	void BuildTiledFloor();
	void SetupGridVisuals_Stretchy();


	// Fix height, rotation and direction to be grid-reliant instead of world space?  

	// base settings
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Grid|DefaultSettings",
		meta = (AllowPrivateAccess = "true", ClampMin = "1"))
	FIntPoint GridDimensions = FIntPoint(40, 40);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Grid|DefaultSettings",
		meta = (AllowPrivateAccess = "true"))
	FVector2D GridOrigin = FVector2D::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Grid|DefaultSettings",
		meta = (AllowPrivateAccess = "true"))
	float GridWorldZ = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Grid|DefaultSettings",
		meta = (AllowPrivateAccess = "true", ClampMin = "1"))
	int32 CellSize = 100;

	// don't use this 
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Grid|DefaultSettings",
		meta = (AllowPrivateAccess = "true"))
	TObjectPtr<AActor> SnakeSpawnPoint;

	// going to use this 
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Grid|DefaultSettings",
		meta = (AllowPrivateAccess = "true"))
	FIntPoint SnakeSpawnCell = FIntPoint(0, 0);

	// visuals
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Grid|DefaultSettings",
		meta = (AllowPrivateAccess = "true"))
	bool bHasFancyWalls = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Grid|DefaultSettings",
		meta = (AllowPrivateAccess = "true"))
	bool bHasFancyFloor = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Grid|Assets",
		meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UStaticMesh> WallMeshAsset;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Grid|Assets",
		meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UStaticMesh> FloorMeshAsset;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Grid|Assets",
		meta=(AllowPrivateAccess="true"))
	TObjectPtr<UStaticMesh> FallbackCubeMesh;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Grid|Assets",
		meta=(AllowPrivateAccess="true"))
	TObjectPtr<UStaticMesh> FallbackPlaneMesh;

	// components
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Grid|Components",
		meta=(AllowPrivateAccess="true"))
	TObjectPtr<USceneComponent> SceneRoot;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Grid|Components",
		meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UInstancedStaticMeshComponent> FloorInstances;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Grid|Components",
		meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UInstancedStaticMeshComponent> WallInstances;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Grid|Components",
		meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UStaticMeshComponent> FloorVisuals;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Grid|Components",
		meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UStaticMeshComponent> NorthWallVisual;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Grid|Components",
		meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UStaticMeshComponent> SouthWallVisual;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Grid|Components",
		meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UStaticMeshComponent> EastWallVisual;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Grid|Components",
		meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UStaticMeshComponent> WestWallVisual;

	// runtime values 
	TArray<EGridCellType> Cells;
};
