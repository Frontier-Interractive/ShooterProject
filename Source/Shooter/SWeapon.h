// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SWeapon.generated.h"

class UDamageType;
class UParticleSystem;
class USkeletalMeshComponent;
class UCameraShake;

USTRUCT()
struct FHitScanTrace
{
	GENERATED_BODY()

public:

	UPROPERTY()
	TEnumAsByte<EPhysicalSurface> SurfaceType;

	UPROPERTY()
	FVector_NetQuantize TraceEnd;
};

UCLASS()
class SHOOTER_API ASWeapon : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ASWeapon();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	USkeletalMeshComponent* SkeletalMeshComp;
	
	virtual void Fire();

	UFUNCTION(Server, Reliable, WithValidation)
	void ServerFire();
	
	virtual void ReloadWeapon();
	
public:

	virtual void StartFire();

	virtual void StopFire();
	
	virtual void AltFire() {return;};

	void Reload();

protected:

	void PlayFiringEffects(FVector TracerEndPoint) const;
	
	void PlayImpactEffects(EPhysicalSurface SurfaceType, FVector ImpactPoint) const;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon")
	TSubclassOf<UDamageType> DamageType;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon")
	UParticleSystem *MuzzleEffect;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon")
	UParticleSystem* TracerEffect;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "Weapon")
	FName MuzzleSocketName;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "Weapon")
	FName TracerTargetName;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon")
	UParticleSystem *DefaultImpactEffect;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon")
	UParticleSystem * VulnerableImpactEffect;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon")
	TSubclassOf<UCameraShake> CamShake;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon")
	float BaseDamage;
		
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon")
	float RateOfFire;

	float LastTimeFired;
	
	float TimeBetweenShots;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon")
	int RoundsReserves;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon")
	int RoundsToChamber;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "Weapon")
	int RoundsChambered;

	FTimerHandle TimerHandle_Fire;
	
	FTimerHandle TimerHandle_Reload;

	UPROPERTY(ReplicatedUsing=OnRep_HitScanTrace)
	FHitScanTrace HitScaneTrace;

	UFUNCTION()
	void OnRep_HitScanTrace();

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon")
	bool bHasAltFire;

	bool bIsReloading;

	int GetMaxAmmo() {return RoundsReserves; };
	int GetAmmoRemaining() {return RoundsChambered; };
	
};
