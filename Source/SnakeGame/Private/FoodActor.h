// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "FoodActor.generated.h"

class AFoodActor; // need this since the delegate uses AAFoodActor* before the class? 
class USphereComponent;
class UStaticMeshComponent;
class UPrimitiveComponent;


DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(
	FFoodConsumedSignature,
	AFoodActor*, Food,
	AActor*, ConsumerActor);


UCLASS()
class AFoodActor : public AActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	AFoodActor();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// --- Events --- 
	UPROPERTY(BlueprintAssignable, Category = "Pickups|Food|Events")
	FFoodConsumedSignature OnFruitConsumed;

	// --- Information getters ---
	UFUNCTION(BlueprintPure, Category = "Pickups|Food")
	FIntPoint GetFoodGridPosition() const { return FoodGridPosition; }

	UFUNCTION(BlueprintPure, Category = "Pickups|Food")
	bool IsActive() const { return bIsActive; }

	UFUNCTION(BlueprintPure, Category = "Pickups|Food")
	int32 GetScoreValue() const { return ScoreValue; }

	UFUNCTION(BlueprintPure, Category = "Pickups|Food")
	int32 GetGrowthValue() const { return GrowthValue; }

	UFUNCTION(BlueprintPure, Category = "Pickups|Food")
	float GetPlacementHalfHeight() const;

	// --- Logic setters --- 
	UFUNCTION(BlueprintCallable, Category = "Pickups|Food")
	void SetFoodGridPosition(const FIntPoint& NewGridPosition, const FVector& NewWorldLocation);

	UFUNCTION(BlueprintCallable, Category = "Pickups|Food")
	void SetFoodValues(const int32 NewScoreValue, const int32 NewGrowthValue);

	UFUNCTION(BlueprintCallable, Category = "Pickups|Food")
	void SetActiveStatus(bool bShouldBeActive, bool bShouldBroadcastChange = false);

private:
	UFUNCTION()
	void HandleFoodOverlap(
		UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex,
		bool bFromSweep,
		const FHitResult& SweepResult);

	void ConsumeBy(AActor* ConsumerActor);
	void InactivateFood();
	void ActivateFood();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = true))
	TObjectPtr<UStaticMeshComponent> FoodMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = true))
	TObjectPtr<USphereComponent> CollisionSphere;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = true))
	float CollisionRadius = 45.f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Food", meta = (AllowPrivateAccess = true))
	FIntPoint FoodGridPosition = FIntPoint::ZeroValue;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Food", meta = (AllowPrivateAccess = true))
	int32 ScoreValue = 1;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Food", meta = (AllowPrivateAccess = true))
	int32 GrowthValue = 1;

	bool bIsActive = true;
};
