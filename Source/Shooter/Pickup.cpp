// Fill out your copyright notice in the Description page of Project Settings.


#include "Pickup.h"

// Sets default values
APickup::APickup()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	PickupInterval = 0.0f;
	TotalNumberOfTicks = 0;
}

// Called when the game starts or when spawned
void APickup::BeginPlay()
{
	Super::BeginPlay();
}

void APickup::OnTickPickup()
{
	TicksProcessed++;

	OnPickupTicked();

	if (TicksProcessed >= TotalNumberOfTicks)
	{ 
		OnExpired();
		GetWorldTimerManager().ClearTimer(TimeHandle_Pickup);
	}
}

// Called every frame
void APickup::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void APickup::ActivatePickup()
{
	if (PickupInterval > 0.0f)
		GetWorldTimerManager().SetTimer(TimeHandle_Pickup, this, &APickup::OnTickPickup, PickupInterval, true, 0.0f);
	else
		OnTickPickup();
}

