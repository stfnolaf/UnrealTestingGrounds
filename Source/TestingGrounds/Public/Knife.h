// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/SceneComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Components/TimelineComponent.h"
#include "Containers/EnumAsByte.h"
#include "Knife.generated.h"

UENUM(BlueprintType)
enum class EKnifeState : uint8
{
	VE_Idle 	UMETA(DisplayName = "Idle"),
	VE_Launched 	UMETA(DisplayName = "Launched"),
	VE_LodgedInSomething	UMETA(DisplayName = "LodgedInSomething"),
	VE_Returning	UMETA(DisplayName = "Returning")
};

UCLASS()
class TESTINGGROUNDS_API AKnife : public AActor
{
	GENERATED_BODY()

protected:

	FRotator CameraStartRotation = FRotator();

	FVector ThrowDirection = FVector();

	FVector CameraLocationAtThrow = FVector();

	float KnifeThrowSpeed = 2500.0f;

	float KnifeSpinAxisOffset = 0.0f;

	float KnifeSpinRate = 2.5f;

	EKnifeState KnifeState = EKnifeState::VE_Idle;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Pivots)
	USceneComponent* PivotPoint;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Movement)
	UProjectileMovementComponent* ProjectileMovementVar;

	// KNIFE ROTATION TIMELINE VARS
	UPROPERTY()
	UTimelineComponent* KnifeRotTimeline = nullptr;

	UPROPERTY(EditAnywhere)
	UCurveFloat* KnifeRotCurve = nullptr;

	UFUNCTION()
	void KnifeRotTimelineCallback(float val);

	UFUNCTION()
	void KnifeRotTimelineFinishedCallback();

	UPROPERTY()
	TEnumAsByte<ETimelineDirection::Type> KnifeRotTimelineDirection;

	// KNIFE THROW TRACE TIMELINE VARS
	UPROPERTY()
	UTimelineComponent* KnifeThrowTraceTimeline = nullptr;

	UPROPERTY(EditAnywhere)
	UCurveFloat* KnifeThrowTraceCurve = nullptr;

	UFUNCTION()
	void KnifeThrowTraceTimelineCallback(float val);

	UFUNCTION()
	void KnifeThrowTraceTimelineFinishedCallback();

	UPROPERTY()
	TEnumAsByte<ETimelineDirection::Type> KnifeThrowTraceTimelineDirection;


	
public:	
	// Sets default values for this actor's properties
	AKnife();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	void SnapKnifeToStartPosition(FRotator StartRotation, FVector ThrowDirectionVector, FVector CameraLocation);

	void LaunchKnife();

	void StartKnifeRotForward();

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	void Throw(FRotator CameraRotation, FVector ThrowDirectionVector, FVector CameraLocation, float ThrowSpeed);

};
