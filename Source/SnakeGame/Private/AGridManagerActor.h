// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "AGridManagerActor.generated.h"

class AAFoodActor;
class UStaticMesh;
class UStaticMeshComponent;
class UInstancedStaticMeshComponent;
class UMaterialInterface;
class USceneComponent;

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
	bool IsInBounds(const FIntPoint& Cell) const;

	UFUNCTION(BlueprintPure, Category = "Grid")
	int32 ToIndex(const FIntPoint& Cell) const;

	UFUNCTION(BlueprintPure, Category = "Grid") // BlueprintCallable or BlueprintPure, and why? 
	FIntPoint WorldToCell(const FVector& WorldLocation) const;

	UFUNCTION(BlueprintPure, Category = "Grid")
	FVector CellToWorld(const FIntPoint Cell) const;

	UFUNCTION(BlueprintPure, Category = "Grid")
	bool IsCellBlockedByBoard(const FIntPoint& Cell) const;


	UFUNCTION(BlueprintPure, Category = "Grid")
	bool IsFoodAtCell_Temp(const FIntPoint& Cell) const;


	UFUNCTION(BlueprintCallable, Category = "Grid")
	bool TryFindRandomFreeCell(FIntPoint& OutCell, const TArray<FIntPoint>& ForbiddenCells,
	                           int32 MaxAttempts = 200) const;

	UFUNCTION(BlueprintPure, Category = "Grid|Queries")
	bool CanPlaceFruitAtCell_Temp(const FIntPoint& Cell) const;

	UFUNCTION(BlueprintCallable, Category = "Grid|Food")
	void SpawnFruitAtCell_Temp(const FIntPoint& Cell);

	UFUNCTION(BlueprintCallable, Category = "Grid|Food")
	void RespawnFruit_Temp();

private:
	// math 
	FVector2D GetBoardWorldSize() const;
	FVector GetBoardWorldCenter() const;
	void CalculateBoardSize();

	UStaticMesh* GetWallMeshToUse() const;
	UStaticMesh* GetFloorMeshToUse() const;

	// board setup 
	void InitializeCells();
	void BuildTiledFloor();
	void RebuildGridVisuals();


	UPROPERTY(EditAnywhere, Category = "Grid|Food", meta = (AllowPrivateAccess = "true"))
	TSubclassOf<AAFoodActor> FoodClass;

	UPROPERTY(Transient, Category = "Grid|Food", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<AAFoodActor> CurrentFood;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Grid|Settings",
		meta = (AllowPrivateAccess = "true", ClampMin = "1"))
	FIntPoint GridDimensions = FIntPoint(40, 40);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Grid|Settings",
		meta = (AllowPrivateAccess = "true"))
	FVector2D GridOrigin = FVector2D::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Grid|Settings",
		meta = (AllowPrivateAccess = "true", ClampMin = "1"))
	int CellSize = 100;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Grid|Settings",
		meta = (AllowPrivateAccess = "true"))
	float GridWorldZ = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Grid|Settings",
		meta = (AllowPrivateAccess = "true"))
	int32 RandomSeed = 12345;

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

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Grid|Components",
		meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UStaticMeshComponent> FloorVisuals;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Grid|Components",
		meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UStaticMeshComponent> NorthWallVisual;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Grid|Components",
		meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UStaticMeshComponent> SouthWallVisual;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Grid|Components",
		meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UStaticMeshComponent> EastWallVisual;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Grid|Components",
		meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UStaticMeshComponent> WestWallVisual;


	TArray<EGridCellType> Cells;

	FRandomStream RandomStream;
};
