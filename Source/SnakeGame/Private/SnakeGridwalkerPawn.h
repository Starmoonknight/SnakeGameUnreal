// Fill out your copyright notice in the Description page of Project Settings.

//SnakeGridwalkerPawn.h
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "InputActionValue.h"
#include "SnakeGridwalkerPawn.generated.h"

class UStaticMeshComponent;
class USphereComponent;
class USpringArmComponent;
class UCameraComponent;
class UInputMappingContext;
class UInputAction;


UENUM(BlueprintType)
enum class EGridDirection : uint8
{
	UP,
	DOWN,
	LEFT,
	RIGHT,
	NONE
};


UCLASS()
class ASnakeGridwalkerPawn : public APawn
{
	GENERATED_BODY()

public:
	// Sets default values for this pawn's properties
	ASnakeGridwalkerPawn();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

private:
	EGridDirection ResolveDirectionFromInput(const FVector2D& Input) const;
	void Move(const FInputActionValue& Value);

	void StepMove();


	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "SnakeBody|Components",
		meta = (AllowPrivateAccess = "true"))
	TObjectPtr<USphereComponent> CollisionSphere;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "SnakeBody|Components",
		meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UStaticMeshComponent> VisualMesh;

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

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "SnakeBody|Movement",
		meta = (AllowPrivateAccess = "true"))
	float MoveSpeed = 500.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "SnakeBody|Movement",
		meta = (AllowPrivateAccess = "true"))
	float TurnSpeed = 120.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "SnakeBody|Movement",
		meta = (AllowPrivateAccess = "true"))
	float StepInterval = 0.5f;

	float StepAccumulator = 0.0f;

	FVector2D RawMoveInput = FVector2D::ZeroVector;
	EGridDirection CurrentDirection = EGridDirection::UP;
	EGridDirection PendingNextDirection = EGridDirection::UP;

	FIntPoint GridPosition = FIntPoint::ZeroValue;

	FVector2D GridOrigin = FVector2D::ZeroVector;
	float CellSize = 100.0f;


	/*
	FVector CurrentPosition = FVector::ZeroVector;
	FVector PendingNextPosition = FVector::ZeroVector;
	
	FIntPoint CurrentGridCell = FIntPoint::ZeroValue;
	FIntPoint PendingNextGridCell = FIntPoint::ZeroValue;
	*/
};
