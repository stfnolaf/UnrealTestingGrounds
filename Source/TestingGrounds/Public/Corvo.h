// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "BlinkAbility.h"
#include "Corvo.generated.h"

UCLASS()
class TESTINGGROUNDS_API ACorvo : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	ACorvo();

protected:

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Corvo")
	UStaticMeshComponent* Hand = nullptr;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Corvo")
	UCameraComponent* myCamera = nullptr;

	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	// CONTROL METHODS
	void MoveRight(float Val);
	void MoveForward(float Val);
	void OnInitiateAbility();
	void OnReleaseAbility();
	void OnQuit();

	TArray<UBlinkAbility*> abilities;

	int activeAbility = 0;

	int numJumps = 0;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	UStaticMeshComponent* GetHand();

	UCameraComponent* GetCamera();

	virtual void Landed(const FHitResult& hit) override;

	void MyJump();

};
