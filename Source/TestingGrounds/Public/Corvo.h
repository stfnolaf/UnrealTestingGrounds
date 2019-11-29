// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "BlinkAbility.h"
#include "Grapple.h"
#include "CorvoAnimInstance.h"
#include "Runtime/Engine/Classes/Components/CapsuleComponent.h"
#include "Runtime/Engine/Classes/Components/BoxComponent.h"
#include "Runtime/Engine/Classes/GameFramework/SpringArmComponent.h"
#include "Corvo.generated.h"

UCLASS()
class TESTINGGROUNDS_API ACorvo : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	ACorvo();

protected:

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Corvo")
	UStaticMeshComponent* Hand = nullptr;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Corvo")
	UCapsuleComponent* myCapsule;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Corvo")
	UCameraComponent* myCamera = nullptr;

	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	// CONTROL METHODS
	void MoveRight(float Val);
	void MoveForward(float Val);
	void AddYaw(float Val);
	void AddPitch(float Val);
	void OnInitiateAbility();
	void OnReleaseAbility();
	void OnQuit();

	TArray<UGrapple*> abilities;

	int activeAbility = 0;

	int numJumps = 0;
	int maxJumps = 2;

	bool horizontalMovementEnabled = true;

	bool railMovementEnabled = false;

	bool onGround = true;

	FVector railDir = FVector();

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Corvo")
	USpringArmComponent* mySpringArm = nullptr;

	UCorvoAnimInstance* animInst = nullptr;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	UStaticMeshComponent* GetHand();

	UCameraComponent* GetCamera();

	UCapsuleComponent* GetCapsuleComponent();

	virtual void Landed(const FHitResult& hit) override;

	void ResetJumps();

	void MyJump();

	void DisableHorizontalMovement();

	void EnableHorizontalMovement();

	void LockRailMovement();

	void UnlockRailMovement();

	void SetRailDir(FVector vect);

	bool IsOnGround();
};
