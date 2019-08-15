// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Runtime/Engine/Classes/Components/TimelineComponent.h"
#include "Runtime/Engine/Classes/Curves/CurveFloat.h"
#include "Runtime/Engine/Classes/Materials/Material.h"
#include "Runtime/Engine/Classes/Camera/CameraComponent.h"
#include "Corvo.generated.h"

UCLASS()
class TESTINGGROUNDS_API ACorvo : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	ACorvo();

	// HAND ANIMATION VARIABLES
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Corvo")
	UStaticMeshComponent* Hand = nullptr;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Corvo")
	UCapsuleComponent* capsule = nullptr;
	UPROPERTY(EditAnywhere, Category = "Corvo")
	UCurveFloat* handBounceCurve;
	UPROPERTY(EditAnywhere, Category = "Corvo")
	float handZOffset;
	FOnTimelineFloat HandInterpFunction{};
	FOnTimelineEvent HandTimelineFinished{};
	UFUNCTION()
	void HandTimelineFloatReturn(float value);
	UFUNCTION()
	void OnHandTimelineFinished();

	// CAMERA ANIMATION VARIABLES
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Corvo")
	UCameraComponent* corvoCam;
	UPROPERTY(EditAnywhere, Category = "Corvo")
	UCurveFloat* cameraFOVCurve;
	FOnTimelineFloat CamFOVInterpFunction{};
	UFUNCTION()
	void CamTimelineFloatReturn(float value);

	// HAND MATERIALS
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Corvo")
	UMaterial* white;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Corvo")
	UMaterial* blue;

	// TELEPORT FUNCTION
	UFUNCTION()
	void SetNewLoc(FVector vect);

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Corvo")
	UStaticMeshComponent* teleportIndicator = nullptr;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Corvo")
	UStaticMeshComponent* climbIndicator = nullptr;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Corvo")
	float maxTeleportDistance;


private:

	// ANIMATION TIMELINES
	UTimelineComponent* handBounceTimeline;
	UTimelineComponent* teleportFOVTimeline;

	// HAND POSITIONS
	FVector handStartPos;
	FVector handOffsetPos;

	// CAMERA FOVs
	float cameraDefaultFOV;
	float cameraTeleportFOV;

	// TELEPORT DELAY TIMER HANDLE
	FTimerHandle teleportDelayHandle;

	bool teleportIndicatorActive = false;

	bool HandleWallClimbing(FHitResult hit, bool isTeleporting);

	float collisionCheckRadius = 34.0f;
	float collisionCheckHalfHeight = 88.0f;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	// CONTROL METHODS
	void MoveRight(float Val);
	void MoveForward(float Val);
	void OnInitiateTeleport();
	void OnTeleport();
	void Teleport(FVector teleportLoc);

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

};
