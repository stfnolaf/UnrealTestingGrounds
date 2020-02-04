// Fill out your copyright notice in the Description page of Project Settings.


#include "WeaponReturnPath.h"
#include "Knife.h"
#include "Corvo.h"
#include "Components/SplineComponent.h"
#include "DrawDebugHelpers.h"

// Sets default values
AWeaponReturnPath::AWeaponReturnPath()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	Spline = CreateDefaultSubobject<USplineComponent>(TEXT("Spline"));
	RootComponent = Spline;
}

// Called when the game starts or when spawned
void AWeaponReturnPath::BeginPlay()
{
	Super::BeginPlay();
	Spline->SetDefaultUpVector(FVector::UpVector, ESplineCoordinateSpace::World);
	Spline->SetSplinePointType(1, ESplinePointType::CurveCustomTangent, true);
	
}

// Called every frame
void AWeaponReturnPath::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

USplineComponent* AWeaponReturnPath::GetSpline() {
	return Spline;
}

void AWeaponReturnPath::SetKnifeOwnerAndTarget(AKnife* weapon, ACorvo* owner) {
	Weapon = weapon;
	Target = owner;
	InitialDistToTarget = FVector::Dist(Target->GetMyMesh()->GetSocketLocation(FName("knife_socket")), Weapon->GetActorLocation());
	Spline->SetWorldLocationAtSplinePoint(0, Weapon->GetActorLocation());
}

float AWeaponReturnPath::UpdateReturnSpeed(float distance) {
	return ((distance / Spline->GetSplineLength()) * (Weapon->MaxReturnSpeed - Weapon->MinReturnSpeed)) + Weapon->MinReturnSpeed;
}

float AWeaponReturnPath::EaseIn(float alpha) {
	return 1.0f - FMath::Sqrt(1.0f - FMath::Pow(FMath::Clamp(alpha, 0.0f, 1.0f), 2.0f));
}

void AWeaponReturnPath::UpdatePath(FRotator TargetRotation) {
	if (Weapon != nullptr && Target != nullptr) {
		Spline->SetWorldLocationAtSplinePoint(1, Target->GetMyMesh()->GetSocketLocation(FName("knife_socket")));
		Spline->SetTangentAtSplinePoint(1, (TargetRotation + FRotator(0.0f, 0.0f, 45.0f)).RotateVector(FVector::DownVector * InitialDistToTarget * -0.5f), ESplineCoordinateSpace::World, true);
		//DRAWS LINE EXTENDING FROM CENTER OF PALM
		/*DrawDebugLine(GetWorld(), Target->GetMyMesh()->GetSocketLocation(FName("knife_socket")),
			Target->GetMyMesh()->GetSocketLocation(FName("knife_socket")) + (rot + FRotator(0.0f, 0.0f, 55.0f)).RotateVector(FVector::DownVector * InitialDistToTarget * 0.5f),
			FColor::Red, false, 10.0f, (uint8)'\000', 2.0f);*/
	}
}