// Fill out your copyright notice in the Description page of Project Settings.


#include "BlinkAbility.h"
#include "Runtime/Engine/Classes/Engine/World.h"
#include "Runtime/Engine/Public/TimerManager.h"
#include "Runtime/Engine/Public/DrawDebugHelpers.h"
#include "Runtime/Engine/Classes/Kismet/KismetSystemLibrary.h"
#include "Runtime/Engine/Classes/Kismet/GameplayStatics.h"

// Sets default values for this component's properties
UBlinkAbility::UBlinkAbility()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// set up animation timelines for effects
	handBounceTimeline = CreateDefaultSubobject<UTimelineComponent>(TEXT("Hand Bounce Timeline"));
	HandInterpFunction.BindUFunction(this, FName("HandTimelineFloatReturn"));
	HandTimelineFinished.BindUFunction(this, FName("OnHandTimelineFinished"));
	teleportFOVTimeline = CreateDefaultSubobject<UTimelineComponent>(TEXT("Teleport FOV Timeline"));
	CamFOVInterpFunction.BindUFunction(this, FName("CamTimelineFloatReturn"));
} // end of Constructor


// Called when the game starts
void UBlinkAbility::BeginPlay()
{
	Super::BeginPlay();

	// If we have a curve for the hand animation, set it up
	if (handBounceCurve) {
		handBounceTimeline->AddInterpFloat(handBounceCurve, HandInterpFunction, FName("Alpha"));
		handBounceTimeline->SetTimelineFinishedFunc(HandTimelineFinished);

		handStartPos = Hand->RelativeLocation;
		handOffsetPos = FVector(handStartPos.X, handStartPos.Y, handStartPos.Z + handZOffset);

		handBounceTimeline->SetLooping(false);
		handBounceTimeline->SetIgnoreTimeDilation(false);

		handBounceTimeline->Play();
	}

	// If we have a curve for the fov animation, set it up
	if (cameraFOVCurve) {
		teleportFOVTimeline->AddInterpFloat(cameraFOVCurve, CamFOVInterpFunction, FName("Alpha"));

		cameraDefaultFOV = 106.0f;
		cameraTeleportFOV = cameraDefaultFOV + 20.0f;

		teleportFOVTimeline->SetLooping(false);
		teleportFOVTimeline->SetIgnoreTimeDilation(false);
	}

	if (capsule != nullptr) {
		collisionCheckRadius = capsule->GetScaledCapsuleRadius();
		collisionCheckHalfHeight = capsule->GetScaledCapsuleHalfHeight();
	}
} // end of BeginPlay()

// Hand timeline value is passed in via float
void UBlinkAbility::HandTimelineFloatReturn(float value)
{
	Hand->SetRelativeLocation(FMath::Lerp(handStartPos, handOffsetPos, value), false, nullptr, ETeleportType::None);
} // end of HandTimelineFloatReturn()

// Called when the hand timeline finishes playing
void UBlinkAbility::OnHandTimelineFinished()
{
	if (handBounceTimeline->GetPlaybackPosition() == 0.0f)
		handBounceTimeline->Play();
	else
		handBounceTimeline->Reverse();
} // end of OnHandTimelineFinished()

// cam timeline value is passed in via float
void UBlinkAbility::CamTimelineFloatReturn(float value)
{
	corvoCam->SetFieldOfView(FMath::Lerp(cameraDefaultFOV, cameraTeleportFOV, value));
} // end of CamTimelineFloatReturn

// Handle wall climbing and return whether we climbed
bool UBlinkAbility::HandleWallClimbing(FHitResult hit, bool isTeleporting)
{
	// Cannot climb if wall isn't vertical
	if (hit.ImpactNormal.Z > 0.1f) {
		if (isTeleporting) {
			//UE_LOG(LogTemp, Warning, TEXT("Surface not vertical"));
			Teleport(hit.Location);
		}
		return false;
	}

	// STEP 1: FIND THE TOP OF THE WALL
	// Draw vector from above the wall downward to hit the top of the wall
	FVector directionInsideWall = hit.ImpactNormal;
	directionInsideWall.Normalize(0.0001f);
	directionInsideWall = directionInsideWall * (-1.25f * collisionCheckRadius);
	FVector scaleTraceEnd = directionInsideWall + hit.ImpactPoint;
	FVector scaleTraceStart = scaleTraceEnd + FVector(0.0f, 0.0f, collisionCheckHalfHeight * 2.0f);
	FHitResult wallTopSurfaceHit;
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

		// STEP 2: ENSURE WE HAVE AMPLE SPACE ON TOP OF THE WALL
		// Run a sphere trace on top of the wall.  If we get a blocking hit, we can't go there.
		TArray<AActor*> list = TArray<AActor*>();
		list.Add(this->GetOwner());
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
		// STEP 3: TELEPORT
		// if we don't get a blocking hit, we can go there.
		else if (!heightTestHit.bBlockingHit) {
			if (isTeleporting) {
				UE_LOG(LogTemp, Warning, TEXT("AAAAAAAAAAAAA"));
				Teleport(wallTopSurfaceHit.ImpactPoint + FVector(0.0f, 0.0f, collisionCheckHalfHeight));
			}
			return true;
		}
		// This means we didn't hit anything with our trace.  I don't believe this should ever be reached.
		else {
			if (isTeleporting) {
				UE_LOG(LogTemp, Warning, TEXT("IDK"));
				Teleport(hit.Location);
			}
			return true;
		}
	}
	// BUG: This shouldn't be reached.  This means the initial line trace didn't find the top of the wall.
	// I suspect the line trace is coming up short of the surface sometimes.
	else {
		if (isTeleporting) {
			UE_LOG(LogTemp, Warning, TEXT("Top of wall not found"));
			Teleport(hit.Location);
		}
		return false;
	}
} // end of HandleWallClimbing()

void UBlinkAbility::Teleport(FVector teleportLoc) {
	if (cameraFOVCurve) {
		teleportFOVTimeline->PlayFromStart();

		// WAIT 0.15 seconds and then teleport
		FTimerDelegate teleportTimerDeleg;
		teleportTimerDeleg.BindUFunction(this, FName("SetNewLoc"), teleportLoc, corvoCam->GetOwner()->GetActorLocation());
		GetOwner()->GetWorldTimerManager().SetTimer(teleportDelayHandle, teleportTimerDeleg, 0.12f, false);
	}
} // end of Teleport()

void UBlinkAbility::SetNewLoc(FVector endVect)
{
	FLatentActionInfo info;
	info.CallbackTarget = this;
	UKismetSystemLibrary::MoveComponentTo(
		this->GetOwner()->GetRootComponent(),
		endVect,
		FRotator(0.0f, 0.0f, 0.0f),
		false,
		false,
		0.05f,
		true,
		EMoveComponentAction::Move,
		info
	);
} // end of SetNewLoc()

// Called when right mouse is held down
void UBlinkAbility::OnInitiateTeleport() {
	Hand->SetMaterial(0, blue);
	teleportIndicator->SetVisibility(true, true);
	teleportIndicatorActive = true;
} // end of OnInitiateTeleport()

// called when right mouse is released
void UBlinkAbility::OnTeleport() {
	Hand->SetMaterial(0, white);
	teleportIndicatorActive = false;
	teleportIndicator->SetVisibility(false, true);
	FHitResult hit;
	TArray<AActor*> list = TArray<AActor*>();
	list.Add(this->GetOwner());
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

// Called every frame
void UBlinkAbility::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if(Hand != nullptr)
		Hand->AddRelativeRotation(FRotator(0.0f, 0.3f, 0.0f), false, nullptr, ETeleportType::None);

	if(climbIndicator != nullptr)
		climbIndicator->SetVisibility(false, true);

	// handle indicator updates
	if (teleportIndicatorActive && teleportIndicator != nullptr) {
		FHitResult hit;
		TArray<AActor*> list = TArray<AActor*>();
		list.Add(this->GetOwner());
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

} // end of TickComponent()

