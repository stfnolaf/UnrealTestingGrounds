// Fill out your copyright notice in the Description page of Project Settings.


#include "Knife.h"
#include "Corvo.h"

// Sets default values
AKnife::AKnife()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

bool AKnife::InitializeOwner(AActor* actor) {
	Owner = Cast<ACorvo>(actor);
	if (Owner == nullptr) {
		return false;
	}
	return true;
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

		KnifeRotTimeline->SetLooping(true);
		KnifeRotTimeline->SetTimelineLength(0.66f);
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

		KnifeThrowTraceTimeline->SetLooping(true);
		KnifeThrowTraceTimeline->SetTimelineLength(1.2f);
		KnifeThrowTraceTimeline->SetTimelineLengthMode(ETimelineLengthMode::TL_LastKeyFrame);

		KnifeThrowTraceTimeline->SetPlaybackPosition(0.0f, false);

		onKnifeThrowTraceTimelineCallback.BindUFunction(this, { FName("KnifeThrowTraceTimelineCallback") });
		onKnifeThrowTraceTimelineFinishedCallback.BindUFunction(this, { FName("KnifeThrowTraceTimelineFinishedCallback") });
		KnifeThrowTraceTimeline->AddInterpFloat(KnifeThrowTraceCurve, onKnifeThrowTraceTimelineCallback);
		KnifeThrowTraceTimeline->SetTimelineFinishedFunc(onKnifeThrowTraceTimelineFinishedCallback);

		KnifeThrowTraceTimeline->RegisterComponent();
	}

	//UGameplayStatics::SetGlobalTimeDilation(GetWorld(), 0.25f);
}

// Called every frame
void AKnife::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (KnifeRotTimeline != nullptr) {
		KnifeRotTimeline->TickComponent(DeltaTime, ELevelTick::LEVELTICK_TimeOnly, NULL);
	}

	if (KnifeThrowTraceTimeline != nullptr) {
		KnifeThrowTraceTimeline->TickComponent(DeltaTime, ELevelTick::LEVELTICK_TimeOnly, NULL);
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
	//this->SetActorLocationAndRotation((CameraLocation + 250.0f * ThrowDirectionVector) - (PivotPoint->GetRelativeLocation()), StartRotation, false, nullptr, ETeleportType::None);
	this->SetActorRotation(StartRotation);
}

void AKnife::LaunchKnife() {
	ProjectileMovementVar->Velocity = ThrowDirection * KnifeThrowSpeed;
	ProjectileMovementVar->Activate();
	//StartKnifeRotForward();
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

void AKnife::LodgeKnife() {
	StopKnifeMoving();
	PivotPoint->SetRelativeRotation(FRotator::ZeroRotator);
	SetActorRotation(CameraStartRotation);
	LodgePoint->SetRelativeRotation(FRotator(AdjustKnifeImpactPitch(UKismetMathLibrary::RandomFloatInRange(-30.0f, -55.0f), UKismetMathLibrary::RandomFloatInRange(-5.0f, -25.0f)), 0.0f, UKismetMathLibrary::RandomFloatInRange(-3.0f, -8.0f)));
	SetActorLocation(AdjustKnifeImpactLocation());
	KnifeState = EKnifeState::VE_LodgedInSomething;
}

FVector AKnife::AdjustKnifeImpactLocation() {
	float pitch = UKismetMathLibrary::MakeRotationFromAxes(ImpactNormal, FVector::ZeroVector, FVector::ZeroVector).Pitch;
	ZAdjustment = (pitch > 0.0f ? pitch : 0.0f) - 90.0f;
	ZAdjustment /= 90.0f;
	ZAdjustment *= 10.0f;
	return ImpactLocation /*+ FVector(0.0f, 0.0f, ZAdjustment)*/ + (GetActorLocation() - LodgePoint->GetComponentLocation());
}

float AKnife::AdjustKnifeImpactPitch(float InclinedSurfaceRange, float RegularSurfaceRange) {
	float pitch = UKismetMathLibrary::MakeRotationFromAxes(ImpactNormal, FVector::ZeroVector, FVector::ZeroVector).Pitch;
	return (pitch > 0.0f ? RegularSurfaceRange : InclinedSurfaceRange) - pitch;
}

void AKnife::StopKnifeMoving() {
	ProjectileMovementVar->Deactivate();
	check(KnifeRotTimeline);
	KnifeRotTimeline->Stop();
}

void AKnife::KnifeRotTimelineCallback(float val) {
	PivotPoint->SetRelativeRotation(FRotator(val * -360.0f, 0.0f, 0.0f));
}

void AKnife::KnifeRotTimelineFinishedCallback() {
	// nothing
}

void AKnife::KnifeThrowTraceTimelineCallback(float val) {
	// TODO: IMPLEMENT KNIFE THROW LOOP
	ProjectileMovementVar->ProjectileGravityScale = val;
	FHitResult knifeHit;
	TArray<AActor*> Actors;
	FVector vel = GetVelocity();
	UKismetMathLibrary::Vector_Normalize(vel, 0.0001f);
	if (UKismetSystemLibrary::LineTraceSingle(GetWorld(), GetActorLocation(), GetActorLocation() + (vel * KnifeThrowTraceDistance), ETraceTypeQuery::TraceTypeQuery1, false, Actors, EDrawDebugTrace::None, knifeHit, true, FLinearColor::Red, FLinearColor::Green, 0.0f)) {
		if (knifeHit.bBlockingHit) {
			ImpactLocation = knifeHit.ImpactPoint;
			ImpactNormal = knifeHit.ImpactNormal;
			HitBoneName = knifeHit.BoneName;
			if (knifeHit.PhysMaterial.IsValid()) {
				UPhysicalMaterial* temp = knifeHit.PhysMaterial.Get();
				SurfaceType = temp->SurfaceType;
				// if hit actor can be cast to a destructible, handle destruction
				// else
				check(KnifeThrowTraceTimeline);
				KnifeThrowTraceTimeline->Stop();
				ProjectileMovementVar->Deactivate();
				// if hit actor can be cast to an AI, handle hit
				// else
				LodgeKnife();
			}

		}
	}
}

void AKnife::KnifeThrowTraceTimelineFinishedCallback() {
	// TODO: IMPLEMENT KNIFE STOP
}

void AKnife::Recall() {
	if (ReturnPathClass) {
		UWorld* world = GetWorld();
		if (world) {
			FActorSpawnParameters spawnParams;
			spawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
			//spawnParams.Owner = this;
			FRotator rotator = FRotator::ZeroRotator;
			FVector spawnLocation = GetActorLocation();
			ReturnPath = world->SpawnActor<AWeaponReturnPath>(ReturnPathClass, spawnLocation, rotator, spawnParams);
			ReturnPath->GetSpline()->ClearSplinePoints();
			UE_LOG(LogTemp, Warning, TEXT("spawned return path with %d points"), ReturnPath->GetSpline()->GetNumberOfSplinePoints());
		}
	}
	/*KnifeThrowTraceTimeline->Stop();
	KnifeMeshVar->SetVisibility(true, false);
	ZAdjustment = 10.0f;
	KnifeState = EKnifeState::VE_Returning;
	DistanceFromCharacter = GetClampedKnifeDistanceFromCharacter(MaxCalculationDistance);
	AdjustKnifeReturnLocation();
	InitialLocation = GetActorLocation();
	InitialRotation = GetActorRotation();
	CameraStartRotation = Owner->GetCamera()->GetComponentRotation();
	LodgePoint->SetRelativeLocation(FVector::ZeroVector);*/
}

float AKnife::GetClampedKnifeDistanceFromCharacter(float maxDist) {
	check(Owner);
	return UKismetMathLibrary::FClamp(UKismetMathLibrary::Vector_Distance(GetActorLocation(), Owner->GetMyMesh()->GetSocketLocation(FName("knife_socket"))), 0.0f, maxDist);
}

void AKnife::AdjustKnifeReturnLocation() {
	SetActorLocation(GetActorLocation() + FVector(0.0f, 0.0f, (((ZAdjustment / 10.0f) - 1.0f) * 30.0f) + 20.0f));
}