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

	// Collision Sphere is the root object
	CollisionSphere = CreateDefaultSubobject<USphereComponent>(TEXT("Collision"));
	SetRootComponent(CollisionSphere);
	CollisionSphere->InitSphereRadius(50.f);

	// attach the VisualMesh to the sphere
	VisualMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("VisualMesh"));
	VisualMesh->SetupAttachment(CollisionSphere);

	// attach the SpringArm to the sphere and set position settings
	SpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArm"));
	SpringArm->SetupAttachment(CollisionSphere);
	SpringArm->TargetArmLength = 700.f;
	SpringArm->SetRelativeRotation(FRotator(-55.0f, 0.0f, 0.0f));
	SpringArm->bDoCollisionTest = false;
	SpringArm->bInheritPitch = false;
	SpringArm->bInheritRoll = false;
	SpringArm->bInheritYaw = false;

	// attach the camera to the SpringArm
	Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	Camera->SetupAttachment(SpringArm);

	AutoPossessPlayer = EAutoReceiveInput::Player0;
}

// Called when the game starts or when spawned
void ABoxRoverPawn::BeginPlay()
{
	Super::BeginPlay();

	//if (APlayerController* PlayerController = Cast<APlayerController>(GetWorld()->GetFirstPlayerController()))

	// should set up PlayerController and Input mappings
	if (APlayerController* PlayerController = Cast<APlayerController>(GetController()))
	{
		if (ULocalPlayer* LocalPlayer = PlayerController->GetLocalPlayer())
		{
			if (UEnhancedInputLocalPlayerSubsystem* InputSubsystem = LocalPlayer->GetSubsystem<
				UEnhancedInputLocalPlayerSubsystem>())
			{
				if (DefaultMappingContext)
				{
					InputSubsystem->AddMappingContext(DefaultMappingContext, 0);
				}
			}
		}
	}
}

// Called every frame
void ABoxRoverPawn::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// Update/check movement/turning
	if (!FMath::IsNearlyZero(TurnInput))
	{
		AddActorLocalRotation(FRotator(0.0f, TurnInput * TurnSpeed * DeltaTime, 0.0f));
	}

	if (!FMath::IsNearlyZero(MoveInput))
	{
		const FVector Delta = GetActorForwardVector() * MoveInput * MoveSpeed * DeltaTime;
		AddActorWorldOffset(Delta, true);
	}
}

// Called to bind functionality to input
void ABoxRoverPawn::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	// should bind actions
	if (UEnhancedInputComponent* EnhancedInput = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		if (MoveAction)
		{
			EnhancedInput->BindAction(MoveAction, ETriggerEvent::Triggered, this, &ABoxRoverPawn::Move);
			EnhancedInput->BindAction(MoveAction, ETriggerEvent::Completed, this, &ABoxRoverPawn::Move);
		}

		if (TurnAction)
		{
			EnhancedInput->BindAction(TurnAction, ETriggerEvent::Triggered, this, &ABoxRoverPawn::Turn);
			EnhancedInput->BindAction(TurnAction, ETriggerEvent::Completed, this, &ABoxRoverPawn::Turn);
		}
	}
}

void ABoxRoverPawn::Move(const FInputActionValue& Value)
{
	MoveInput = Value.Get<float>();
}

void ABoxRoverPawn::Turn(const FInputActionValue& Value)
{
	TurnInput = Value.Get<float>();
}
