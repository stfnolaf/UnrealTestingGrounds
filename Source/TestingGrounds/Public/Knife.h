// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/SceneComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Components/TimelineComponent.h"
#include "Containers/EnumAsByte.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Kismet/KismetMathLibrary.h"
#include "Engine/World.h"
#include "PhysicalMaterials/PhysicalMaterial.h"
#include "Kismet/GameplayStatics.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SplineComponent.h"
#include "WeaponReturnPath.h"
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

	class ACorvo* Owner = nullptr;

	FRotator CameraStartRotation = FRotator::ZeroRotator;

	FVector ThrowDirection = FVector::ZeroVector;

	FVector CameraLocationAtThrow = FVector::ZeroVector;

	float KnifeThrowSpeed = 2500.0f;

	float KnifeSpinAxisOffset = 0.0f;

	float KnifeSpinRate = 2.5f;

	float KnifeThrowTraceDistance = 9.0f;

	float MaxCalculationDistance = 3000.0f;

	float DistanceFromCharacter = 0.0f;

	FVector ImpactLocation = FVector::ZeroVector;

	FVector ImpactNormal = FVector::ZeroVector;

	FVector InitialLocation = FVector::ZeroVector;

	FRotator InitialRotation = FRotator::ZeroRotator;

	FName HitBoneName = FName("");

	EPhysicalSurface SurfaceType = EPhysicalSurface::SurfaceType1;

	EKnifeState KnifeState = EKnifeState::VE_Idle;

	float ZAdjustment = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Pivots)
	USceneComponent* PivotPoint;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Pivots)
	USceneComponent* LodgePoint;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Movement)
	UProjectileMovementComponent* ProjectileMovementVar;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Mesh)
	UStaticMeshComponent* KnifeMeshVar;

	UPROPERTY(EditAnywhere)
	TSubclassOf<class AActor> ReturnPathClass;

	AWeaponReturnPath* ReturnPath = nullptr;

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

	void LodgeKnife();

	void StopKnifeMoving();

	float AdjustKnifeImpactPitch(float InclinedSurfaceRange, float RegularSurfaceRange);

	FVector AdjustKnifeImpactLocation();

	float GetClampedKnifeDistanceFromCharacter(float maxDist);

	void AdjustKnifeReturnLocation();

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	bool InitializeOwner(AActor* actor);

	void Throw(FRotator CameraRotation, FVector ThrowDirectionVector, FVector CameraLocation, float ThrowSpeed);

	void Recall();

};
