// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Runtime/Engine/Classes/Components/TimelineComponent.h"
#include "Runtime/Engine/Classes/Curves/CurveFloat.h"
#include "Runtime/Engine/Classes/Materials/Material.h"
#include "Runtime/Engine/Classes/Camera/CameraComponent.h"
#include "BlinkIndicator.h"
#include "BlinkAbility.generated.h"


UCLASS( Blueprintable, ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class TESTINGGROUNDS_API UBlinkAbility : public UActorComponent
{
	GENERATED_BODY()

public:
	// Sets default values for this component's properties
	UBlinkAbility();

	// HAND ANIMATION VARIABLES
	UPROPERTY(EditAnywhere, Category = "Corvo")
	UCurveFloat* handBounceCurve;
	UPROPERTY(EditAnywhere, Category = "Corvo")
	float handZOffset = 5.0f;
	FOnTimelineFloat HandInterpFunction{};
	FOnTimelineEvent HandTimelineFinished{};
	UFUNCTION()
	void HandTimelineFloatReturn(float value);
	UFUNCTION()
	void OnHandTimelineFinished();

	// CAMERA ANIMATION VARIABLES
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
	float maxTeleportDistance = 2700.0f;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Corvo")
	float teleportTime = 0.05f;

	void OnInitiateAbility();
	void OnReleaseAbility();

	UPROPERTY(EditAnywhere, Category = "Corvo")
	TSubclassOf<class ABlinkIndicator> blinkMarker;

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

	void Teleport(FVector teleportLoc);

	ABlinkIndicator* blinkIndicator = nullptr;

	UCameraComponent* corvoCam = nullptr;


protected:
	// Called when the game starts
	virtual void BeginPlay() override;

	UPROPERTY(EditAnywhere, Category = "Corvo")
		float collisionCheckRadius = 34.0f;
	UPROPERTY(EditAnywhere, Category = "Corvo")
		float collisionCheckHalfHeight = 88.0f;

	UPROPERTY(EditAnywhere, Category = "Corvo")
		float sphereTraceRadius = 34.0f;

	UStaticMeshComponent* Hand = nullptr;

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

		
};
