// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "GameFramework/Character.h"
#include "SCharacter.generated.h"

class UCameraComponent;
class USpringArmComponent;
class ASWeapon;

struct Timer
{
	float StartTime = 0;

	float EndTime = 0;

	float GetCurrentTime() const
	{
		return EndTime - StartTime;
	}
};

enum PlayerDirection
{
	EForward = 0,
	ELeft,
	ERight,
	EBack,
	EDefault
};

UCLASS()
class SHOOTER_API ASCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	ASCharacter();
	
	
protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	void MoveForward(float value);
	void MoveRight(float value);

	void BeginCrouch();
	void EndCrouch();

	void BeginZoom();
	void EndZoom();

	void BeginFire();
	void EndFire();

	void ReloadWeapon();

	void SwapWeapon(float value);

	void SpawnWeapon(TSubclassOf<ASWeapon> WeaponClass);
	
	void StartDash();
	
	void StopDash();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UCameraComponent* CameraComp;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	USpringArmComponent* SpringArmComp;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	bool bWantsToZoom;

	UPROPERTY(EditDefaultsOnly, Category = "Player")
	float ZoomedFOV;

	float defaultFOV;

	UPROPERTY(EditDefaultsOnly, Category = "Player", meta = (ClampMin = 0.1, ClampMax = 100))
	float InterSpeed;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon")
	TArray<TSubclassOf<ASWeapon>> Weapons;

	UPROPERTY(EditDefaultsOnly, Category = "Player")
	TSubclassOf<ASWeapon> StarterWeaponClass;

	UPROPERTY(VisibleDefaultsOnly, Category = "Player")
	FName WeaponAttachSocketName;

	int CurrentWeaponIndex;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Weapon")
	ASWeapon* CurrentWeapon;

	bool bIsSwappingWeapons;

	FTimerHandle TimerHandle_Dash;

	UPROPERTY(EditDefaultsOnly, Category = "Player")
	float DashForce;

	PlayerDirection Direction;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	

	virtual FVector GetPawnViewLocation() const override;

private:

	Timer timer;

};


