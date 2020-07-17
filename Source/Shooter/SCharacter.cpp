// Fill out your copyright notice in the Description page of Project Settings.


#include "SCharacter.h"

#include <string>

#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/PawnMovementComponent.h"
#include "SWeapon.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Math/UnrealMathUtility.h"
#include "TimerManager.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "HealthComponent.h"
#include "UnrealNetwork.h"
#include "Components/CapsuleComponent.h"

// Sets default values
ASCharacter::ASCharacter()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	SpringArmComp = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArmComp"));
	SpringArmComp->SetupAttachment(RootComponent);
	SpringArmComp->bUsePawnControlRotation = true;

	CameraComp = CreateDefaultSubobject<UCameraComponent>(TEXT("CameraComp"));
	CameraComp->SetupAttachment(SpringArmComp);

	HealthComp = CreateDefaultSubobject<UHealthComponent>(TEXT("Health Component"));

	GetMovementComponent()->GetNavAgentPropertiesRef().bCanCrouch = true;
	
	ZoomedFOV = 65.0f;

	InterSpeed = 20.0f;

	WeaponAttachSocketName = "WeaponSocket";

	bIsSwappingWeapons = false;

	DashForce = 20000;

	Direction = PlayerDirection::EDefault;

	bIsAlive = true;
}

// Called when the game starts or when spawned
void ASCharacter::BeginPlay()
{
	Super::BeginPlay();
	defaultFOV = CameraComp->FieldOfView;
	
	if (Role == ROLE_Authority)
	{
		SpawnWeapon(StarterWeaponClass);

		HealthComp->OnHealthChanged.AddDynamic(this, &ASCharacter::OnHealthChanged);
	}
}

// Called every frame
void ASCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	
	float TargetFOV = bWantsToZoom ? ZoomedFOV : defaultFOV;

	float NewFOV = FMath::FInterpTo(CameraComp->FieldOfView, TargetFOV, DeltaTime, InterSpeed);

	CameraComp->SetFieldOfView(NewFOV);
	timer.EndTime = GetWorld()->TimeSeconds;

	if (bIsSwappingWeapons && timer.GetCurrentTime() > 0.25f)
	{
		SpawnWeapon(Weapons[CurrentWeaponIndex < 0 ? CurrentWeaponIndex = Weapons.Num()-1 : 
					CurrentWeaponIndex>Weapons.Num()-1 ? CurrentWeaponIndex = 0 : CurrentWeaponIndex]);
	}

	if (CurrentWeapon)
	{
		UpdateHud(HealthComp->GetHealth(), HealthComp->GetDefaultHealth(), 
					CurrentWeapon->GetMaxAmmo(), CurrentWeapon->GetAmmoRemaining());
	}
	
}

void ASCharacter::MoveForward(float value)
{
	Direction = value == 1 ? PlayerDirection::EForward :
	value == -1 ? PlayerDirection::EBack : PlayerDirection::EDefault;
	AddMovementInput(GetActorForwardVector() * (bIsAlive ? value : value = 0));
}

void ASCharacter::MoveRight(float value)
{
	Direction = value == 1 ? PlayerDirection::ERight :
	value == -1 ? PlayerDirection::ELeft :  PlayerDirection::EDefault;
	AddMovementInput(GetActorRightVector() * (bIsAlive ? value : value = 0));
}

void ASCharacter::BeginCrouch()
{
	Crouch();
}

void ASCharacter::EndCrouch()
{
	UnCrouch();
}

void ASCharacter::BeginZoom()
{
	
	if (CurrentWeapon && CurrentWeapon->bHasAltFire)
	{
		BeginAltFire();
		
	}else
	{
		bWantsToZoom = true;
	}
	
	
}

void ASCharacter::EndZoom()
{
	
	if (CurrentWeapon && CurrentWeapon->bHasAltFire )
	{
		EndFire();
		
	}else
	{
		bWantsToZoom = false;
	}
	
}

void ASCharacter::BeginFire()
{
	if (CurrentWeapon && !CurrentWeapon->bIsReloading)
	{
		CurrentWeapon->StartFire();
	}
}

void ASCharacter::BeginAltFire()
{
	if (CurrentWeapon && !CurrentWeapon->bIsReloading)
	{
		CurrentWeapon->AltFire();
	}
}

void ASCharacter::EndFire()
{
	if (CurrentWeapon && !CurrentWeapon->bIsReloading)
	{
		CurrentWeapon->StopFire();
	}
}

void ASCharacter::ReloadWeapon()
{
	if (CurrentWeapon)
	{
		CurrentWeapon->Reload();
	}
	
}

void ASCharacter::SwapWeapon(float value)
{
	if (value && !bIsSwappingWeapons)
	{
		bIsSwappingWeapons = true;
		timer.StartTime = GetWorld()->TimeSeconds;
		CurrentWeaponIndex += value;
	}
}

void ASCharacter::SpawnWeapon(TSubclassOf<ASWeapon> WeaponClass)
{
	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	if (CurrentWeapon)
	{
		CurrentWeapon->Destroy();
	}
	
	CurrentWeapon = GetWorld()->SpawnActor<ASWeapon>(WeaponClass, FVector::ZeroVector, FRotator::ZeroRotator, SpawnParams);

	if (CurrentWeapon)
	{
		CurrentWeapon->SetOwner(this);
		CurrentWeapon->AttachToComponent(GetMesh(), FAttachmentTransformRules::SnapToTargetNotIncludingScale, WeaponAttachSocketName);
		bIsSwappingWeapons = false;
	}
	
}

void ASCharacter::StartDash()
{
	FVector Vel = FVector::ZeroVector;
	
	switch (Direction)
	{
	case EDefault:	
	case EForward:
		Vel = CameraComp->GetForwardVector() * DashForce;
		break;
	case ELeft:
		Vel = (CameraComp->GetRightVector()*-1) * DashForce;
		break;
	case ERight:
		Vel = CameraComp->GetRightVector() * DashForce;
		break;
	default:;
	}

	LaunchCharacter(Vel, false, false);

	GetWorldTimerManager().SetTimer(TimerHandle_Dash, this, &ASCharacter::StopDash, 0.015, false);
}

void ASCharacter::StopDash()
{
	GetCharacterMovement()->StopMovementImmediately();
	GetWorldTimerManager().ClearTimer(TimerHandle_Dash);
}

void ASCharacter::OnHealthChanged(UHealthComponent* HealthComp, float Health, float HealthDelta,
	const UDamageType* DamageType, AController* InstigatedBy, AActor* DamageCauser)
{
	if (Health <= 0.0f && bIsAlive)
	{
		//Death!
		bIsAlive = false;

		if (CurrentWeapon)
		{
			UpdateHud(HealthComp->GetHealth(), HealthComp->GetDefaultHealth(), 
						CurrentWeapon->GetMaxAmmo(), CurrentWeapon->GetAmmoRemaining());
		}
		
		GetMovementComponent()->StopMovementImmediately();
		GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);

		GetWorldTimerManager().SetTimer(TimerHandle_Kill, this, &APawn::DetachFromControllerPendingDestroy, 0.1, false);
		
		SetLifeSpan(3.0f);

	}
}

// Called to bind functionality to input
void ASCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
	PlayerInputComponent->BindAxis("MoveForward", this, &ASCharacter::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &ASCharacter::MoveRight);
	PlayerInputComponent->BindAxis("LookUp", this, &ASCharacter::AddControllerPitchInput);
	PlayerInputComponent->BindAxis("Turn", this, &ASCharacter::AddControllerYawInput);
	PlayerInputComponent->BindAxis("SwapWeapon", this, &ASCharacter::SwapWeapon);

	PlayerInputComponent->BindAction("Crouch", IE_Pressed, this, &ASCharacter::BeginCrouch);
	PlayerInputComponent->BindAction("Crouch", IE_Released, this, &ASCharacter::EndCrouch);

	PlayerInputComponent->BindAction("Zoom", IE_Pressed, this, &ASCharacter::BeginZoom);
	PlayerInputComponent->BindAction("Zoom", IE_Released, this, &ASCharacter::EndZoom);

	PlayerInputComponent->BindAction("Fire", IE_Pressed, this, &ASCharacter::BeginFire);
	PlayerInputComponent->BindAction("Fire", IE_Released, this, &ASCharacter::EndFire);
	
	PlayerInputComponent->BindAction("Reload", IE_Pressed, this, &ASCharacter::ReloadWeapon);

	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &ASCharacter::Jump);
	
	PlayerInputComponent->BindAction("ForwardDash", IE_Pressed, this, &ASCharacter::StartDash);
	

}

FVector ASCharacter::GetPawnViewLocation() const
{
	if (CameraComp)
	{
		return CameraComp->GetComponentLocation();
	}

	return Super::GetPawnViewLocation();
}

void ASCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ASCharacter, CurrentWeapon);
	DOREPLIFETIME(ASCharacter, bIsAlive);
	
}