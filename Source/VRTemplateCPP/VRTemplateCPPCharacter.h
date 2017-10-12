// Copyright 1998-2016 Epic Games, Inc. All Rights Reserved.
#pragma once
#include "GameFramework/Character.h"
#include "VRTemplateCPPCharacter.generated.h"

class UAnimBlueprint;
class UInputComponent;
class USceneComponent;
class UCameraComponent;
class USkeletalMeshComponent;
class UPhysicsHandleComponent;
class UMotionControllerComponent;

UCLASS(config=Game)
class AVRTemplateCPPCharacter : public ACharacter
{
	GENERATED_BODY()

protected:
	//Character mesh: 1st person view (arms; seen only by self).
	UPROPERTY(EditAnywhere, Category = Mesh)
	USkeletalMeshComponent* myFirstPersonMesh;

	//Left Hand mesh: 1st person view (seen only by self).
	UPROPERTY(EditAnywhere, Category = Mesh)
	USkeletalMeshComponent* myFirstPersonLeftHand;

	//Right Hand mesh: 1st person view (seen only by self).
	UPROPERTY(EditAnywhere, Category = Mesh)
	USkeletalMeshComponent* myFirstPersonRightHand;

	//VR Left Hand mesh: VR view (attached to the VR controller directly, no arm, just the hand).
	UPROPERTY(EditAnywhere, Category = Mesh)
	USkeletalMeshComponent* myVRLeftHand;

	//VR Right Hand mesh: VR view (attached to the VR controller directly, no arm, just the hand).
	UPROPERTY(EditAnywhere, Category = Mesh)
	USkeletalMeshComponent* myVRRightHand;

	//First person camera attached to the character.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	UCameraComponent* myFirstPersonCamera;

	//Base turn rate, in deg/sec. Other scaling may affect final turn rate.
	UPROPERTY(EditAnywhere, Category = Camera)
	float myBaseTurnRate;

	//Base look up / down rate, in deg / sec. Other scaling may affect final rate.
	UPROPERTY(EditAnywhere, Category = Camera)
	float myBaseLookUpRate;

	//Motion controller (left hand).
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "VR", meta = (AllowPrivateAccess = "true"))
	UMotionControllerComponent* myLeftMotionController;

	//Motion controller (right hand).
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "VR", meta = (AllowPrivateAccess = "true"))
	UMotionControllerComponent* myRightMotionController;

	//Current Distance of grabbed items from their respective controllers.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "VR")
	float myDistanceFromController;

	//The farthest distance you grab extends.
	UPROPERTY(EditAnywhere, Category = "VR")
	float myReachDistance;

	//Min Distance for Controller for grabbed objects.
	UPROPERTY(EditAnywhere, Category = "VR")
	float myMinDistanceFromController;

	//Max Distance for Controller for grabbed objects.
	UPROPERTY(EditAnywhere, Category = "VR")
	float myMaxDistanceFromController;
	
	//Whether to use motion controller location for aiming.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VR")
	bool myIsUsingMotionControllers;

	//Location of the currently grabbing controller.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "VR")
	FVector myCurrControllerLocation;

	//Rotation of the currently grabbing controller.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "VR")
	FRotator myCurrControllerRotation;

	//True if the left hand is currently grabbing.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "VR")
	bool myLeftHandIsGrabbing;					

	//True if the right hand is currently grabbing.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "VR")
	bool myRightHandIsGrabbing;					

	//True if the grab is a locking type grab.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "VR")
	bool myIsLockGrab;

	//True only during actual grab action/input.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "VR")
	bool myIsJustGrabbed;						

	//The Transform of the currently grabbed object.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "VR")
	FTransform myGrabbedObjectTransform;

	//The Transform of the controller that is currently grabbing.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "VR")
	FTransform myControllerTransform;

	//Target location for grabbed object.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "VR")
	FVector myNewGrabbedLocation;				

	//Difference between previous and old grabbed object locations.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "VR")
	FVector myDiffGrabbedLocation;				

	//Difference between controller rotation and grabbed object rotation.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "VR")
	FQuat myDiffGrabbedRotation;				

	//If not NULL, the object that is currently being grabbed.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "VR")
	UPhysicsHandleComponent* myGrabbedObject;

public:
	///Constructor
	AVRTemplateCPPCharacter();

	///Virtual Override Functions
	virtual void SetupPlayerInputComponent(UInputComponent* playerInputComponent) override;
	virtual void Tick(float deltaTime) override;

	///Functions
	virtual void BeginPlay();
	virtual void OnGrabLeftHand();
	virtual void OnGrabRightHand();
	virtual void OnReleaseLeftHand();
	virtual void OnReleaseRightHand();
	virtual void Grab(float reach, bool lockGrab, bool showDebugLine, USkeletalMeshComponent* theHand);
	virtual AActor* CastRay(FVector& lineTraceStart, FVector& lineTraceEnd);
	virtual void OnResetVR();
	virtual void MoveForward(float val);
	virtual void MoveStafing(float val);
	virtual void Turn(float rate);
	virtual void LookUp(float rate);

	///Getters
	USkeletalMeshComponent* GetFirstPersonMesh();
	USkeletalMeshComponent* GetFirstPersonLeftHand();
	USkeletalMeshComponent* GetFirstPersonRightHand();
	USkeletalMeshComponent* GetVRLeftHand();
	USkeletalMeshComponent* GetVRRightHand();
	float GetBaseTurnRate();
	float GetBaseLookUpRate();
	UMotionControllerComponent* GetLeftMotionController();
	UMotionControllerComponent* GetRightMotionController();
	float GetDistanceFromController();
	float GetReachDistance();
	float GetMinDistanceFromController();
	float GetMaxDistanceFromController();
	bool GetIsUsingMotionControllers();
	FVector GetCurrControllerLocation();
	FRotator GetCurrControllerRotation();
	bool GetLeftHandIsGrabbing();
	bool GetRightHandIsGrabbing();
	bool GetIsLockGrab();
	bool GetIsJustGrabbed();
	FTransform GetGrabbedObjectTransform();
	FTransform GetControllerTransform();
	FVector GetNewGrabbedLocation();
	FVector GetDiffGrabbedLocation();
	FQuat GetDiffGrabbedRotation();
	UPhysicsHandleComponent* GetGrabbedObject();


	///Setters
	void SetFirstPersonMesh(USkeletalMeshComponent* firstPersonMesh);
	void SetFirstPersonLeftHand(USkeletalMeshComponent* firstPersonLeftHandMesh);
	void SetFirstPersonRightHand(USkeletalMeshComponent* firstPersonRightHandMesh);
	void SetVRLeftHand(USkeletalMeshComponent* vrLeftHandMesh);
	void SetVRRightHand(USkeletalMeshComponent* vrRightHandMesh);
	void SetBaseTurnRate(float baseTurnRate);
	void SetBaseLookUpRate(float baseLookUpRate);
	void SetReachDistance(float reachDistance);
	void SetMinDistanceFromController(float minDistance);
	void SetMaxDistanceFromController(float maxDistance);
	void SetIsUsingMotionControllers(bool isUsingMotionControllers);
	void SetCurrControllerLocation(FVector currControllerLocation);
	void SetCurrControllerRotation(FRotator currControllerRotation);
	void SetLeftHandIsGrabbing(bool leftHandIsGrabbing);
	void SetRightHandIsGrabbing(bool rightHandIsGrabbing);
	void SetIsLockGrab(bool isLockGrab);
	void SetIsJustGrabbed(bool isJustGrabbed);
	void SetGrabbedObjectTransform(FTransform grabbedObjectTransform);
	void SetControllerTransform(FTransform controllerTransform);
	void SetGrabbedObject(UPhysicsHandleComponent* grabbedObject);
	void SetDistanceFromController(float newDistance);
};

