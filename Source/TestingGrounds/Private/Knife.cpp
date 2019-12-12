// Fill out your copyright notice in the Description page of Project Settings.


#include "Knife.h"

// Sets default values
AKnife::AKnife()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void AKnife::BeginPlay()
{
	FOnTimelineFloat onKnifeRotTimelineCallback;
	FOnTimelineEventStatic onKnifeRotTimelineFinishedCallback;

	FOnTimelineFloat onKnifeThrowTraceTimelineCallback;
	FOnTimelineEventStatic onKnifeThrowTraceTimelineFinishedCallback;

	Super::BeginPlay();
	
	if (KnifeRotCurve != nullptr) {
		KnifeRotTimeline = NewObject<UTimelineComponent>(this, FName("Knife Rotation Timeline"));
		KnifeRotTimeline->CreationMethod = EComponentCreationMethod::UserConstructionScript;
		this->BlueprintCreatedComponents.Add(KnifeRotTimeline);
		KnifeRotTimeline->SetNetAddressable();

		KnifeRotTimeline->SetPropertySetObject(this);
		KnifeRotTimeline->SetDirectionPropertyName(FName("KnifeRotTimelineDirection"));

		KnifeRotTimeline->SetLooping(true);
		KnifeRotTimeline->SetTimelineLength(1.0f);
		KnifeRotTimeline->SetTimelineLengthMode(ETimelineLengthMode::TL_LastKeyFrame);

		KnifeRotTimeline->SetPlaybackPosition(0.0f, false);

		onKnifeRotTimelineCallback.BindUFunction(this, { FName("KnifeRotTimelineCallback") });
		onKnifeRotTimelineFinishedCallback.BindUFunction(this, { FName("KnifeRotTimelineFinishedCallback") });
		KnifeRotTimeline->AddInterpFloat(KnifeRotCurve, onKnifeRotTimelineCallback);
		KnifeRotTimeline->SetTimelineFinishedFunc(onKnifeRotTimelineFinishedCallback);

		KnifeRotTimeline->RegisterComponent();
	}

	if (KnifeThrowTraceCurve != nullptr) {
		KnifeThrowTraceTimeline = NewObject<UTimelineComponent>(this, FName("Knife ThrowTraceation Timeline"));
		KnifeThrowTraceTimeline->CreationMethod = EComponentCreationMethod::UserConstructionScript;
		this->BlueprintCreatedComponents.Add(KnifeThrowTraceTimeline);
		KnifeThrowTraceTimeline->SetNetAddressable();

		KnifeThrowTraceTimeline->SetPropertySetObject(this);
		KnifeThrowTraceTimeline->SetDirectionPropertyName(FName("KnifeThrowTraceTimelineDirection"));

		KnifeThrowTraceTimeline->SetLooping(true);
		KnifeThrowTraceTimeline->SetTimelineLength(5.0f);
		KnifeThrowTraceTimeline->SetTimelineLengthMode(ETimelineLengthMode::TL_LastKeyFrame);

		KnifeThrowTraceTimeline->SetPlaybackPosition(0.0f, false);

		onKnifeThrowTraceTimelineCallback.BindUFunction(this, { FName("KnifeThrowTraceTimelineCallback") });
		onKnifeThrowTraceTimelineFinishedCallback.BindUFunction(this, { FName("KnifeThrowTraceTimelineFinishedCallback") });
		KnifeThrowTraceTimeline->AddInterpFloat(KnifeThrowTraceCurve, onKnifeThrowTraceTimelineCallback);
		KnifeThrowTraceTimeline->SetTimelineFinishedFunc(onKnifeThrowTraceTimelineFinishedCallback);

		KnifeThrowTraceTimeline->RegisterComponent();
	}
}

// Called every frame
void AKnife::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (KnifeRotTimeline != nullptr) {
		KnifeRotTimeline->TickComponent(DeltaTime, ELevelTick::LEVELTICK_TimeOnly, NULL);
	}

}

void AKnife::Throw(FRotator CameraRotation, FVector ThrowDirectionVector, FVector CameraLocation, float ThrowSpeed)
{
	CameraStartRotation = CameraRotation;
	ThrowDirection = ThrowDirectionVector;
	CameraLocationAtThrow = CameraLocation;
	KnifeThrowSpeed = ThrowSpeed;
	SnapKnifeToStartPosition(FRotator(CameraStartRotation.Pitch, CameraStartRotation.Yaw, CameraStartRotation.Roll + KnifeSpinAxisOffset), ThrowDirection, CameraLocationAtThrow);
	LaunchKnife();
}

void AKnife::SnapKnifeToStartPosition(FRotator StartRotation, FVector ThrowDirectionVector, FVector CameraLocation)
{
	this->SetActorLocationAndRotation((CameraLocation + 250.0f * ThrowDirectionVector) - (PivotPoint->RelativeLocation), StartRotation, false, nullptr, ETeleportType::None);
}

void AKnife::LaunchKnife() {
	ProjectileMovementVar->Velocity = ThrowDirection * KnifeThrowSpeed;
	ProjectileMovementVar->Activate();
	StartKnifeRotForward();
	KnifeState = EKnifeState::VE_Launched;
	ProjectileMovementVar->ProjectileGravityScale = 0.0f;
	if (KnifeThrowTraceTimeline != NULL) {
		KnifeThrowTraceTimeline->PlayFromStart();
	}
}

void AKnife::StartKnifeRotForward() {
	if (KnifeRotTimeline != NULL) {
		KnifeRotTimeline->SetPlayRate(KnifeSpinRate);
		KnifeRotTimeline->PlayFromStart();
	}
}

void AKnife::KnifeRotTimelineCallback(float val) {
	PivotPoint->SetRelativeRotation(FRotator(val * -360.0f, 0.0f, 0.0f));
}

void AKnife::KnifeRotTimelineFinishedCallback() {
	// nothing
}

void AKnife::KnifeThrowTraceTimelineCallback(float val) {
	// TODO: IMPLEMENT KNIFE THROW LOOP
}

void AKnife::KnifeThrowTraceTimelineFinishedCallback() {
	// TODO: IMPLEMENT KNIFE STOP
}