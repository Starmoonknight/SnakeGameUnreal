// Fill out your copyright notice in the Description page of Project Settings.


#include "FoodActor.h"
#include "SnakeGridwalkerPawn.h"

#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"

// Sets default values
AFoodActor::AFoodActor()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	CollisionSphere = CreateDefaultSubobject<USphereComponent>(TEXT("CollisionSphere"));
	SetRootComponent(CollisionSphere);
	CollisionSphere->SetSphereRadius(CollisionRadius);
	CollisionSphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	CollisionSphere->SetCollisionObjectType(ECC_WorldDynamic);
	CollisionSphere->SetCollisionResponseToAllChannels(ECR_Ignore);
	CollisionSphere->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	CollisionSphere->SetGenerateOverlapEvents(true);

	FoodMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("FoodStaticMesh"));
	FoodMesh->SetupAttachment(CollisionSphere);
	FoodMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	FoodMesh->SetSimulatePhysics(false);
}


// Called when the game starts or when spawned
void AFoodActor::BeginPlay()
{
	Super::BeginPlay();

	CollisionSphere->OnComponentBeginOverlap.AddDynamic(this, &AFoodActor::HandleFoodOverlap);
}

// Called every frame
void AFoodActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

float AFoodActor::GetPlacementHalfHeight() const
{
	if (!CollisionSphere)
	{
		return 0.0f;
	}

	return CollisionSphere->GetScaledSphereRadius();
}

void AFoodActor::SetFoodGridPosition(const FIntPoint& NewGridPosition, const FVector& NewWorldLocation)
{
	FoodGridPosition = NewGridPosition;
	SetActorLocation(NewWorldLocation);
}

void AFoodActor::SetFoodValues(const int32 NewScoreValue, const int32 NewGrowthValue)
{
	ScoreValue = FMath::Max(0, NewScoreValue);
	GrowthValue = FMath::Max(0, NewGrowthValue);
}

void AFoodActor::SetActiveStatus(bool bShouldBeActive, bool bShouldBroadcastChange)
{
	if (bIsActive == bShouldBeActive)
	{
		return;
	}
	// call inactive / activate
	bShouldBeActive ? ActivateFood() : InactivateFood();

	// should it broadcast anything by events, need other event for this /or way to give player actor ref ?  
	// if (bShouldBroadcastChange)
	// {
	//     OnFoodActiveStateChanged.Broadcast(this, bIsActive);
	// }
}

void AFoodActor::HandleFoodOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
                                   UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep,
                                   const FHitResult& SweepResult)
{
	//if (!bFromSweep)  // snake currently teleports, not by swept movement 
	if (!bIsActive)
	{
		return;
	}

	ASnakeGridwalkerPawn* SnakePawn = Cast<ASnakeGridwalkerPawn>(OtherActor);
	if (!SnakePawn)
	{
		return;
	}

	SnakePawn->RequestGrowth(GrowthValue);
	ConsumeBy(SnakePawn);
}

void AFoodActor::ConsumeBy(AActor* ConsumerActor)
{
	if (!bIsActive)
	{
		return;
	}

	InactivateFood(); // later, remove Destroy()
	OnFruitConsumed.Broadcast(this, ConsumerActor);
	Destroy(); // pooling does not exist yet, maybe later make DestroyFood() to handle end of life. 
}

void AFoodActor::InactivateFood()
{
	bIsActive = false;

	SetActorHiddenInGame(true);
	CollisionSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void AFoodActor::ActivateFood()
{
	bIsActive = true;

	SetActorHiddenInGame(false);
	CollisionSphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
}

/*
void AFoodActor::DestroyFood()
{
	// custom bookkeeping event / unregister from manager / remove from arrays
	OnFoodAboutToBeDestroyed.Broadcast(this);

	Destroy();
}
*/
