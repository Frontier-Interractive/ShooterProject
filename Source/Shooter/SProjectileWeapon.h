// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "SWeapon.h"
#include "SProjectileWeapon.generated.h"

class AGLProjectile;
/**
 * 
 */
UCLASS()
class SHOOTER_API ASProjectileWeapon : public ASWeapon
{
	GENERATED_BODY()
public:
	virtual void StartFire() override;
	
	virtual void StopFire() override;

	virtual void AltFire() override;
	
protected:

	virtual void Fire() override;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon")
	TSubclassOf<AGLProjectile> ProjectileActor;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "Weapon")
	AGLProjectile * Projectile;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon")
	TSubclassOf<AGLProjectile> BounceProjectileActor;

};
