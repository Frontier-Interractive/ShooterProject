// Fill out your copyright notice in the Description page of Project Settings.


#include "GLProjectile.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"
#include "TimerManager.h"
#include "Engine/Engine.h"

// Sets default values
AGLProjectile::AGLProjectile():
BaseDamage(200),
MinimumDamage(50),
DamageFalloff(0.5f)
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void AGLProjectile::BeginPlay()
{
	Super::BeginPlay();
	//GetWorldTimerManager().SetTimer(TimerHandle_Explosion, this, &AGLProjectile::Explosion, 0.5f, false);
}

void AGLProjectile::Explosion()
{
	if (this)
	{
		TArray<AActor *> IgnoreActors;
		//IgnoreActors.Add(GetOwner()->GetOwner());
		UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), ExplosionEffect, GetActorLocation());
		UGameplayStatics::ApplyRadialDamageWithFalloff(GetWorld(), BaseDamage, MinimumDamage, GetActorLocation(), MinimumDamage*2, BaseDamage*2, DamageFalloff, DamageType, IgnoreActors , this, GetInstigatorController());
		DrawDebugSphere(GetWorld(), GetActorLocation(), MinimumDamage*2, 12, FColor::Yellow, false, 1, 0, 1);
		DrawDebugSphere(GetWorld(), GetActorLocation(), BaseDamage*2, 12, FColor::Red, false, 1, 0, 1);
	}
	
	Destroy();
	GEngine->ForceGarbageCollection(true);
}

// Called every frame
void AGLProjectile::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

