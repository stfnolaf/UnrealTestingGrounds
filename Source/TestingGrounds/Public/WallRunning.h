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

	AActor* lastWall = nullptr;

	FVector playerDir = FVector();

	bool onWall = false;

	void StartWallRunning();

	void StopWallRunning();

	float timeStartNewRotateAnim = 0.0f;

	float timeToRotateOnWall = 0.25f;

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

		
};
