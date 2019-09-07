// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Grapple.generated.h"


UCLASS(Blueprintable, ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class TESTINGGROUNDS_API UGrapple : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UGrapple();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

	class ACorvo* player = nullptr;

	APlayerController* myPC = nullptr;

	FVector hookLocation = FVector();

	bool hooked = false;

	bool hookMoveFinished = false;

	bool MoveCable(float deltaTime);

	bool toggleGrapple = true;

	bool canSetCableLength = true;

	float newCableLength = 0.0f;

	void ResetGrapple();

	bool grappleButtonPressed = false;

	void MovePlayer(float deltaTime);

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	void OnInitiateAbility();

	void OnReleaseAbility();
};
