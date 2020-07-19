// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Pickup.generated.h"

UCLASS()
class SHOOTER_API APickup : public AActor
{
	GENERATED_BODY()
	
public:	
	APickup();

protected:
	virtual void BeginPlay() override;

	/* Time between pickup ticks*/
	UPROPERTY(EditDefaultsOnly, Category="Pickups")
	float PickupInterval;

	/* Total times that the pickup can be applied. */
	UPROPERTY(EditDefaultsOnly, Category = "Pickups")
	int32 TotalNumberOfTicks;

	int32 TicksProcessed;

	/* Handles the tick for the pickup, allows to start and stop. */
	FTimerHandle TimeHandle_Pickup;

	void OnTickPickup();

public:	
	virtual void Tick(float DeltaTime) override;

	void ActivatePickup();

	//These functions are blueprint implementable which means that they can be fully implemented in blueprints and there is no base implementation in C++.
	UFUNCTION(BlueprintImplementableEvent, Category="Pickups")
	void OnActivated();

	UFUNCTION(BlueprintImplementableEvent, Category = "Pickups")
	void OnPickupTicked();

	UFUNCTION(BlueprintImplementableEvent, Category="Pickups")
	void OnExpired();
};
