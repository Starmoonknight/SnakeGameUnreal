// Fill out your copyright notice in the Description page of Project Settings.

//SnakeGridwalkerPawn.h
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "InputActionValue.h"
#include "ASnakeGridwalkerPawn.generated.h"

class AAGridManagerActor;

class UStaticMesh;
class UStaticMeshComponent;
class UInstancedStaticMeshComponent;
class UMaterialInterface;
class USphereComponent;
class USpringArmComponent;
class UCameraComponent;
class UInputMappingContext;
class UInputAction;


UENUM(BlueprintType)
enum class EGridDirection : uint8
{
	Up,
	Down,
	Left,
	Right,
	None // None = no valid input this step, remember to not use as a pause or add more choices 
};


UCLASS()
class AASnakeGridwalkerPawn : public APawn
{
	GENERATED_BODY()

public:
	// Sets default values for this pawn's properties
	AASnakeGridwalkerPawn();

protected:
	virtual void OnConstruction(const FTransform& Transform) override;

	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	// --- Information getters ---
#pragma region Snake Queries

	// Counts
	UFUNCTION(BlueprintPure, Category="Snake")
	int32 GetSnakeTotalSize() const { return BodyCells.Num() + 1; }

	UFUNCTION(BlueprintPure, Category="Snake")
	int32 GetBodySegmentCount() const { return BodyCells.Num(); }

	// Positions
	UFUNCTION(BlueprintPure, Category="Snake")
	FIntPoint GetHeadCellPosition() const { return GridCellHeadPosition; }

	UFUNCTION(BlueprintPure, Category="Snake")
	const TArray<FIntPoint>& GetBodyCellPositions() const { return BodyCells; }

	// Occupancy
	UFUNCTION(BlueprintPure, Category="Snake")
	bool IsHeadAtCell(const FIntPoint& Cell) const { return Cell == GridCellHeadPosition; }

	UFUNCTION(BlueprintPure, Category="Snake")
	bool HasBodySegmentAtCell(const FIntPoint& Cell) const { return BodyCells.Contains(Cell); }

	UFUNCTION(BlueprintPure, Category="Snake")
	bool IsSnakeAtCell(const FIntPoint& Cell) const { return (IsHeadAtCell(Cell) || HasBodySegmentAtCell(Cell)); }

	// Body-Segment Access/Lookup 
	UFUNCTION(BlueprintPure, Category="Snake")
	bool TryGetBodyCellPositionByIndex(int32 SegmentIndex, FIntPoint& OutCell) const;

	UFUNCTION(BlueprintPure, Category="Snake")
	bool TryFindBodyIndexAtCell(const FIntPoint& Cell, int32& OutSegmentIndex) const;

#pragma endregion

	// --- Logic setters --- 
	UFUNCTION(BlueprintCallable, Category="Snake")
	void ResetSnake();

	UFUNCTION(BlueprintCallable, Category="Snake")
	void RequestGrowth(int32 Amount = 1);

private:
	// Helpers
	FTransform MakeBodyInstanceLocalTransform(const FIntPoint& BodyCell) const;
	static EGridDirection ResolveDirectionFromInput(const FVector2D& Input);

	// Input callbacks 
	void Input_OnMove(const FInputActionValue& Value);
	void Input_OnGrowPressed();
	void Input_OnResetPressed();

	void StartHeadTurnVisual(EGridDirection NewDirection);
	void UpdateHeadTurnVisual(float DeltaTime);

	void AddBodyVisualSegment(const FIntPoint& BodyCell);
	void UpdateBodyVisualTransforms();
	void SyncBodyVisuals();

	bool TryConsumeGrowth();
	void ApplyPendingDirection();
	FIntPoint PeekNextHeadCell() const;

	void UpdateHeadWorldLocation(const FIntPoint& NextHeadCell);
	void AdvanceBodySegments(FIntPoint VacatedCell);
	void AdvanceSnakeOneStep();


	UPROPERTY(EditAnywhere, Category = "SnakeBody|References", meta=(AllowPrivateAccess="true"))
	TObjectPtr<AAGridManagerActor> GridManager;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SnakeBody|Assets",
		meta = (AllowPrivateAccess = "true")) // place for actually assigning the mesh used
	TObjectPtr<UStaticMesh> HeadMeshAsset;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SnakeBody|Assets",
		meta = (AllowPrivateAccess = "true")) // create editor menu option for changing material on StaticMesh used
	TObjectPtr<UMaterialInterface> HeadMaterialAsset;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SnakeBody|Assets",
		meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UStaticMesh> SegmentMeshAsset;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SnakeBody|Assets",
		meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UMaterialInterface> SegmentMaterialAsset;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "SnakeBody|Components",
		meta = (AllowPrivateAccess = "true"))
	TObjectPtr<USphereComponent> CollisionSphere;

	UPROPERTY(VisibleInstanceOnly, Category = "SnakeBody|Components") // for showing property in property windows only,
	TObjectPtr<UStaticMeshComponent> SnakeHeadMesh; // only kept visible for debug purpose 

	UPROPERTY(VisibleInstanceOnly, Category = "SnakeBody|Components") // for showing property in property windows only,
	TObjectPtr<UInstancedStaticMeshComponent> VisualSegmentMesh; // only kept visible for debug purpose 

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "SnakeBody|Components",
		meta = (AllowPrivateAccess = "true"))
	TObjectPtr<USpringArmComponent> SpringArm;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "SnakeBody|Camera",
		meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UCameraComponent> Camera;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "SnakeBody|Input",
		meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UInputMappingContext> DefaultMappingContext;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "SnakeBody|Input",
		meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UInputAction> MoveAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "SnakeBody|Input",
		meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UInputAction> TestGrowthAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "SnakeBody|Input",
		meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UInputAction> TestResetAction;

	/*
	Reminder:
	May later switch from direct time values to speed-based tuning.

	Possible conversions:
	- If MoveSpeed is world units per second:
		StepInterval = CellSize / MoveSpeed;
	- If using CellsPerSecond instead:
		StepInterval = 1.0f / CellsPerSecond;

	Same general idea for turning:
	- Turn duration could be derived from a turn speed instead of entered directly.

	For current MVP, StepInterval / TurnDuration are simpler because they are the actual timing values in seconds.
	*/
	/*
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "SnakeBody|Movement",
		meta = (AllowPrivateAccess = "true"))
	float MoveSpeed = 500.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "SnakeBody|Movement",
		meta = (AllowPrivateAccess = "true"))
	float TurnSpeed = 120.0f;
	*/

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "SnakeBody|Movement",
		meta = (AllowPrivateAccess = "true", ClampMin = "0.01", UIMin = "0.01"))
	float StepInterval = 0.8f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "SnakeBody|Movement",
		meta = (AllowPrivateAccess = "true", ClampMin = "0.01", UIMin = "0.01"))
	float TurnDuration = 0.15f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "SnakeBody|Grid",
		meta = (AllowPrivateAccess = "true"))
	FIntPoint SpawnCell = FIntPoint::ZeroValue;

	FIntPoint GridCellHeadPosition = FIntPoint::ZeroValue;

	bool bIsAlive = true;

	float StepAccumulator = 0.0f;
	float TurnRotationElapsed = 0.0f;

	// switch to using TRotator later when I need pitch/roll too
	float TurnStartYaw = 0.0f;
	float TurnTargetYaw = 0.0f;
	bool IsTurning = false;

	int32 PendingGrowth = 0;
	TArray<FIntPoint> BodyCells;

	FVector2D RawMoveInput = FVector2D::ZeroVector;
	EGridDirection CurrentDirection = EGridDirection::Up;
	EGridDirection PendingNextDirection = EGridDirection::None;


	//TArray<TWeakObjectPtr<>>

	/*
	FVector CurrentPosition = FVector::ZeroVector;
	FVector PendingNextPosition = FVector::ZeroVector;
	
	FIntPoint CurrentGridCell = FIntPoint::ZeroValue;
	FIntPoint PendingNextGridCell = FIntPoint::ZeroValue;
	*/
};
