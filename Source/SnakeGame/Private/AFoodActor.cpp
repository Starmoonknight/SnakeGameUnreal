// Fill out your copyright notice in the Description page of Project Settings.


#include "AFoodActor.h"
#include "ASnakeGridwalkerPawn.h"

#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"

// Sets default values
AAFoodActor::AAFoodActor()
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
void AAFoodActor::BeginPlay()
{
	Super::BeginPlay();

	CollisionSphere->OnComponentBeginOverlap.AddDynamic(this, &AAFoodActor::HandleFoodOverlap);
}

// Called every frame
void AAFoodActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

float AAFoodActor::GetPlacementHalfHeight() const
{
	if (!CollisionSphere)
	{
		return 0.0f;
	}

	return CollisionSphere->GetScaledSphereRadius();
}

void AAFoodActor::SetFoodGridPosition(const FIntPoint& NewGridPosition, const FVector& NewWorldLocation)
{
	FoodGridPosition = NewGridPosition;
	SetActorLocation(NewWorldLocation);
}

void AAFoodActor::SetFoodValues(const int32 NewScoreValue, const int32 NewGrowthValue)
{
	ScoreValue = FMath::Max(0, NewScoreValue);
	GrowthValue = FMath::Max(0, NewGrowthValue);
}

void AAFoodActor::SetActiveStatus(bool bShouldBeActive, bool bShouldBroadcastChange)
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

void AAFoodActor::HandleFoodOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
                                    UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep,
                                    const FHitResult& SweepResult)
{
	//if (!bFromSweep)  // snake currently teleports, not by swept movement 
	if (!bIsActive)
	{
		return;
	}

	AASnakeGridwalkerPawn* SnakePawn = Cast<AASnakeGridwalkerPawn>(OtherActor);
	if (!SnakePawn)
	{
		return;
	}

	SnakePawn->RequestGrowth(GrowthValue);
	ConsumeBy(SnakePawn);
}

void AAFoodActor::ConsumeBy(AActor* ConsumerActor)
{
	if (!bIsActive)
	{
		return;
	}

	InactivateFood(); // later, remove Destroy()
	OnFruitConsumed.Broadcast(this, ConsumerActor);
	Destroy(); // pooling does not exist yet, maybe later make DestroyFood() to handle end of life. 
}

void AAFoodActor::InactivateFood()
{
	bIsActive = false;

	SetActorHiddenInGame(true);
	CollisionSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void AAFoodActor::ActivateFood()
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
