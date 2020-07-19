// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PickupManager.generated.h"

UCLASS()
class SHOOTER_API APickupManager : public AActor
{
	GENERATED_BODY()
	
public:	
	APickupManager();

protected:
	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere, Category="Components")
	class USphereComponent* SphereComp;

	UPROPERTY(VisibleAnywhere, Category="Components")
	class UDecalComponent* DecalComp;

public:	
	virtual void Tick(float DeltaTime) override;
	virtual void NotifyActorBeginOverlap(AActor* OtherActer) override;
};
