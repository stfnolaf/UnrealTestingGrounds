// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Climbing.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class TESTINGGROUNDS_API UClimbing : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UClimbing();

protected:
	class ACorvo* Owner;

	// LEDGE TRACER
	bool CanTraceForLedges = false;

	FVector WallTraceImpact = FVector::ZeroVector;

	FVector WallNormal = FVector::ZeroVector;

	FVector LedgeHeight = FVector::ZeroVector;

	bool IsHangingFromLedge = false;

	bool CanGrabLedge = false;

	bool JustLetGo = false;

	FTimerHandle LedgeGrabDelayHandle;

	// Called when the game starts
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	bool CanTraceForWall();

	void SetCanTraceForWall(bool value);

	bool HangingFromLedge();

	bool CanGrabOntoLedge();

	void ClimbMoveRight(float Value);

	void LetGoOfLedge();

	void GrabLedge();

	UFUNCTION()
	void ResetJustLetGo();
};
