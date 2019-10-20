// Fill out your copyright notice in the Description page of Project Settings.


#include "WallRunning.h"
#include "Corvo.h"
#include "Runtime/Engine/Classes/GameFramework/PawnMovementComponent.h"
#include "Runtime/Engine/Classes/Kismet/GameplayStatics.h"
#include "Runtime/Engine/Classes/GameFramework/CharacterMovementComponent.h"
#include "Runtime/Engine/Classes/Engine/Engine.h"
#include "Runtime/Engine/Classes/Kismet/KismetSystemLibrary.h"
#include "Runtime/Engine/Classes/Kismet/KismetMathLibrary.h"

// Sets default values for this component's properties
UWallRunning::UWallRunning()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
}


// Called when the game starts
void UWallRunning::BeginPlay()
{
	Super::BeginPlay();

	player = Cast<ACorvo>(GetOwner());
	if (player == nullptr) {
		UE_LOG(LogTemp, Warning, TEXT("PLAYER VARIABLE NOT INITIALIZED"));
	}
	else {
		player->GetMovementComponent()->SetPlaneConstraintEnabled(true);
	}

	// ...
	myPC = UGameplayStatics::GetPlayerController(GetWorld(), 0);
}


// Called every frame
void UWallRunning::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (!player->GetMovementComponent()->IsFalling() && player->IsOnGround()) {
		lastWall = nullptr;
	}

	// Fire raycasts to left and right of player
	FHitResult leftHit;
	FHitResult rightHit;
	FVector startLoc = player->GetActorLocation();
	FVector rightVect = player->GetActorRightVector();
	FVector forwardVect = player->GetActorForwardVector();
	FVector diagonalLeftVect = (forwardVect - rightVect).GetSafeNormal();
	FVector diagonalRightVect = (forwardVect + rightVect).GetSafeNormal();
	bool contactLeft = false;
	bool contactRight = false;
	FHitResult finalHitResult;
	bool finalHitIsLeft = false;

	float lineTraceDist = 55.0f;
	if (onWall)
		lineTraceDist = 100.0f;
	
	contactLeft = GetWorld()->LineTraceSingleByChannel(leftHit, startLoc, startLoc - lineTraceDist * rightVect, ECollisionChannel::ECC_Visibility);
	if(!contactLeft && directionLocked || (contactLeft && leftHit.Actor.IsValid() && !leftHit.Actor.Get()->ActorHasTag("Wall")))
		contactLeft = GetWorld()->LineTraceSingleByChannel(leftHit, startLoc, startLoc + lineTraceDist * diagonalLeftVect, ECollisionChannel::ECC_Visibility);
	contactRight = GetWorld()->LineTraceSingleByChannel(rightHit, startLoc, startLoc + lineTraceDist * rightVect, ECollisionChannel::ECC_Visibility);
	if(!contactRight && directionLocked || (contactRight && rightHit.Actor.IsValid() && !rightHit.Actor.Get()->ActorHasTag("Wall")))
		contactRight = GetWorld()->LineTraceSingleByChannel(rightHit, startLoc, startLoc + lineTraceDist * diagonalRightVect, ECollisionChannel::ECC_Visibility);

	// If actor is tagged as a wall and the impact normal's z coordinate is less than 0.1f

	if (contactLeft) {
		contactLeft = contactLeft && leftHit.ImpactNormal.Z < 0.1f && leftHit.ImpactNormal.Z >= -0.05f && leftHit.Actor.IsValid() && leftHit.Actor->ActorHasTag("Wall");
		contactLeft = contactLeft && (lastWall == nullptr || (lastWall != nullptr && leftHit.Actor.IsValid() && !UKismetMathLibrary::EqualEqual_ObjectObject(lastWall, leftHit.Actor.Get())));
	}

	if (contactRight) {
		contactRight = contactRight && rightHit.ImpactNormal.Z < 0.1f && rightHit.ImpactNormal.Z >= -0.05f && rightHit.Actor.IsValid() && rightHit.Actor->ActorHasTag("Wall");
		contactRight = contactRight && (lastWall == nullptr || (lastWall != nullptr && rightHit.Actor.IsValid() && !UKismetMathLibrary::EqualEqual_ObjectObject(lastWall, rightHit.Actor.Get())));
	}

	// if both raycasts hit an acceptable surface, discard the further one
	if (contactLeft && contactRight) {
		if ((leftHit.ImpactPoint - startLoc).Size() < (rightHit.ImpactPoint - startLoc).Size()) {
			contactRight = false;
			finalHitIsLeft = true;
			finalHitResult = leftHit;
		}
		else {
			contactLeft = false;
			finalHitIsLeft = false;
			finalHitResult = rightHit;
		}
	}
	else {
		if (contactLeft) {
			finalHitIsLeft = true;
			finalHitResult = leftHit;
		}
		else if (contactRight) {
			finalHitIsLeft = false;
			finalHitResult = rightHit;
		}
	}

	FVector wallNormal = finalHitResult.ImpactNormal;
	if (finalHitResult.Actor.IsValid() && finalHitResult.Actor.Get()->ActorHasTag("Curved")) {
		wallNormal = finalHitResult.ImpactPoint - finalHitResult.Actor.Get()->GetActorLocation();
		wallNormal = FVector(wallNormal.X, wallNormal.Y, 0.0f).GetSafeNormal();
		if (lastWallNormal != FVector::ZeroVector) {
			float deltaAngle = wallNormal.Rotation().Yaw - lastWallNormal.Rotation().Yaw;
			myPC->SetControlRotation(FRotator(myPC->GetControlRotation().Pitch, myPC->GetControlRotation().Yaw + deltaAngle, myPC->GetControlRotation().Roll));
		}
		lastWallNormal = wallNormal;
	}
	// set wallrunning direction based on impact normal
	if (contactLeft || contactRight) {
		if(finalHitIsLeft)
			playerDir = FVector::CrossProduct(wallNormal, FVector::UpVector);
		else
			playerDir = FVector::CrossProduct(wallNormal, FVector::DownVector);

		player->SetRailDir(playerDir);
		bool acceptableAngle = (finalHitIsLeft && FMath::FindDeltaAngleDegrees(playerDir.Rotation().Yaw, myPC->GetControlRotation().Yaw) < 15.0f) || (!finalHitIsLeft && FMath::FindDeltaAngleDegrees(playerDir.Rotation().Yaw, myPC->GetControlRotation().Yaw) > -15.0f);
		FHitResult groundCheck;
		bool groundHit = GetWorld()->LineTraceSingleByChannel(groundCheck, player->GetActorLocation(), player->GetActorLocation() + FVector::DownVector * 150.0f, ECollisionChannel::ECC_Visibility);
		if (!onWall
			&& player->GetMovementComponent()->IsFalling()
			&& !groundHit && acceptableAngle)
		{
			StartWallRunning();
		}
	}
	APlayerController* myPC = UGameplayStatics::GetPlayerController(GetWorld(), 0);

	if (onWall) {

		if (!directionLocked && finalHitIsLeft && FMath::FindDeltaAngleDegrees(playerDir.Rotation().Yaw, myPC->GetControlRotation().Yaw) > -30.0f)
			directionLocked = true;
		else if (!directionLocked && !finalHitIsLeft && FMath::FindDeltaAngleDegrees(playerDir.Rotation().Yaw, myPC->GetControlRotation().Yaw) < 30.0f)
			directionLocked = true;

		if (finalHitIsLeft && FMath::FindDeltaAngleDegrees(playerDir.Rotation().Yaw, myPC->GetControlRotation().Yaw) < 5.0f) {
			myPC->SetControlRotation(FMath::RInterpTo(myPC->GetControlRotation(), FRotator(myPC->GetControlRotation().Pitch, playerDir.Rotation().Yaw + 5.0f, myPC->GetControlRotation().Roll), DeltaTime, 2.0f));
		}
		else if (!finalHitIsLeft && FMath::FindDeltaAngleDegrees(playerDir.Rotation().Yaw, myPC->GetControlRotation().Yaw) > -5.0f) {
			myPC->SetControlRotation(FMath::RInterpTo(myPC->GetControlRotation(), FRotator(myPC->GetControlRotation().Pitch, playerDir.Rotation().Yaw - 5.0f, myPC->GetControlRotation().Roll), DeltaTime, 2.0f));
		}
		else if (finalHitIsLeft && FMath::FindDeltaAngleDegrees(playerDir.Rotation().Yaw, myPC->GetControlRotation().Yaw) > 20.0f) {
			myPC->SetControlRotation(FMath::RInterpTo(myPC->GetControlRotation(), FRotator(myPC->GetControlRotation().Pitch, playerDir.Rotation().Yaw + 20.0f, myPC->GetControlRotation().Roll), DeltaTime, 2.0f));
		}
		else if (!finalHitIsLeft && FMath::FindDeltaAngleDegrees(playerDir.Rotation().Yaw, myPC->GetControlRotation().Yaw) < -20.0f) {
			myPC->SetControlRotation(FMath::RInterpTo(myPC->GetControlRotation(), FRotator(myPC->GetControlRotation().Pitch, playerDir.Rotation().Yaw - 20.0f, myPC->GetControlRotation().Roll), DeltaTime, 2.0f));
		}

		if (finalHitIsLeft) {
			//rotate to left wall
			myPC->SetControlRotation(FMath::RInterpTo(myPC->GetControlRotation(), FRotator(myPC->GetControlRotation().Pitch, myPC->GetControlRotation().Yaw, 12.5f), DeltaTime, 10.0f));
		}
		else {
			//rotate to right wall
			myPC->SetControlRotation(FMath::RInterpTo(myPC->GetControlRotation(), FRotator(myPC->GetControlRotation().Pitch, myPC->GetControlRotation().Yaw, -12.5f), DeltaTime, 10.0f));
		}

		// keep player constant distance from wall
		if (finalHitResult.Actor.IsValid()) {
			FPlane wallPlane = FPlane(finalHitResult.ImpactPoint, wallNormal);
			float dist = wallPlane.PlaneDot(player->GetActorLocation());

			if (dist < 45.0f) {
				// move player further from the wall
				player->SetActorLocation(FMath::VInterpTo(player->GetActorLocation(), finalHitResult.ImpactPoint + 45.0f * wallNormal, DeltaTime, 20.0f));
			}
			else if (dist > 45.0f) {
				// move player towards the wall
				player->SetActorLocation(FMath::VInterpTo(player->GetActorLocation(), finalHitResult.ImpactPoint + 45.0f * wallNormal, DeltaTime, 20.0f));
			}
		}

		if (UGameplayStatics::GetPlayerController(GetWorld(), 0)->WasInputKeyJustPressed(EKeys::SpaceBar)) {
			StopWallRunning();
			if (finalHitResult.Actor.IsValid()) {
				lastWall = finalHitResult.Actor.Get();
			}
			else {
				UE_LOG(LogTemp, Error, TEXT("LASTWALL COULD NOT BE SET - WALLRUNNING::TICKCOMPONENT()"));
			}
			player->ResetJumps();
			player->MyJump();
		}

		if (UGameplayStatics::GetPlayerController(GetWorld(), 0)->GetInputKeyTimeDown(EKeys::W) == 0.0f
			|| (finalHitIsLeft && FMath::FindDeltaAngleDegrees(playerDir.Rotation().Yaw, myPC->GetControlRotation().Yaw) > 30.0f)
			|| (!finalHitIsLeft && FMath::FindDeltaAngleDegrees(playerDir.Rotation().Yaw, myPC->GetControlRotation().Yaw) < -30.0f)
			|| (finalHitIsLeft && FMath::FindDeltaAngleDegrees(playerDir.Rotation().Yaw, myPC->GetControlRotation().Yaw) < -30.0f && directionLocked)
			|| (!finalHitIsLeft && FMath::FindDeltaAngleDegrees(playerDir.Rotation().Yaw, myPC->GetControlRotation().Yaw) > 30.0f && directionLocked)) {
			StopWallRunning();
			if (finalHitResult.Actor.IsValid()) {
				lastWall = finalHitResult.Actor.Get();
			}
		}

		if (!contactLeft && !contactRight) {
			StopWallRunning();
		}
	}
	else if (myPC->GetControlRotation().Roll != 0.0f) {
		myPC->SetControlRotation(FMath::RInterpTo(myPC->GetControlRotation(), FRotator(myPC->GetControlRotation().Pitch, myPC->GetControlRotation().Yaw, 0.0f), DeltaTime, 10.0f));
	}
} // end of TickComponent()

void UWallRunning::StartWallRunning() {
	onWall = true;
	player->GetCharacterMovement()->GravityScale = 0.0f;
	player->GetCharacterMovement()->SetPlaneConstraintNormal(FVector::UpVector);
	player->GetCharacterMovement()->AirControl = 1.0f;
	player->GetCharacterMovement()->MaxWalkSpeed = 850.0f;
	player->LockRailMovement();
	player->ResetJumps();
}

void UWallRunning::StopWallRunning() {
	onWall = false;
	player->GetCharacterMovement()->GravityScale = 1.0f;
	player->GetCharacterMovement()->SetPlaneConstraintNormal(FVector::ZeroVector);
	player->GetCharacterMovement()->AirControl = 0.5f;
	player->GetCharacterMovement()->MaxWalkSpeed = 600.0f;
	player->UnlockRailMovement();
	directionLocked = false;
	lastWallNormal = FVector::ZeroVector;
}