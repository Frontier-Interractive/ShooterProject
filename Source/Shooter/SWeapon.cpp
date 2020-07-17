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
#include "UnrealNetwork.h"

// Sets default values
ASWeapon::ASWeapon()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	SkeletalMeshComp = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("Mesh Comp"));

	SetReplicates(true);

	NetUpdateFrequency = 66.0f;
	MinNetUpdateFrequency = 33.0f;

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
	if (Role < ROLE_Authority)
	{
		ServerFire();
	}
	
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
			EPhysicalSurface SurfaceType = EPhysicalSurface::SurfaceType_Default;
			
			FHitResult Hit;
			if (GetWorld()->LineTraceSingleByChannel(Hit, EyeLocation, TraceEnd, COLLISION_WEAPON, QuerryParams))
			{
				//Hit
				AActor* HitActor = Hit.GetActor();

				EPhysicalSurface SurfaceType = UPhysicalMaterial::DetermineSurfaceType(Hit.PhysMaterial.Get());

				float Damage = BaseDamage;
				
				if (SurfaceType == SURFACE_FLESHVULNERABLE)
				{
					Damage *=4; 
				}
				
				UGameplayStatics::ApplyPointDamage(HitActor, Damage, ShotDirection, Hit, 
						MyOwner->GetInstigatorController(), this, DamageType);
				
				TracerEndPoint = Hit.ImpactPoint;
				HitScaneTrace.SurfaceType = SurfaceType;

				PlayImpactEffects(SurfaceType, Hit.ImpactPoint);
			}
			
			//DrawDebugLine(GetWorld(), EyeLocation, TraceEnd, FColor::Red, false, 1, 0, 1);
			PlayFiringEffects(TracerEndPoint);

			if (Role == ROLE_Authority)
			{
				HitScaneTrace.TraceEnd = TracerEndPoint;
				HitScaneTrace.SurfaceType = SurfaceType;
			}
			
			LastTimeFired = GetWorld()->TimeSeconds;
			--RoundsChambered;

		}else
		{
			Reload();
		}

		
	}
}

void ASWeapon::ServerFire_Implementation()
{
	Fire();
}

bool ASWeapon::ServerFire_Validate()
{
	return true;
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

void ASWeapon::PlayImpactEffects(EPhysicalSurface SurfaceType, FVector ImpactPoint) const
{
	
	UParticleSystem * SelectedEffect = nullptr;
	
	switch (SurfaceType)
	{

	case SURFACE_FLESHDEFAULT:
		SelectedEffect = DefaultImpactEffect;
		break;
		
	case SURFACE_FLESHVULNERABLE:
		SelectedEffect = VulnerableImpactEffect;
		break;
		
	default:
		SelectedEffect = DefaultImpactEffect;
	}

	if (SelectedEffect)
	{
		FVector ShotDir = ImpactPoint - SkeletalMeshComp->GetSocketLocation(MuzzleSocketName) ;
		ShotDir.Normalize();
		
		UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), SelectedEffect, ImpactPoint, ShotDir.Rotation());
	}
				
}

void ASWeapon::OnRep_HitScanTrace()
{
	//playFX
	PlayFiringEffects(HitScaneTrace.TraceEnd);

	PlayImpactEffects(HitScaneTrace.SurfaceType, HitScaneTrace.TraceEnd);
}

// Called every frame
void ASWeapon::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}


void ASWeapon::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION(ASWeapon, HitScaneTrace, COND_SkipOwner);
	
}
