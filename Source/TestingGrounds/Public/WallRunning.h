// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Corvo.h"
#include "Runtime/Engine/Classes/Components/CapsuleComponent.h"
#include "WallRunning.generated.h"


UCLASS( Blueprintable, ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class TESTINGGROUNDS_API UWallRunning : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UWallRunning();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

	ACorvo* player = nullptr;

	FVector playerDir = FVector();

	bool onWall = false;

	FVector wallNormal = FVector();

	// WALLRUN ANIMATION VARIABLES
	UTimelineComponent* wallRunTimeline;
	FOnTimelineFloat WallRunInterpFunction{};
	UFUNCTION()
	void WallRunUpdate(float val);
	UPROPERTY(EditAnywhere, Category = "WallRunning")
	UCurveFloat* wallRunCurve;

	UTimelineComponent* smoothRunRightTimeline;
	FOnTimelineFloat SmoothRunRightInterpFunction{};
	UFUNCTION()
	void SmoothRunRightUpdate(float val);
	UPROPERTY(EditAnywhere, Category = "WallRunning")
	UCurveFloat* smoothRunCurve;

	UTimelineComponent* smoothRunLeftTimeline;
	FOnTimelineFloat SmoothRunLeftInterpFunction{};
	UFUNCTION()
	void SmoothRunLeftUpdate(float val);

	void TurnOffWallRunning();

	void SetOnWall(bool val);

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

		
};
