// Fill out your copyright notice in the Description page of Project Settings.


#include "Knife.h"
#include "Corvo.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "WallRunning.h"
#include "DrawDebugHelpers.h"

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

	FOnTimelineFloat onKnifeReturnTraceTimelineCallback;
	FOnTimelineEventStatic onKnifeReturnTraceTimelineFinishedCallback;

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
		KnifeThrowTraceTimeline = NewObject<UTimelineComponent>(this, FName("Knife Throw Trace Timeline"));
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

	if (KnifeReturnTraceCurve != nullptr) {
		KnifeReturnTraceTimeline = NewObject<UTimelineComponent>(this, FName("Knife Return Trace Timeline"));
		KnifeReturnTraceTimeline->CreationMethod = EComponentCreationMethod::UserConstructionScript;
		this->BlueprintCreatedComponents.Add(KnifeReturnTraceTimeline);
		KnifeReturnTraceTimeline->SetNetAddressable();

		KnifeReturnTraceTimeline->SetLooping(true);
		KnifeReturnTraceTimeline->SetTimelineLength(1.2f);
		KnifeReturnTraceTimeline->SetTimelineLengthMode(ETimelineLengthMode::TL_LastKeyFrame);

		KnifeReturnTraceTimeline->SetPlaybackPosition(0.0f, false);

		onKnifeReturnTraceTimelineCallback.BindUFunction(this, { FName("KnifeReturnTraceTimelineCallback") });
		onKnifeReturnTraceTimelineFinishedCallback.BindUFunction(this, { FName("KnifeReturnTraceTimelineFinishedCallback") });
		KnifeReturnTraceTimeline->AddInterpFloat(KnifeReturnTraceCurve, onKnifeReturnTraceTimelineCallback);
		KnifeReturnTraceTimeline->SetTimelineFinishedFunc(onKnifeReturnTraceTimelineFinishedCallback);

		KnifeReturnTraceTimeline->RegisterComponent();
	}

	//UGameplayStatics::SetGlobalTimeDilation(GetWorld(), 0.25f);
}

// Called every frame
void AKnife::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	this->DeltaTime = DeltaTime;

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

void AKnife::LodgeKnife(bool permalodge) {
	StopKnifeMoving();
	PivotPoint->SetRelativeRotation(FRotator::ZeroRotator);
	SetActorRotation(CameraStartRotation.Add(0.0f, 0.0f, -75.0f));
	//LodgePoint->SetRelativeRotation(FRotator(AdjustKnifeImpactPitch(UKismetMathLibrary::RandomFloatInRange(-30.0f, -55.0f), UKismetMathLibrary::RandomFloatInRange(-5.0f, -25.0f)), 0.0f, UKismetMathLibrary::RandomFloatInRange(-3.0f, -8.0f)));
	SetActorLocation(AdjustKnifeImpactLocation());
	if (!permalodge)
		KnifeState = EKnifeState::VE_LodgedInSomething;
	else
		KnifeState = EKnifeState::VE_PermaLodged;
}

FVector AKnife::AdjustKnifeImpactLocation() {
	return ImpactLocation + (GetRootComponent()->GetComponentLocation() - LodgePoint->GetComponentLocation());
}

void AKnife::StopKnifeMoving() {
	KnifeThrowTraceTimeline->Stop();
	ProjectileMovementVar->Deactivate();
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
			//DrawDebugLine(GetWorld(), Owner->GetActorLocation(), Owner->GetActorLocation() + (vel * KnifeThrowTraceDistance), FColor::Red, true, 100.0f, (uint8)'\000', 1.0f);
			ImpactLocation = knifeHit.ImpactPoint;
			ImpactNormal = knifeHit.ImpactNormal;
			HitBoneName = knifeHit.BoneName;
			if (knifeHit.PhysMaterial.IsValid()) {
				UPhysicalMaterial* temp = knifeHit.PhysMaterial.Get();
				SurfaceType = temp->SurfaceType;
				// if hit actor can be cast to a destructible, handle destruction
				// else
				// if hit actor can be cast to an AI, handle hit
				// else
				LodgeKnife(false);
			}

		}
	}
}

void AKnife::KnifeThrowTraceTimelineFinishedCallback() {
	// TODO: IMPLEMENT KNIFE STOP
}

bool AKnife::Recall() {
	if (ReturnPathClass && KnifeState != EKnifeState::VE_Idle && KnifeState != EKnifeState::VE_Returning) {
		UWorld* world = GetWorld();
		if (world) {
			if (KnifeState == EKnifeState::VE_Launched)
				StopKnifeMoving();
			KnifeState = EKnifeState::VE_Returning;
			FActorSpawnParameters spawnParams;
			spawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
			FRotator rotator = FRotator::ZeroRotator;
			FVector spawnLocation = FVector::ZeroVector;
			if (ReturnPath == nullptr) {
				ReturnPath = world->SpawnActor<AWeaponReturnPath>(ReturnPathClass, spawnLocation, rotator, spawnParams);
			}
			ReturnPath->SetKnifeOwnerAndTarget(this, Owner);
			ReturnPath->UpdatePath(Owner->GetMyMesh()->GetSocketRotation(FName("knife_socket")));
			TimeSinceRecall = 0.0f;
			ReturnSpeed = 500.0f;
			LodgePoint->SetRelativeRotation(FRotator::ZeroRotator);
			KnifeReturnTraceTimeline->PlayFromStart();
			return true;
		}
		return false;
	}
	return false;
}

EKnifeState AKnife::GetKnifeState() {
	return KnifeState;
}

float AKnife::GetClampedKnifeDistanceFromCharacter(float maxDist) {
	check(Owner);
	return UKismetMathLibrary::FClamp(UKismetMathLibrary::Vector_Distance(GetActorLocation(), Owner->GetMyMesh()->GetSocketLocation(FName("knife_socket"))), 0.0f, maxDist);
}

void AKnife::AdjustKnifeReturnLocation() {
	SetActorLocation(GetActorLocation() + FVector(0.0f, 0.0f, (((ZAdjustment / 10.0f) - 1.0f) * 30.0f) + 20.0f));
}

void AKnife::KnifeReturnTraceTimelineCallback(float val) {
	Owner->UpdateDeltaYawBetweenPlayerAndKnife();
	FRotator SocketRotation = Owner->GetMyMesh()->GetSocketRotation(FName("knife_socket"));
	ReturnPath->UpdatePath(SocketRotation);
	TimeSinceRecall += DeltaTime;
	SetActorLocation(ReturnPath->GetSpline()->GetLocationAtDistanceAlongSpline(TimeSinceRecall * ReturnSpeed, ESplineCoordinateSpace::World), true, nullptr, ETeleportType::None);
	float tValue = FMath::Clamp((TimeSinceRecall * ReturnSpeed) / ReturnPath->GetSpline()->GetSplineLength(), 0.0f, 1.0f);
	float TimeUntilCatch = (TimeSinceRecall / tValue) - TimeSinceRecall;
	if (tValue < 0.75f) {
		PivotPoint->SetRelativeRotation(FRotator(TimeSinceRecall * 360.0f, 0.0f, 0.0f));
	}
	else {
		float CurrRot = FMath::Fmod(PivotPoint->GetRelativeRotation().Pitch, 360.0f);
		float RemainingRotation = 360.0f - CurrRot;
		PivotPoint->SetRelativeRotation(FMath::RInterpTo(PivotPoint->GetRelativeRotation(), FRotator::ZeroRotator, DeltaTime, (RemainingRotation / TimeUntilCatch)));
	}
	float AngleDifference = FMath::Acos(FVector::DotProduct(GetActorRotation().Vector(), SocketRotation.Vector()));
	SetActorRotation(FMath::RInterpTo(GetActorRotation(), SocketRotation, DeltaTime, (AngleDifference / TimeUntilCatch)));
	ReturnSpeed = ReturnPath->UpdateReturnSpeed(TimeSinceRecall * ReturnSpeed);
	if (FVector::Dist(Owner->GetMyMesh()->GetSocketLocation(FName("knife_socket")), GetActorLocation()) < 5.0f) {
		HandleKnifeReturn();
	}
}

void AKnife::KnifeReturnTraceTimelineFinishedCallback() {
	//TODO: Knife returns to hand
	UE_LOG(LogTemp, Warning, TEXT("knife return ended"))
}

void AKnife::HandleKnifeReturn() {
	if (KnifeState == EKnifeState::VE_Returning) {
		KnifeState = EKnifeState::VE_Idle;
		KnifeReturnTraceTimeline->Stop();
		Owner->EndWaitForKnife();
	}
}