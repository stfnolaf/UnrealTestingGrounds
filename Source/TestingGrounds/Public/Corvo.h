// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "BlinkAbility.h"
#include "Grapple.h"
#include "CorvoAnimInstance.h"
#include "Knife.h"
#include "Runtime/Engine/Classes/Components/CapsuleComponent.h"
#include "Runtime/Engine/Classes/Components/BoxComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Corvo.generated.h"

UCLASS()
class TESTINGGROUNDS_API ACorvo : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	ACorvo();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	// COMPONENTS
	UPROPERTY(VisibleDefaultsOnly)
	class USpringArmComponent* SpringArm;
	UPROPERTY(VisibleDefaultsOnly)
	USpringArmComponent* ArmController;
	UPROPERTY(VisibleDefaultsOnly, Category = Camera)
	UCameraComponent* PlayerCamera;
	UPROPERTY(VisibleDefaultsOnly, Category = Mesh)
	USkeletalMeshComponent* ArmsMesh;
	UPROPERTY(VisibleDefaultsOnly, Category = WallRunning)
	class UWallRunning* WallRunningComponent;
	//UPROPERTY(VisibleDefaultsOnly, Category = WallClimbing)
	//class USphereComponent* SphereTracer;
	//UPROPERTY(VisibleDefaultsOnly, Category = WallClimbing)
	//class UClimbing* ClimbingComponent;

	// CONTROL SYSTEM
	void MoveRight(float Val);
	void MoveForward(float Val);
	void AddYaw(float Val);
	void AddPitch(float Val);
	void OnInitiateAbility();
	void OnReleaseAbility();
	void OnInitiateAttack();
	void OnReleaseAttack();
	void OnQuit();
	void ToggleTime();
	void CrouchAction();
	void Sprint();
	void Walk();
	UInputComponent* PlayerInputComponent;
	bool CanSprint();

	// JUMPING
	int numJumps = 0;
	int maxJumps = 2;
	bool onGround = true;

	// RAIL MOVEMENT SYSTEM (used for wallrunning)
	bool horizontalMovementEnabled = true;
	bool railMovementEnabled = false;
	FVector railDir = FVector();

	// LEDGE TRACER OVERLAP
	/*UFUNCTION()
	void OnLedgeTracerOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
	UFUNCTION()
	void OnLedgeTracerOverlapEnd(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);*/

	// ANIMATION INSTANCE
	UCorvoAnimInstance* animInst = nullptr;

	// KNIFE VARIABLES
	AKnife* knife;
	UFUNCTION(BlueprintCallable)
	void SpawnKnife();
	UPROPERTY(EditAnywhere)
	TSubclassOf<class AKnife> knifeClass;
	UPROPERTY(EditAnywhere)
	TSubclassOf<class UCameraShake> CatchCameraShake;
	FTimerHandle knifeWaitHandle;
	UFUNCTION(BlueprintCallable, Category = KnifeThrowing)
	void ThrowKnife();
	UFUNCTION(BlueprintCallable, Category = KnifeThrowing)
	void RecallKnife();
	void GrappleToLocation(FVector loc);

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	// ACCESS METHODS
	USkeletalMeshComponent* GetMyMesh();
	UCameraComponent* GetCamera();
	UWallRunning* GetWallRunningComponent();
	float GetForwardMovement();
	float GetRightMovement();

	// RAIL MOVEMENT SYSTEM
	void LockRailMovement();
	void UnlockRailMovement();
	void DisableHorizontalMovement();
	void EnableHorizontalMovement();
	void SetRailDir(FVector vect);

	// KNIFE
	UFUNCTION()
	void EndWaitForKnife();
	void UpdateDeltaYawBetweenPlayerAndKnife();

	// JUMPING
	void SpacebarAction();
	virtual void Landed(const FHitResult& hit) override;
	void ResetJumps();
	bool IsOnGround();

	// ANIMATION CALLS
	void AnimHang();

};
