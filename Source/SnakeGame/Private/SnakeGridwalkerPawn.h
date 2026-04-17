// Fill out your copyright notice in the Description page of Project Settings.

//SnakeGridwalkerPawn.h
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "InputActionValue.h"
#include "SnakeGridwalkerPawn.generated.h"

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
class ASnakeGridwalkerPawn : public APawn
{
	GENERATED_BODY()

public:
	// Sets default values for this pawn's properties
	ASnakeGridwalkerPawn();

protected:
	virtual void OnConstruction(const FTransform& Transform) override;

	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	void RequestGrowth(int32 Amount = 1);

private:
	void OnGrowPressed(const FInputActionValue& Value);

	static EGridDirection ResolveDirectionFromInput(const FVector2D& Input);
	void OnMoveInput(const FInputActionValue& Value);
	bool TryConsumeGrowth();

	FVector CellToWorld(const FIntPoint Cell) const;

	static float DirectionToYaw(EGridDirection Direction);
	void StartTurnVisual(EGridDirection NewDirection);
	void UpdateTurnVisual(float DeltaTime);

	void ApplyPendingDirection();
	FIntPoint PeekNextHeadCell() const;

	void UpdateHeadWorldLocation(const FIntPoint& NextHeadCell);
	void AdvanceBodySegments(FIntPoint VacatedCell);
	void AdvanceSnakeOneStep();

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

	UPROPERTY(VisibleAnywhere, Category = "SnakeBody|Components") // for showing property in property windows only
	TObjectPtr<UStaticMeshComponent> VisualMesh;

	UPROPERTY(VisibleAnywhere, Category = "SnakeBody|Components")
	TObjectPtr<UInstancedStaticMeshComponent> VisualSegmentMesh;

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

	float CellSize = 100.0f;
	FVector2D GridOrigin = FVector2D::ZeroVector;
	FIntPoint GridPosition = FIntPoint::ZeroValue;

	bool IsAlive = true;

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
	EGridDirection PendingNextDirection = EGridDirection::Up;


	//TArray<TWeakObjectPtr<>>

	/*
	FVector CurrentPosition = FVector::ZeroVector;
	FVector PendingNextPosition = FVector::ZeroVector;
	
	FIntPoint CurrentGridCell = FIntPoint::ZeroValue;
	FIntPoint PendingNextGridCell = FIntPoint::ZeroValue;
	*/
};
