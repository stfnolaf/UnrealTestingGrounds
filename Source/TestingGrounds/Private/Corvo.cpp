// Fill out your copyright notice in the Description page of Project Settings.


#include "Corvo.h"
#include "Runtime/Engine/Classes/Engine/World.h"

// Sets default values
ACorvo::ACorvo()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	handBounceTimeline = CreateDefaultSubobject<UTimelineComponent>(TEXT("Timeline"));

	HandInterpFunction.BindUFunction(this, FName("HandTimelineFloatReturn"));
	HandTimelineFinished.BindUFunction(this, FName("OnHandTimelineFinished"));

	handZOffset = 5.0f;

	cameraFOVTimeline = CreateDefaultSubobject<UTimelineComponent>(TEXT("FOV Timeline"));

	CamFOVInterpFunction.BindUFunction(this, FName("CamTimelineFloatReturn"));
	CamFOVTimelineFinished.BindUFunction(this, FName("OnCamTimelineFinished"));
}

void ACorvo::HandTimelineFloatReturn(float value)
{
	Hand->SetRelativeLocation(FMath::Lerp(startLocation, endLocation, value), false, nullptr, ETeleportType::None);
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

void ACorvo::OnCamTimelineFinished()
{
	return;
}

// Called when the game starts or when spawned
void ACorvo::BeginPlay()
{
	Super::BeginPlay();

	if (handBounceCurve) {
		handBounceTimeline->AddInterpFloat(handBounceCurve, HandInterpFunction, FName("Alpha"));
		handBounceTimeline->SetTimelineFinishedFunc(HandTimelineFinished);

		startLocation = Hand->RelativeLocation;

		endLocation = FVector(startLocation.X, startLocation.Y, startLocation.Z + handZOffset);

		handBounceTimeline->SetLooping(false);
		handBounceTimeline->SetIgnoreTimeDilation(false);

		handBounceTimeline->Play();
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
}

void ACorvo::OnTeleport() {
	Hand->SetMaterial(0, white);
	FHitResult hit;
	if (GetWorld()->LineTraceSingleByChannel(
			hit, 
			corvoCam->GetOwner()->GetActorLocation(),
			corvoCam->GetOwner()->GetActorForwardVector() * 8000.0f + corvoCam->GetOwner()->GetActorLocation(),
			ECollisionChannel::ECC_Visibility
		) && cameraFOVCurve
	) {
		cameraFOVTimeline->AddInterpFloat(cameraFOVCurve, CamFOVInterpFunction, FName("Alpha"));
		cameraFOVTimeline->SetTimelineFinishedFunc(CamFOVTimelineFinished);

		cameraDefaultFOV = 106.0f;

		cameraTeleportFOV = cameraDefaultFOV + 20.0f;

		cameraFOVTimeline->SetLooping(false);
		cameraFOVTimeline->SetIgnoreTimeDilation(false);

		cameraFOVTimeline->PlayFromStart();

		//TODO - add 0.15 second delay and then set new character location
	}

}

void ACorvo::SetNewLoc()
{
}

// Called every frame
void ACorvo::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	Hand->AddRelativeRotation(FRotator(0.0f, 0.3f, 0.0f), false, nullptr, ETeleportType::None);
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

	// We have 2 versions of the rotation bindings to handle different kinds of devices differently
	// "turn" handles devices that provide an absolute delta, such as a mouse.
	// "turnrate" is for devices that we choose to treat as a rate of change, such as an analog joystick
	PlayerInputComponent->BindAxis("Turn", this, &APawn::AddControllerYawInput);
	PlayerInputComponent->BindAxis("LookUp", this, &APawn::AddControllerPitchInput);

	PlayerInputComponent->BindAction("Teleport", IE_Pressed, this, &ACorvo::OnInitiateTeleport);
	PlayerInputComponent->BindAction("Teleport", IE_Released, this, &ACorvo::OnTeleport);

}

