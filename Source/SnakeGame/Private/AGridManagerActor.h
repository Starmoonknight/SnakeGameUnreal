// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "AGridManagerActor.generated.h"

class AAFoodActor;
class AASnakeGridwalkerPawn;
class UStaticMesh;
class UStaticMeshComponent;
class UInstancedStaticMeshComponent;
class UMaterialInterface;
class USceneComponent;


// Test to set up a Primary Data Asset, and from that a Data Asset, that can be used in blueprints 
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
class AAGridManagerActor : public AActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	AAGridManagerActor();

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

	UFUNCTION(BlueprintPure, Category = "Grid")
	bool IsInBounds(const FIntPoint& Cell) const;

	UFUNCTION(BlueprintPure, Category = "Grid")
	FVector CellToWorld(const FIntPoint& Cell) const;

	UFUNCTION(BlueprintPure, Category = "Grid") // BlueprintCallable or BlueprintPure, and why? 
	FIntPoint WorldToCell(const FVector& WorldLocation) const;

	UFUNCTION(BlueprintPure, Category = "Grid")
	bool IsCellBlockedByBoard(const FIntPoint& Cell) const;


	UFUNCTION(BlueprintPure, Category = "Grid")
	bool IsFoodAtCell_Temp(const FIntPoint& Cell) const;

	UFUNCTION(BlueprintCallable, Category = "Grid|Food")
	void RespawnFruit_Temp();

private:
	UFUNCTION()
	void HandleFruitConsumed_Temp(AAFoodActor* Food, AActor* ConsumerActor);

	UFUNCTION()
	void HandleSnakeDeath_Temp(AASnakeGridwalkerPawn* DeadSnake);

	int32 FlatIndex(const FIntPoint& Cell) const;
	FIntPoint IndexToCellCoord(const int32 Index) const;

	UStaticMesh* GetWallMeshToUse() const;
	UStaticMesh* GetFloorMeshToUse() const;

	void PlaceSnakeOnGrid_Temp();

	bool CanPlaceFruitAtCell_Temp(const FIntPoint& Cell) const;
	bool TryFindRandomFreeCell_Temp(FIntPoint& OutCell);
	void SpawnFruitAtCellDestructive_Temp(const FIntPoint& Cell);

	// board setup 
	void InitializeCells();
	void BuildTiledFloor();
	void SetupGridVisuals_Stretchy();

	// Game 
	void StartGameLoop_Temp();
	void StopGameLoop_Temp();

	// Add in a spawn point to grid later, that snake can access for start location. 
	// Fix height, rotation and direction to be grid-reliant instead of world space?  

	UPROPERTY(EditInstanceOnly, Category = "Grid|Snake", meta=(AllowPrivateAccess="true"))
	TObjectPtr<AActor> SnakeSpawnPoint;

	UPROPERTY(EditInstanceOnly, Category = "Grid|Snake", meta=(AllowPrivateAccess="true"))
	TSubclassOf<AASnakeGridwalkerPawn> SnakeClass;

	UPROPERTY(Transient, VisibleInstanceOnly, BlueprintReadOnly, Category = "Grid|Snake",
		meta=(AllowPrivateAccess="true"))
	TObjectPtr<AASnakeGridwalkerPawn> Snake;

	UPROPERTY(EditAnywhere, Category = "Grid|Food", meta = (AllowPrivateAccess = "true"))
	TSubclassOf<AAFoodActor> FoodClass;

	// Grid owns the current spawned food reference for this MVP.
	// Food broadcasts when consumed; grid responds by spawning the next fruit.
	UPROPERTY(Transient, VisibleInstanceOnly, BlueprintReadOnly, Category = "Grid|Food",
		meta = (AllowPrivateAccess = "true"))
	TObjectPtr<AAFoodActor> CurrentFood;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Grid|Settings",
		meta = (AllowPrivateAccess = "true", ClampMin = "1"))
	FIntPoint GridDimensions = FIntPoint(40, 40);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Grid|Settings",
		meta = (AllowPrivateAccess = "true"))
	FVector2D GridOrigin = FVector2D::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Grid|Settings",
		meta = (AllowPrivateAccess = "true"))
	float GridWorldZ = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Grid|Settings",
		meta = (AllowPrivateAccess = "true", ClampMin = "1"))
	int32 CellSize = 100;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Grid|Settings",
		meta = (AllowPrivateAccess = "true"))
	int32 RootSeed = 12345;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Grid|Settings",
		meta = (AllowPrivateAccess = "true"))
	bool bHasFancyWalls = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Grid|Settings",
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

	TArray<EGridCellType> Cells;


	FRandomStream FoodRandomStream;
};
