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

	void ShootCable(float deltaTime);

	UFUNCTION(BlueprintCallable, Category = "Grapple")
	void ResetGrapple();

	float deltaTime = 0.0f;

	bool canGrapple = true;

	bool grapplingHookEnabled = false;

	float timeShootingGrapple = 0.0f;

	UPROPERTY(EditAnywhere, Category = "Corvo")
	float grappleDistance = 3000.0f;

	UPROPERTY(EditAnywhere, Category = "Corvo")
	float grappleCooldown = 1.0f;

	UFUNCTION(BlueprintCallable, Category = "Grapple")
	void GrappleAfterDelay();

	UFUNCTION(BlueprintCallable, Category = "Grapple")
	void ResetGrappleCoolDownAfterDelay();

	UFUNCTION()
	void OnHitWall(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	void OnInitiateAbility();

	void OnReleaseAbility();
};
