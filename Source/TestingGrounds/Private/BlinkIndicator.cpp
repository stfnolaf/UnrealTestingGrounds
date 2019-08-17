// Fill out your copyright notice in the Description page of Project Settings.


#include "BlinkIndicator.h"

// Sets default values
ABlinkIndicator::ABlinkIndicator()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void ABlinkIndicator::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void ABlinkIndicator::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void ABlinkIndicator::SetIndicatorVisibility(bool visible)
{
	if (!teleportIndicator)
		return;
	if (visible)
		teleportIndicator->SetVisibility(visible, false);
	else
		teleportIndicator->SetVisibility(visible, true);
}

void ABlinkIndicator::SetClimbVisibility(bool visible)
{
	if (!climbIndicator)
		return;
	climbIndicator->SetVisibility(visible, true);
}

