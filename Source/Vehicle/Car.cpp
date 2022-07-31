// Fill out your copyright notice in the Description page of Project Settings.


#include "Car.h"
#include "GameFramework/SpringArmComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SceneComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "Camera/CameraComponent.h"
#include "Math/UnrealMathUtility.h"
#include "DrawDebugHelpers.h"
#include <vector>


// Sets default values
ACar::ACar()
{
 	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	MeshComp = CreateDefaultSubobject<UStaticMeshComponent>(FName("Mesh"));
	SetRootComponent(MeshComp);

	SpringArm = CreateDefaultSubobject<USpringArmComponent>(FName("SpringArm"));
	SpringArm->SetupAttachment(RootComponent);

	Camera = CreateDefaultSubobject<UCameraComponent>(FName("Camera"));
	Camera->SetupAttachment(SpringArm);


	/*TopLink_FL = CreateDefaultSubobject<USceneComponent>(FName("FL"));
	TopLink_FL->SetupAttachment(RootComponent);
	TopLink_FR = CreateDefaultSubobject<USceneComponent>(FName("FR"));
	TopLink_FR->SetupAttachment(RootComponent);
	TopLink_RL = CreateDefaultSubobject<USceneComponent>(FName("RL"));
	TopLink_RL->SetupAttachment(RootComponent);
	TopLink_RR = CreateDefaultSubobject<USceneComponent>(FName("RR"));
	TopLink_RR->SetupAttachment(RootComponent);*/
	SteeringAngle = 0.f;
}

// Called when the game starts or when spawned
void ACar::BeginPlay()
{
	Super::BeginPlay();
	
	if (Links.Num())
	{	
		int32 n = Links.Num();
		MaxSuspensionLength.SetNum(n);
		S_Car.SetNum(n);
		W_Car.SetNum(n);
		WheelHit.SetNum(n);
		HitArray.SetNum(n);
		SpringLength.SetNum(n);
		SpringForce.SetNum(n);
		LastLength.SetNum(n);
		WheelLS.SetNum(n);
		FrictionForceY.SetNum(n);
		FrictionForceX.SetNum(n);
		//UE_LOG(LogTemp, Warning, TEXT("SpringLength: %d"), SpringLength.Num());
		for (int i = 0; i < n; i++)
		{	
			MaxSuspensionLength[i] = S_Car[i].Rest_length + S_Car[i].Travel + W_Car[i].Radius;
			SpringLength[i] = S_Car[i].Rest_length;
			LastLength[i] = S_Car[i].Rest_length;
			//UE_LOG(LogTemp, Warning, TEXT("Max Length: %f"), S_Car[i].Length_max);
			//UE_LOG(LogTemp, Warning, TEXT("Radius: %f"), W_Car[i].Radius);
			//UE_LOG(LogTemp, Warning, TEXT("Suspension Length: %f"), MaxSuspensionLength[i]);
		}
	}
}


// Called every frame
void ACar::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	Deltatime = DeltaTime;

	UpdateSpringLength();
	UpdateSpringForce();
	UpdateSteeringForce();
	RotateWheel();
	OnDebug();
}

// Called to bind functionality to input
void ACar::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
	check(PlayerInputComponent);

	PlayerInputComponent->BindAxis("LookUp", this, &ACar::LookUp);
	PlayerInputComponent->BindAxis("Turn", this, &ACar::Turn);

	PlayerInputComponent->BindAxis("Forward", this, &ACar::Forward);
	PlayerInputComponent->BindAxis("Right", this, &ACar::Right);

	PlayerInputComponent->BindAction("Debug", IE_Pressed, this, &ACar::Debug);
}

void ACar::Turn(float Value)
{
	if (Value)
	{
		zRot += Value;
		FRotator NewRot = FRotator(yRot, zRot, 0.f);
		SpringArm->SetRelativeRotation(NewRot);
	}
}

void ACar::LookUp(float Value)
{	
	if (Value)
	{
		yRot += Value;
		yRot = FMath::Clamp(yRot, -85.f, 10.f);
		FRotator NewRot = FRotator(yRot, zRot, 0.f);
		SpringArm->SetRelativeRotation(NewRot);
	}
}

void ACar::Forward(float Value)
{
	return;
}

void ACar::Debug()
{
	bIsDebug = !bIsDebug;
}

void ACar::OnDebug()
{	
	
	if (bIsDebug)
	{	
		MeshComp->SetVisibility(!bIsDebug);
		for (int i = 0; i < 4; i++)
		{
			FVector Start = Wheels[i]->GetComponentLocation();
			FVector EndZ = Start + Links[i]->GetUpVector() * (SpringLength[i]/1.f);
			FVector EndY = Start + -Links[i]->GetRightVector() * (FrictionForceX[i] / 20.f);
			FVector EndX = Start + Links[i]->GetForwardVector() * (FrictionForceY[i] / 20.f);
			DrawDebugLine(GetWorld(), Start, EndZ, FColor::Blue, false, 0.f, 0.f, 2.f);
			DrawDebugLine(GetWorld(), Start, EndY, FColor::Green, false, 0.f, 0.f, 2.f);
			DrawDebugLine(GetWorld(), Start, EndX, FColor::Red, false, 0.f, 0.f, 2.f);
		}
		
	}
	else
	{
		MeshComp->SetVisibility(!bIsDebug);
	}
}

void ACar::Right(float Value)
{	
	if (Links.Num())
	{
		if (Value)
		{
			SteeringAngle += Value;
			SteeringAngle = FMath::Clamp(SteeringAngle, -45.f, 45.f);
		}
		else 
		{
			SteeringAngle *= 0.9f;
		}
		FRotator NewRot = FRotator(0.f, SteeringAngle, 0.f);
		Links[0]->SetRelativeRotation(NewRot);
		Links[1]->SetRelativeRotation(NewRot);
	}
}


void ACar::UpdateSpringLength()
{
	if (SpringLength.Num()) // not needed
	{
		for (int i = 0; i < 4; i++)
		{
			FHitResult Hit;
			FVector Start{ Links[i]->GetComponentLocation() };
			FVector End{ Start - Links[i]->GetUpVector() * MaxSuspensionLength[i] };
			FCollisionQueryParams CollisionQueryParams;
			CollisionQueryParams.AddIgnoredActor(this);
			GetWorld()->LineTraceSingleByChannel(OUT Hit, Start, End, ECC_Camera, CollisionQueryParams);
			WheelHit[i] = Hit.bBlockingHit;
			HitArray[i] = Hit;
			if (!WheelHit[i])
			{
				SpringLength[i] = S_Car[i].Rest_length + S_Car[i].Travel;
			}
			else
			{
				float Min = S_Car[i].Rest_length - S_Car[i].Travel;
				float Max = S_Car[i].Rest_length + S_Car[i].Travel;
				FVector CurrentLength = Links[i]->GetComponentLocation() - (Hit.Location + Links[i]->GetUpVector() * W_Car[i].Radius);
				SpringLength[i] = FMath::Clamp(CurrentLength.Size(), Min, Max); //calc spring length
			}
		}
	}
	
}

void ACar::UpdateSpringForce()
{
	for (int i = 0; i < 4; i++)
	{
		float Min = S_Car[i].Force_min;
		float Max = S_Car[i].Force_max;
		float DampingForce = S_Car[i].Damping * (LastLength[i] - SpringLength[i]) / Deltatime; //calc damping force
		float CurrentForce = S_Car[i].Stifness * (S_Car[i].Rest_length - SpringLength[i]) + DampingForce; //calc spring force
		SpringForce[i] = FMath::Clamp(CurrentForce, Min, Max);
		LastLength[i] = SpringLength[i];			//update last length of the spring
		MeshComp->AddForceAtLocation(SpringForce[i] * 100.f * Links[i]->GetUpVector(), Links[i]->GetComponentLocation());
	}
}

void ACar::UpdateSteeringForce()
{
	for (int i = 0; i < 4; i++)
	{
		FVector NewWheelLocation = FVector(0.f, 0.f, -(SpringLength[i] + 3.f));
		Wheels[i]->SetRelativeLocation(NewWheelLocation);

		FVector Point = Wheels[i]->GetComponentLocation() - Links[i]->GetUpVector() * W_Car[i].Radius; //we can take any point other than this!
		FVector PointVelocity = MeshComp->GetPhysicsLinearVelocityAtPoint(Point);
		FTransform LinkTransform = UKismetMathLibrary::InvertTransform(Links[i]->GetComponentTransform());
		WheelLS[i] = UKismetMathLibrary::TransformDirection(LinkTransform, PointVelocity);

		if (WheelHit[i])
		{
			FrictionForceX[i] = GetInputAxisValue("Forward") * SpringForce[i] * 0.5f;
			float Min = -SpringForce[i];
			float Max = SpringForce[i];
			FrictionForceY[i] = WheelLS[i].X * SpringForce[i] * -1;
			FrictionForceY[i] = FMath::Clamp(FrictionForceY[i], Min, Max);
		}
		else
		{
			FrictionForceX[i] = 0.f;
			FrictionForceY[i] = 0.f;
		}

		FVector Force = -Links[i]->GetRightVector() * FrictionForceX[i] + Links[i]->GetForwardVector() * FrictionForceY[i];
		MeshComp->AddForceAtLocation(Force * 100.f, HitArray[i].Location);
	}
}

void ACar::RotateWheel()
{
	for (int i = 0; i < 4; i++)
	{
		float AngularVelocity = (-WheelLS[i].Y / W_Car[i].Radius) * Deltatime * (180.f / 3.14f);
		UE_LOG(LogTemp, Warning, TEXT("av: %f"), AngularVelocity);
		//float z = Wheels[i]->GetComponentRotation().Yaw;
		FRotator NewRotation = FRotator(AngularVelocity , 0.f, 0.f);
		Wheels[i]->AddLocalRotation(NewRotation);
	}
}
