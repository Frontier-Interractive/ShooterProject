// Fill out your copyright notice in the Description page of Project Settings.


#include "SWeapon.h"
#include "Components/SkeletalMeshComponent.h"
#include "DrawDebugHelpers.h"
#include "Kismet/GameplayStatics.h"
#include "Particles/ParticleSystem.h"
#include "Particles/ParticleSystemComponent.h"
#include "Camera/CameraShake.h"
#include "PhysicalMaterials/PhysicalMaterial.h"
#include "Shooter.h"
#include "TimerManager.h"

// Sets default values
ASWeapon::ASWeapon()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	SkeletalMeshComp = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("Mesh Comp"));

	MuzzleSocketName = "MuzzleSocket";
	TracerTargetName = "BeamEnd";

	BaseDamage = 25.0f;
	RateOfFire = 600.0f;

	RoundsReserves = 500;
	RoundsToChamber = 42;

	bHasAltFire = false;

	bIsReloading = false;
}

// Called when the game starts or when spawned
void ASWeapon::BeginPlay()
{
	Super::BeginPlay();

	TimeBetweenShots = 60 / RateOfFire;
	RoundsChambered = RoundsToChamber;
}

void ASWeapon::Fire()
{
	AActor* MyOwner = GetOwner();

	if (MyOwner)
	{
		if ( RoundsChambered > 0 )
		{
			FVector EyeLocation;
			FRotator EyeRotation;
			MyOwner->GetActorEyesViewPoint(EyeLocation, EyeRotation);

			FVector ShotDirection = EyeRotation.Vector();
			FVector TraceEnd = EyeLocation + (EyeRotation.Vector() * 10000);

			FCollisionQueryParams QuerryParams;
			QuerryParams.AddIgnoredActor(MyOwner);
			QuerryParams.AddIgnoredActor(this);
			QuerryParams.bTraceComplex = true;
			QuerryParams.bReturnPhysicalMaterial = true;

			FVector TracerEndPoint = TraceEnd;

			FHitResult Hit;
			if (GetWorld()->LineTraceSingleByChannel(Hit, EyeLocation, TraceEnd, COLLISION_WEAPON, QuerryParams))
			{
				//Hit
				AActor* HitActor = Hit.GetActor();

				EPhysicalSurface SurfaceType = UPhysicalMaterial::DetermineSurfaceType(Hit.PhysMaterial.Get());

				UParticleSystem * SelectedEffect = nullptr;

				float Damage = BaseDamage;
				
				switch (SurfaceType)
				{
		
				case SURFACE_FLESHDEFAULT:
					SelectedEffect = DefaultImpactEffect;
					break;
					
				case SURFACE_FLESHVULNERABLE:
					SelectedEffect = VulnerableImpactEffect;
					Damage *= 2;
					break;
					
				default:
					SelectedEffect = DefaultImpactEffect;
				}

				if (SelectedEffect)
				{
					UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), SelectedEffect, Hit.ImpactPoint, Hit.ImpactNormal.Rotation());
				}
				
				UGameplayStatics::ApplyPointDamage(HitActor, Damage, ShotDirection, Hit, MyOwner->GetInstigatorController(), this, DamageType);
				TracerEndPoint = Hit.ImpactPoint;
			}
			
		

			//DrawDebugLine(GetWorld(), EyeLocation, TraceEnd, FColor::Red, false, 1, 0, 1);
			PlayFiringEffects(TracerEndPoint);

			LastTimeFired = GetWorld()->TimeSeconds;
			--RoundsChambered;
		}else
		{
			Reload();
		}

		
	}
}

void ASWeapon::ReloadWeapon()
{
	if (RoundsReserves > RoundsToChamber)
	{

		RoundsReserves -= RoundsToChamber;
		RoundsChambered = RoundsToChamber;
		
	}else if (RoundsReserves > 0)
	{
		RoundsChambered = RoundsReserves;
		RoundsReserves = 0;
	}

	bIsReloading = false;
}

void ASWeapon::StartFire()
{
	const float FirstDelay = FMath::Max(LastTimeFired + TimeBetweenShots - GetWorld()->TimeSeconds, 0.0f);

	GetWorldTimerManager().SetTimer(TimerHandle_Fire, this, &ASWeapon::Fire,  TimeBetweenShots, true, FirstDelay);
}

void ASWeapon::StopFire()
{
	GetWorldTimerManager().ClearTimer(TimerHandle_Fire);
}

void ASWeapon::Reload()
{
	if (!bIsReloading)
	{
		GetWorldTimerManager().SetTimer(TimerHandle_Reload, this, &ASWeapon::ReloadWeapon, 1.5f, false);
		bIsReloading = true;
	}
	
}

void ASWeapon::PlayFiringEffects(FVector TracerEndPoint) const
{

	if (MuzzleEffect)
	{
		UGameplayStatics::SpawnEmitterAttached(MuzzleEffect, SkeletalMeshComp, MuzzleSocketName);
	}

	FVector MuzzleLocation = SkeletalMeshComp->GetSocketLocation(MuzzleSocketName);

	if (TracerEffect)
	{
		UParticleSystemComponent* TracerComp = UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), TracerEffect, MuzzleLocation);

		if (TracerComp)
		{
			TracerComp->SetVectorParameter(TracerTargetName, TracerEndPoint);
		}

	}

	if (APawn * MyPawn = static_cast<APawn*>(GetOwner()))
	{
		if (APlayerController * PlayerController = static_cast<APlayerController*>(MyPawn->GetController()))
		{
			PlayerController->ClientPlayCameraShake(CamShake);
		}
	}
}

// Called every frame
void ASWeapon::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

