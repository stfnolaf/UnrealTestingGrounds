// Fill out your copyright notice in the Description page of Project Settings.


#include "Climbing.h"
#include "Corvo.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Runtime/Engine/Public/TimerManager.h"

// Sets default values for this component's properties
UClimbing::UClimbing()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
}


// Called when the game starts
void UClimbing::BeginPlay()
{
	Super::BeginPlay();

	Owner = Cast<ACorvo>(GetOwner());
	if (Owner == nullptr) {
		UE_LOG(LogTemp, Warning, TEXT("PLAYER VARIABLE NOT INITIALIZED"));
	}
}


// Called every frame
void UClimbing::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	CanGrabLedge = false;

	if (CanTraceForLedges || IsHangingFromLedge) {
		FHitResult WallHit;
		FHitResult LedgeHit;
		TArray<AActor*> IgnoreActors = TArray<AActor*>();
		IgnoreActors.Add(Owner);
		FVector OwnerLoc = Owner->GetActorLocation();
		FVector OwnerFwdVector = Owner->GetActorForwardVector();
		if (UKismetSystemLibrary::SphereTraceSingle(GetWorld(), OwnerLoc, OwnerLoc + 150.0f * FVector(OwnerFwdVector.X, OwnerFwdVector.Y, 0.0f), 20.0f, ETraceTypeQuery::TraceTypeQuery3, false, IgnoreActors, EDrawDebugTrace::ForOneFrame, WallHit, true)) {
			WallTraceImpact = WallHit.ImpactPoint;
			WallNormal = WallHit.ImpactNormal;
		}
		if (UKismetSystemLibrary::SphereTraceSingle(GetWorld(), FVector(OwnerLoc.X, OwnerLoc.Y, OwnerLoc.Z + 500.0f) + OwnerFwdVector * 70.0f, OwnerLoc + OwnerFwdVector * 70.0f, 20.0f, ETraceTypeQuery::TraceTypeQuery3, false, IgnoreActors, EDrawDebugTrace::ForOneFrame, LedgeHit, true)) {
			LedgeHeight = LedgeHit.ImpactPoint;
			if (!IsHangingFromLedge) {
				if (Owner->GetCharacterMovement()->IsFalling() && !JustLetGo) {
					GrabLedge();
				}
				else {
					CanGrabLedge = true;
				}
			}
		}
	}
}

void UClimbing::LetGoOfLedge() {
	Owner->GetCharacterMovement()->SetMovementMode(EMovementMode::MOVE_Walking);
	IsHangingFromLedge = false;
	JustLetGo = true;
	FTimerDelegate letGoTimerDeleg;
	letGoTimerDeleg.BindUFunction(this, FName("ResetJustLetGo"));
	GetOwner()->GetWorldTimerManager().SetTimer(LedgeGrabDelayHandle, letGoTimerDeleg, 1.0f, false);
	Owner->GetCharacterMovement()->BrakingDecelerationFlying = 0.0f;
}

void UClimbing::ResetJustLetGo() {
	JustLetGo = false;
}

void UClimbing::GrabLedge() {
	Owner->ResetJumps();
	IsHangingFromLedge = true;
	Owner->GetCharacterMovement()->SetMovementMode(EMovementMode::MOVE_Flying);
	float CapsuleRadius = Owner->GetCapsuleComponent()->GetScaledCapsuleRadius();
	FVector TempVector = FVector(WallNormal * FVector(CapsuleRadius, CapsuleRadius, 0.0f));
	FVector RelativeLocation = FVector(TempVector.X + WallTraceImpact.X, TempVector.Y + WallTraceImpact.Y, LedgeHeight.Z - Owner->GetCapsuleComponent()->GetScaledCapsuleHalfHeight());
	FLatentActionInfo info;
	info.CallbackTarget = this;
	UKismetSystemLibrary::MoveComponentTo(Owner->GetCapsuleComponent(), RelativeLocation, FRotator(0.0f, UKismetMathLibrary::FindLookAtRotation(Owner->GetActorLocation(), WallTraceImpact).Yaw, 0.0f), false, false, 0.2f, true, EMoveComponentAction::Move, info);
	Owner->GetCharacterMovement()->StopMovementImmediately();
	CanGrabLedge = false;
	Owner->GetCharacterMovement()->BrakingDecelerationFlying = 1000.0f;
}

void UClimbing::ClimbMoveRight(float Value) {
	Owner->AddMovementInput(FVector::CrossProduct(WallNormal, FVector::UpVector * Value) * 0.2f);
}

bool UClimbing::CanTraceForWall() {
	return this->CanTraceForLedges;
}

void UClimbing::SetCanTraceForWall(bool value) {
	CanTraceForLedges = value;
}

bool UClimbing::HangingFromLedge() {
	return this->IsHangingFromLedge;
}

bool UClimbing::CanGrabOntoLedge() {
	return this->CanGrabLedge;
}