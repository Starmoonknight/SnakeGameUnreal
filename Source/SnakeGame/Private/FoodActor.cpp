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

	bIsActive = false;

	SetActorHiddenInGame(true);
	CollisionSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	OnFruitConsumed.Broadcast(this, ConsumerActor);
}
