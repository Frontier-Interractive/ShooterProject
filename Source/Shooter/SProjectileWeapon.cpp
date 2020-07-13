// Fill out your copyright notice in the Description page of Project Settings.


#include "SProjectileWeapon.h"
#include "GLProjectile.h"
#include "Engine/World.h"

void ASProjectileWeapon::StartFire()
{
	Fire();
}

void ASProjectileWeapon::StopFire()
{
	if (Projectile && !Projectile->IsActorBeingDestroyed())
	{
		Projectile->Explosion();
	}
}

void ASProjectileWeapon::AltFire()
{
	AActor* MyOwner = GetOwner();
	
	if (MyOwner)
	{
		if ( RoundsChambered > 0 )
		{
			FVector EyeLocation;
			FRotator EyeRotation;
			MyOwner->GetActorEyesViewPoint(EyeLocation, EyeRotation);

			FVector MuzzleLocation = SkeletalMeshComp->GetSocketLocation(MuzzleSocketName);
			FRotator MuzzleRotation = SkeletalMeshComp->GetSocketRotation(MuzzleSocketName);

			FActorSpawnParameters SpawnParams;
			SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

			Projectile = GetWorld()->SpawnActor<AGLProjectile>(BounceProjectileActor, MuzzleLocation, EyeRotation, SpawnParams);

			--RoundsChambered;
		}else
		{
			Reload();
		}
	}
}

void ASProjectileWeapon::Fire()
{
	AActor* MyOwner = GetOwner();
	
	if (MyOwner)
	{
		if ( RoundsChambered > 0 )
		{
			FVector EyeLocation;
			FRotator EyeRotation;
			MyOwner->GetActorEyesViewPoint(EyeLocation, EyeRotation);

			FVector MuzzleLocation = SkeletalMeshComp->GetSocketLocation(MuzzleSocketName);
			FRotator MuzzleRotation = SkeletalMeshComp->GetSocketRotation(MuzzleSocketName);

			FActorSpawnParameters SpawnParams;
			SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

			Projectile = GetWorld()->SpawnActor<AGLProjectile>(ProjectileActor, MuzzleLocation, EyeRotation, SpawnParams);

			--RoundsChambered;
		}else
		{
			Reload();
		}
	}
	
}
