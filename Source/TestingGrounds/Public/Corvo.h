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

	UPROPERTY(VisibleDefaultsOnly, Category = WallClimbing)
	class USphereComponent* SphereTracer;

	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	// CONTROL METHODS
	void MoveRight(float Val);
	void MoveForward(float Val);
	void AddYaw(float Val);
	void AddPitch(float Val);
	void OnInitiateAbility();
	void OnReleaseAbility();
	void OnInitiateAttack();
	void OnReleaseAttack();
	void OnQuit();

	TArray<UGrapple*> abilities;

	int activeAbility = 0;

	int numJumps = 0;
	int maxJumps = 2;

	bool horizontalMovementEnabled = true;

	bool railMovementEnabled = false;

	bool onGround = true;

	// LEDGE TRACER
	bool CanTraceForLedges = false;

	FVector WallTraceImpact = FVector::ZeroVector;

	FVector WallNormal = FVector::ZeroVector;

	FVector LedgeHeight = FVector::ZeroVector;

	bool IsClimbingLedge = false;

	bool CanGrabLedge = false;

	bool JustLetGo = false;

	FTimerHandle LedgeGrabDelayHandle;

	UFUNCTION()
	void OnLedgeTracerOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	void OnLedgeTracerOverlapEnd(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	FVector railDir = FVector();

	UCorvoAnimInstance* animInst = nullptr;

	UFUNCTION(BlueprintCallable)
	void SpawnKnife();

	UPROPERTY(EditAnywhere)
	TSubclassOf<class AKnife> knifeClass;

	UPROPERTY(EditAnywhere)
	TSubclassOf<class UCameraShake> CatchCameraShake;

	AKnife* knife;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Corvo)
	bool knifeThrown = true;

	FTimerHandle knifeWaitHandle;

	UFUNCTION(BlueprintCallable, Category = KnifeThrowing)
	void ThrowKnife();

	UFUNCTION(BlueprintCallable, Category = KnifeThrowing)
	void RecallKnife();

	UInputComponent* PlayerInputComponent;

	void GrappleToLocation(FVector loc);

	void ToggleTime();

	void CrouchAction();

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	USkeletalMeshComponent* GetMyMesh();

	UCameraComponent* GetCamera();

	virtual void Landed(const FHitResult& hit) override;

	void ResetJumps();

	void SpacebarAction();

	void LetGoOfLedge();

	UFUNCTION()
	void ResetJustLetGo();

	void GrabLedge();

	void DisableHorizontalMovement();

	void EnableHorizontalMovement();

	void LockRailMovement();

	void UnlockRailMovement();

	void SetRailDir(FVector vect);

	bool IsOnGround();

	void UpdateDeltaYawBetweenPlayerAndKnife();

	UFUNCTION()
	void EndWaitForKnife();

	float GetForwardMovement();

	UWallRunning* GetWallRunningComponent();
};
