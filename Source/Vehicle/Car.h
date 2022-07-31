// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "Car.generated.h"

USTRUCT(BlueprintType)
struct VEHICLE_API FWheel{

	GENERATED_USTRUCT_BODY()

	UPROPERTY(BlueprintReadWrite)
	float Radius = 35.0f;

	UPROPERTY(BlueprintReadWrite)
	float Mass = 15.0f;
};

USTRUCT(BlueprintType)
struct VEHICLE_API FSuspension{

	GENERATED_USTRUCT_BODY()
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Rest_length = 50.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Travel = 10.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Stifness = 300.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Damping = 10.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Force_min = -3000.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Force_max = 5000.f;
};

UCLASS()
class VEHICLE_API ACar : public APawn
{
	GENERATED_BODY()

public:
	// Sets default values for this pawn's properties
	ACar();

	UPROPERTY(EditAnywhere, Category = "Properties")
	class UStaticMeshComponent* MeshComp;

	UPROPERTY(EditAnywhere, Category = "Properties")
	class USpringArmComponent* SpringArm;

	UPROPERTY(EditAnywhere, Category = "Properties")
	class UCameraComponent* Camera;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Properties")
	TArray<class USceneComponent*> Links;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Properties")
	TArray<class UStaticMeshComponent*> Wheels;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Properties")
	TArray<float> MaxSuspensionLength;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Properties")
	TArray<FSuspension> S_Car;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Properties")
	TArray<FWheel> W_Car;

	TArray<bool>WheelHit;
	TArray<FHitResult> HitArray;
	TArray<float> SpringLength;
	TArray<float> SpringForce;
	TArray<float> LastLength;
	TArray<FVector> WheelLS;
	TArray<float>FrictionForceY; //dynamic friction force
	TArray<float>FrictionForceX; //dynamic friction force

	float Deltatime;
	float zRot = 0.f;
	float yRot = 0.f;
	float SteeringAngle = 0.f;
	float bIsDebug = false;
	/*UPROPERTY(EditAnywhere, Category = "Properties")
	class USceneComponent* TopLink_FL;
	UPROPERTY(EditAnywhere, Category = "Properties")
	class USceneComponent* TopLink_FR;
	UPROPERTY(EditAnywhere, Category = "Properties")
	class USceneComponent* TopLink_RL;
	UPROPERTY(EditAnywhere, Category = "Properties")
	class USceneComponent* TopLink_RR;*/

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	// Called every frame
	virtual void Tick(float DeltaTime) override;
	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	void Turn(float Value);

	void LookUp(float Value);

	void Forward(float Value);

	void Right(float Value);

	void UpdateSpringLength();

	void UpdateSpringForce();

	void UpdateSteeringForce();

	void RotateWheel();

	void Debug();

	void OnDebug();
};
