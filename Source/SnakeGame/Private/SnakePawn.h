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
	void Turn(const FInputActionValue& Value);

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
	TObjectPtr<UInputAction> TurnAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "SnakeBody|Movement", meta = (AllowPrivateAccess = "true"))
	float BaseMoveSpeed = 500.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "SnakeBody|Movement", meta = (AllowPrivateAccess = "true"))
	float BaseTurnSpeed = 120.0f;

	float TurnInput = 0.0f;


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
