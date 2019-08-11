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
		Teleport(hit.Location);
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
		GetWorldTimerManager().SetTimer(teleportDelayHandle, teleportTimerDeleg, 0.125f, false);
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

