// Fill out your copyright notice in the Description page of Project Settings.

// BoxRoverPawn.cpp
#include "BoxRoverPawn.h"

#include "Camera/CameraComponent.h"
#include "Components/InputComponent.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/SpringArmComponent.h"
#include "InputAction.h"
#include "InputMappingContext.h"

// Sets default values
ABoxRoverPawn::ABoxRoverPawn()
{
	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	CollisionSphere = CreateDefaultSubobject<USphereComponent>("CollisionSphere");
	SetRootComponent(CollisionSphere);
	CollisionSphere->InitSphereRadius(50.f);
}

// Called when the game starts or when spawned
void ABoxRoverPawn::BeginPlay()
{
	Super::BeginPlay();
}

void ABoxRoverPawn::Move(const FInputActionValue& Value)
{
}

void ABoxRoverPawn::Turn(const FInputActionValue& Value)
{
}

// Called every frame
void ABoxRoverPawn::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

// Called to bind functionality to input
void ABoxRoverPawn::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
}
