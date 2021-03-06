// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "WeaponReturnPath.generated.h"

UCLASS()
class TESTINGGROUNDS_API AWeaponReturnPath : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AWeaponReturnPath();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	float InitialDistToTarget = 0.0f;

	UPROPERTY(EditAnywhere, Category = "Path")
	class USplineComponent* Spline;

	class AKnife* Weapon = nullptr;

	class ACorvo* Target = nullptr;

	float EaseIn(float alpha);

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	USplineComponent* GetSpline();

	void SetKnifeOwnerAndTarget(AKnife* weapons, ACorvo* owner);

	void UpdatePath(FRotator TargetRotation);

	float UpdateReturnSpeed(float distance);

};
