// Fill out your copyright notice in the Description page of Project Settings.


#include "WeaponReturnPath.h"
#include "Knife.h"
#include "Corvo.h"
#include "Components/SplineComponent.h"

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
	
}

// Called every frame
void AWeaponReturnPath::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

USplineComponent* AWeaponReturnPath::GetSpline() {
	return Spline;
}

void AWeaponReturnPath::SetKnifeOwner(AKnife* owner) {
	Weapon = owner;
}

void AWeaponReturnPath::UpdatePath() {
	if (Weapon != nullptr && Target != nullptr) {
		Spline->SetWorldLocationAtSplinePoint(0, Weapon->GetActorLocation());
		Spline->SetWorldLocationAtSplinePoint(1, Target->GetMyMesh()->GetSocketLocation(FName("knife_socket")));
		Spline->SetTangentAtSplinePoint(1, Target->GetCamera()->GetForwardVector() * -200.0f, ESplineCoordinateSpace::World, true);
	}
}

void AWeaponReturnPath::SetTarget(ACorvo* owner) {
	Target = owner;
}