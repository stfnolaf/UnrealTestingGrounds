// Fill out your copyright notice in the Description page of Project Settings.


#include "Corvo.h"
#include "Runtime/Engine/Classes/Engine/World.h"
#include "Runtime/Engine/Public/TimerManager.h"
#include "Runtime/Engine/Public/DrawDebugHelpers.h"
#include "Runtime/Engine/Classes/Kismet/KismetSystemLibrary.h"
#include "Runtime/Engine/Classes/Kismet/GameplayStatics.h"

// Sets default values
ACorvo::ACorvo()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	// SET UP HAND BOUNCE TIMELINE
	handBounceTimeline = CreateDefaultSubobject<UTimelineComponent>(TEXT("Hand Bounce Timeline"));

	// SET UP DELEGATES FOR HAND BOUNCE MOVEMENT
	HandInterpFunction.BindUFunction(this, FName("HandTimelineFloatReturn"));
	HandTimelineFinished.BindUFunction(this, FName("OnHandTimelineFinished"));

	// SET UP TELEPORTATION FOV TIMELINE
	teleportFOVTimeline = CreateDefaultSubobject<UTimelineComponent>(TEXT("Teleport FOV Timeline"));

	// SET UP DELEGATES FOR CAMERA FOV
	CamFOVInterpFunction.BindUFunction(this, FName("CamTimelineFloatReturn"));
}

void ACorvo::HandTimelineFloatReturn(float value)
{
	Hand->SetRelativeLocation(FMath::Lerp(handStartPos, handOffsetPos, value), false, nullptr, ETeleportType::None);
}

void ACorvo::OnHandTimelineFinished()
{
	if (handBounceTimeline->GetPlaybackPosition() == 0.0f)
		handBounceTimeline->Play();
	else
		handBounceTimeline->Reverse();
}

void ACorvo::CamTimelineFloatReturn(float value)
{
	corvoCam->SetFieldOfView(FMath::Lerp(cameraDefaultFOV, cameraTeleportFOV, value));
}

bool ACorvo::HandleWallClimbing(FHitResult hit, bool isTeleporting)
{
	// IF THE STRUCK PART OF THE WALL IS VERTICAL
	if (hit.ImpactNormal.Z > 0.1f) {
		if (isTeleporting) {
			UE_LOG(LogTemp, Warning, TEXT("Surface not vertical"));
			Teleport(hit.Location);
		}
		return false;
	}

	// TODO - set these based on capsule collider
	float collisionCheckRadius = 34.0f;
	float collisionCheckHalfHeight = 88.0f;

	// Draw vector down to find top of wall
	FVector directionInsideWall = hit.ImpactNormal;
	directionInsideWall.Normalize(0.0001f);
	directionInsideWall = directionInsideWall * (-1.25f * collisionCheckRadius);
	FVector scaleTraceEnd = directionInsideWall + hit.ImpactPoint;
	FVector scaleTraceStart = scaleTraceEnd + FVector(0.0f, 0.0f, collisionCheckHalfHeight * 2.0f);
	FHitResult wallTopSurfaceHit;
	// If it hits something we've found the top of the wall
	if (GetWorld()->LineTraceSingleByChannel(wallTopSurfaceHit, scaleTraceStart, scaleTraceEnd, ECollisionChannel::ECC_Visibility)) {
		// make sure we can stand on the surface
		if (!wallTopSurfaceHit.bBlockingHit) {
			if (isTeleporting) {
				UE_LOG(LogTemp, Warning, TEXT("Teleport failed.  Surface too small."));
				Teleport(hit.Location);
			}
			return false;
		}

		// Make sure our character can reach the top of the wall
		if (wallTopSurfaceHit.ImpactPoint.Z - wallTopSurfaceHit.TraceEnd.Z > collisionCheckHalfHeight) {
			if (isTeleporting) {
				UE_LOG(LogTemp, Warning, TEXT("Teleport failed.  Wall top not in reachable distance %f"), (wallTopSurfaceHit.ImpactPoint.Z - wallTopSurfaceHit.TraceEnd.Z));
				Teleport(hit.Location);
			}
			return false;
		}

		// Run a sphere trace on top of the wall.  If we get a blocking hit, we can't go there.
		TArray<AActor*> list = TArray<AActor*>();
		list.Add(this);
		FHitResult heightTestHit;
		if (UKismetSystemLibrary::SphereTraceSingle(
			GetWorld(),
			wallTopSurfaceHit.ImpactPoint + FVector(0.0f, 0.0f, collisionCheckRadius + 1.0f),
			wallTopSurfaceHit.ImpactPoint + FVector(0.0f, 0.0f, (collisionCheckHalfHeight - collisionCheckRadius) * 2.0f),
			collisionCheckRadius,
			ETraceTypeQuery::TraceTypeQuery1,
			false,
			list,
			EDrawDebugTrace::None,
			heightTestHit,
			true) && heightTestHit.bBlockingHit
		) {
			if (isTeleporting) {
				UE_LOG(LogTemp, Warning, TEXT("Teleport failed.  Insufficient room on top of wall."));
				Teleport(hit.Location);
			}
			return false;
		}
		// if we don't get a blocking hit, we can go there.
		else if (!heightTestHit.bBlockingHit) {
			if (isTeleporting) {
				UE_LOG(LogTemp, Warning, TEXT("AAAAAAAAAAAAA"));
				Teleport(wallTopSurfaceHit.ImpactPoint + FVector(0.0f, 0.0f, collisionCheckHalfHeight));
			}
			return true;
		}
		// Not even sure how to reach this tbh
		else {
			if (isTeleporting) {
				UE_LOG(LogTemp, Warning, TEXT("IDK"));
				Teleport(hit.Location);
			}
			return true;
		}
	}
	// This means the initial line trace didn't find the top of the wall.  I suspect this means the line trace is coming up short of the surface.
	else {
		if (isTeleporting) {
			UE_LOG(LogTemp, Warning, TEXT("Top of wall not found"));
			Teleport(hit.Location);
		}
		return false;
	}
}

// Called when the game starts or when spawned
void ACorvo::BeginPlay()
{
	Super::BeginPlay();

	// IF WE HAVE A CURVE
	if (handBounceCurve) {
		// SET CALL HandInterpFunction WITH handBounceCurve AS VALUES WHILE RUNNING TIMELINE
		handBounceTimeline->AddInterpFloat(handBounceCurve, HandInterpFunction, FName("Alpha"));
		// ON TIMELINE FINISHED CALL HandTimelineFinished DELEGATE
		handBounceTimeline->SetTimelineFinishedFunc(HandTimelineFinished);

		// SET UP HAND START AND END POSITIONS
		handStartPos = Hand->RelativeLocation;
		handOffsetPos = FVector(handStartPos.X, handStartPos.Y, handStartPos.Z + handZOffset);

		// OTHER TIMELINE PROPERTIES
		handBounceTimeline->SetLooping(false);
		handBounceTimeline->SetIgnoreTimeDilation(false);

		// LET THIS TIMELINE LOOP AND REVERSE ENDLESSLY
		handBounceTimeline->Play();
	}

	// IF WE HAVE A CAMERA FOV CURVE
	if (cameraFOVCurve) {
		// SET CALL CamFOVInterpFunction WITH cameraFOVCurve AS VALUES WHILE RUNNINE TIMELINE
		teleportFOVTimeline->AddInterpFloat(cameraFOVCurve, CamFOVInterpFunction, FName("Alpha"));

		// SET UP FOV START AND END VALUES
		cameraDefaultFOV = 106.0f;
		cameraTeleportFOV = cameraDefaultFOV + 20.0f;

		// OTHER TIMELINE PROPERTIES
		teleportFOVTimeline->SetLooping(false);
		teleportFOVTimeline->SetIgnoreTimeDilation(false);
	}
}

void ACorvo::MoveForward(float Value) {
	if (Value != 0.0f) {
		AddMovementInput(GetActorForwardVector(), Value);
	}
}

void ACorvo::MoveRight(float Value) {
	if (Value != 0.0f) {
		AddMovementInput(GetActorRightVector(), Value);
	}
}

void ACorvo::OnInitiateTeleport() {
	Hand->SetMaterial(0, blue);
	teleportIndicator->SetVisibility(true, true);
	teleportIndicatorActive = true;
}

void ACorvo::OnTeleport() {
	Hand->SetMaterial(0, white);
	teleportIndicatorActive = false;
	teleportIndicator->SetVisibility(false, true);
	FHitResult hit;
	TArray<AActor*> list = TArray<AActor*>();
	list.Add(this);
	if (UKismetSystemLibrary::SphereTraceSingle(
			GetWorld(),
			corvoCam->GetComponentLocation(),
			corvoCam->GetForwardVector() * maxTeleportDistance + corvoCam->GetComponentLocation(),
			34.0f,
			ETraceTypeQuery::TraceTypeQuery1,
			false,
			list,
			EDrawDebugTrace::None,
			hit,
			true
		)
	) {
		// IF WE HIT A WALL
		if (hit.Actor->ActorHasTag("Wall")) {
			HandleWallClimbing(hit, true);
		}
		else {
			Teleport(hit.Location);
		}
	}
	else {
		Teleport(corvoCam->GetForwardVector() * maxTeleportDistance + corvoCam->GetComponentLocation());
	}

}

void ACorvo::Teleport(FVector teleportLoc) {
	if (cameraFOVCurve) {
		teleportFOVTimeline->PlayFromStart();

		// WAIT 0.15 seconds and then teleport
		FTimerDelegate teleportTimerDeleg;
		teleportTimerDeleg.BindUFunction(this, FName("SetNewLoc"), teleportLoc, corvoCam->GetOwner()->GetActorLocation());
		GetWorldTimerManager().SetTimer(teleportDelayHandle, teleportTimerDeleg, 0.12f, false);
	}
}

void ACorvo::SetNewLoc(FVector endVect)
{
	FLatentActionInfo info;
	info.CallbackTarget = this;
	UKismetSystemLibrary::MoveComponentTo(
		GetRootComponent(), 
		endVect, 
		FRotator(0.0f,0.0f,0.0f), 
		false, 
		false, 
		0.05f, 
		true,
		EMoveComponentAction::Move, 
		info
	);
}

// Called every frame
void ACorvo::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	Hand->AddRelativeRotation(FRotator(0.0f, 0.3f, 0.0f), false, nullptr, ETeleportType::None);

	climbIndicator->SetVisibility(false, true);

	if (teleportIndicatorActive && teleportIndicator != nullptr) {
		FHitResult hit;
		TArray<AActor*> list = TArray<AActor*>();
		list.Add(this);
		if (UKismetSystemLibrary::SphereTraceSingle(
				GetWorld(),
				corvoCam->GetComponentLocation(),
				corvoCam->GetForwardVector() * maxTeleportDistance + corvoCam->GetComponentLocation(),
				34.0f,
				ETraceTypeQuery::TraceTypeQuery1,
				false,
				list,
				EDrawDebugTrace::None,
				hit,
				true
			)
		) {
			if (hit.Actor->ActorHasTag("Wall")) {
				climbIndicator->SetVisibility(HandleWallClimbing(hit, false), true);
			}
			teleportIndicator->SetWorldLocation(hit.Location, false, nullptr, ETeleportType::TeleportPhysics);
		}
		else {
			teleportIndicator->SetWorldLocation(corvoCam->GetForwardVector() * maxTeleportDistance + corvoCam->GetComponentLocation(), false, nullptr, ETeleportType::TeleportPhysics);
		}
	}
}

// Called to bind functionality to input
void ACorvo::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	//Super::SetupPlayerInputComponent(PlayerInputComponent);

	// set up gameplay key bindings
	check(PlayerInputComponent);

	// Bind jump events
	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &ACharacter::Jump);
	PlayerInputComponent->BindAction("Jump", IE_Released, this, &ACharacter::StopJumping);

	// Bind movement events
	PlayerInputComponent->BindAxis("MoveForward", this, &ACorvo::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &ACorvo::MoveRight);

	PlayerInputComponent->BindAxis("Turn", this, &APawn::AddControllerYawInput);
	PlayerInputComponent->BindAxis("LookUp", this, &APawn::AddControllerPitchInput);

	PlayerInputComponent->BindAction("Teleport", IE_Pressed, this, &ACorvo::OnInitiateTeleport);
	PlayerInputComponent->BindAction("Teleport", IE_Released, this, &ACorvo::OnTeleport);

}

