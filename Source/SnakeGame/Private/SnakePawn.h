// Fill out your copyright notice in the Description page of Project Settings.

// SnakePawn.h
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "InputActionValue.h"
#include "SnakePawn.generated.h"

class UStaticMeshComponent;
class USphereComponent;
class USpringArmComponent;
class UCameraComponent;
class UInputMappingContext;
class UInputAction;


UENUM(BlueprintType)
enum class ETurnPriorityMode : uint8
{
	FirstPress,
	LastPress,
	ExclusiveInput
};


UCLASS()
class SNAKEGAME_API ASnakePawn : public APawn
{
	GENERATED_BODY()

public:
	// Sets default values for this pawn's properties
	ASnakePawn();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

private:
	void OnTurnLeftStarted(const FInputActionValue& Value);
	void OnTurnLeftReleased(const FInputActionValue& Value);

	void OnTurnRightStarted(const FInputActionValue& Value);
	void OnTurnRightReleased(const FInputActionValue& Value);

	float ResolveConflictDirection(const ETurnPriorityMode& PriorityMode) const;
	float GetTurnDirection() const;

	void UpdateRotationFromInput(float DeltaTime);
	void MoveTick(float DeltaTime);

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "SnakeBody|Component",
		meta = (AllowPrivateAccess = "true"))
	TObjectPtr<USphereComponent> CollisionSphere;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "SnakeBody|Component",
		meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UStaticMeshComponent> VisualMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "SnakeBody|Camera",
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
	TObjectPtr<UInputAction> TurnLeftAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "SnakeBody|Input",
		meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UInputAction> TurnRightAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "SnakeBody|Input",
		meta = (AllowPrivateAccess = "true"))
	ETurnPriorityMode TurnPriorityMode = ETurnPriorityMode::LastPress;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "SnakeBody|Movement", meta = (AllowPrivateAccess = "true"))
	float BaseMoveSpeed = 500.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "SnakeBody|Movement", meta = (AllowPrivateAccess = "true"))
	float BaseTurnRateDegPerSec = 120.0f;


	bool bLeftHeld = false;
	bool bRightHeld = false;

	uint32 LeftPressOrder = INDEX_NONE;
	uint32 RightPressOrder = INDEX_NONE;
	uint32 PressSequenceCounter = 0;


	// not used yet
	FVector CurrentPosition = FVector::ZeroVector;
	FVector PendingNextPosition = FVector::ZeroVector;

	// not used yet
	FRotator CurrentRotation = FRotator::ZeroRotator;
	FRotator PendingNextRotation = FRotator::ZeroRotator;


	/*
	//FTransform Transform;  =  Translation: FVector (X, Y, Z), Rotation: FQuat (Quaternion), Scale: FVector (X, Y, Z)  
	
	FTransform Transform;
	
	FVector CurrentPosition = FVector(0.0f, 0.0f, 0.0f);
	FVector TargetPosition = FVector(0.0f, 0.0f, 0.0f);

	// which one to keep? 
	FRotator CurrentRotation = FRotator(0.0f, 0.0f, 0.0f);
	FQuat CurrentRotationQuat = FQuat(0.0f, 0.0f, 0.0f, 0.0f);
	*/
};
