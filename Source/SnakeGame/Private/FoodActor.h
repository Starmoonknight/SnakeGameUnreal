// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "FoodActor.generated.h"

class USphereComponent;
class UStaticMeshComponent;

UCLASS()
class AFoodActor : public AActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	AFoodActor();

	UFUNCTION(BlueprintCallable, Category = "Pickups|Food")
	void SetFoodGridPosition(const FIntPoint& NewGridPosition, const FVector& NewWorldLocation);

	FIntPoint GetFoodGridPosition() const { return FoodGridPosition; }

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

private:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = true))
	TObjectPtr<USphereComponent> CollisionSphere;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = true))
	TObjectPtr<UStaticMeshComponent> FoodMesh;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = true))
	float CollisionRadius = 45.f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Food", meta = (AllowPrivateAccess = true))
	FIntPoint FoodGridPosition = FIntPoint::ZeroValue;

	bool bIsActive = true;
};
