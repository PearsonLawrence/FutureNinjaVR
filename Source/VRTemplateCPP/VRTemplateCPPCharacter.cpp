#include "VRTemplateCPP.h"
#include "VRTemplateCPPCharacter.h"
#include "VRTemplateCPPProjectile.h"
#include "Animation/AnimBlueprint.h"
#include "GameFramework/InputSettings.h"
#include "Kismet/HeadMountedDisplayFunctionLibrary.h"
#include "MotionControllerComponent.h"

DEFINE_LOG_CATEGORY_STATIC(LogFPChar, Warning, All);

/**
* Constructor
*/
AVRTemplateCPPCharacter::AVRTemplateCPPCharacter()
{
	//Won't be ticked by default.
	PrimaryActorTick.bCanEverTick = true; 

	//Set size for collision capsule.
	GetCapsuleComponent()->InitCapsuleSize(55.0f, 96.0f);

	//Set our turn rates for input.
	myBaseTurnRate = 45.0f;
	myBaseLookUpRate = 45.0f;

	//Create a CameraComponent.
	myFirstPersonCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FirstPersonCamera"));
	myFirstPersonCamera->SetupAttachment(GetCapsuleComponent());
	myFirstPersonCamera->RelativeLocation = FVector(-39.56f, 1.75f, 64.0f); // Position the camera
	myFirstPersonCamera->bUsePawnControlRotation = true;

	//Create a mesh component that will be used when being viewed from a '1st person' view (when controlling this pawn).
	myFirstPersonMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("FirstPersonMesh"));
	myFirstPersonMesh->SetOnlyOwnerSee(true);
	myFirstPersonMesh->SetupAttachment(myFirstPersonCamera);
	myFirstPersonMesh->bCastDynamicShadow = false;
	myFirstPersonMesh->CastShadow = false;
	myFirstPersonMesh->RelativeRotation = FRotator(1.9f, -19.19f, 5.2f);
	myFirstPersonMesh->RelativeLocation = FVector(-0.5f, -4.4f, -155.7f);

	//Create the left hand mesh component.
	myFirstPersonLeftHand = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("FirstPersonLeftHand"));
	myFirstPersonLeftHand->SetOnlyOwnerSee(true);
	myFirstPersonLeftHand->bCastDynamicShadow = false;
	myFirstPersonLeftHand->CastShadow = false;
	myFirstPersonLeftHand->SetupAttachment(RootComponent);

	//Create the right hand mesh component.
	myFirstPersonRightHand = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("FirstPersonRightHand"));
	myFirstPersonRightHand->SetOnlyOwnerSee(true);
	myFirstPersonRightHand->bCastDynamicShadow = false;
	myFirstPersonRightHand->CastShadow = false;
	myFirstPersonRightHand->SetupAttachment(RootComponent);

	//Create the left VR Controller.
	myLeftMotionController = CreateDefaultSubobject<UMotionControllerComponent>(TEXT("LeftMotionController"));
	myLeftMotionController->Hand = EControllerHand::Left;
	myLeftMotionController->SetupAttachment(RootComponent);

	//Create the right VR Controller.
	myRightMotionController = CreateDefaultSubobject<UMotionControllerComponent>(TEXT("RightMotionController"));
	myRightMotionController->Hand = EControllerHand::Right;
	myRightMotionController->SetupAttachment(RootComponent);

	//Create a hand and attach it to the left hand VR controller.
	myVRLeftHand = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("VRLeftHand"));
	myVRLeftHand->SetOnlyOwnerSee(true);			// only the owning player will see this mesh
	myVRLeftHand->bCastDynamicShadow = false;
	myVRLeftHand->CastShadow = false;
	myVRLeftHand->SetupAttachment(myLeftMotionController);

	//Create a hand and attach it to the right hand VR controller.
	myVRRightHand = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("VRRightHand"));
	myVRRightHand->SetOnlyOwnerSee(true);			// only the owning player will see this mesh
	myVRRightHand->bCastDynamicShadow = false;
	myVRRightHand->CastShadow = false;
	myVRRightHand->SetupAttachment(myRightMotionController);

	//Turn motion controllers on by default:
	myIsUsingMotionControllers = true;

	myReachDistance = 100.0f;
	myDistanceFromController = 10.0f;
	myMinDistanceFromController = 10.0f;
	myMaxDistanceFromController = 250.0f;
	myCurrControllerLocation = FVector::ZeroVector;
	myCurrControllerRotation = FRotator::ZeroRotator;
	myIsJustGrabbed = false;
	myLeftHandIsGrabbing = false;
	myRightHandIsGrabbing = false;
	myGrabbedObject = NULL;

	//Set to true if we want to use locking style grab.
	myIsLockGrab = false;
}

/**
 * Bind input keywords to functions to call when a button is pressed or released.
 *
 * @param [in] playerInputComponent If non-null, the player input component.
 */
void AVRTemplateCPPCharacter::SetupPlayerInputComponent(UInputComponent* playerInputComponent)
{
	//Setup gameplay key bindings
	check(playerInputComponent);

	playerInputComponent->BindAction("Jump", IE_Pressed, this, &ACharacter::Jump);
	playerInputComponent->BindAction("Jump", IE_Released, this, &ACharacter::StopJumping);

	playerInputComponent->BindAction("GrabLeftHand", IE_Pressed, this, &AVRTemplateCPPCharacter::OnGrabLeftHand);
	playerInputComponent->BindAction("GrabLeftHand", IE_Released, this, &AVRTemplateCPPCharacter::OnReleaseLeftHand);

	playerInputComponent->BindAction("GrabRightHand", IE_Pressed, this, &AVRTemplateCPPCharacter::OnGrabRightHand);
	playerInputComponent->BindAction("GrabRightHand", IE_Released, this, &AVRTemplateCPPCharacter::OnReleaseRightHand);

	playerInputComponent->BindAction("ResetVR", IE_Pressed, this, &AVRTemplateCPPCharacter::OnResetVR);

	playerInputComponent->BindAxis("MoveForward", this, &AVRTemplateCPPCharacter::MoveForward);
	playerInputComponent->BindAxis("MoveStafing", this, &AVRTemplateCPPCharacter::MoveStafing);

	// We have 2 versions of the rotation bindings to handle different kinds of devices differently
	// "turn" handles devices that provide an absolute delta, such as a mouse.
	// "turnrate" is for devices that we choose to treat as a rate of change, such as an analog joystick
	playerInputComponent->BindAxis("Turn", this, &APawn::AddControllerYawInput);
	playerInputComponent->BindAxis("TurnRate", this, &AVRTemplateCPPCharacter::Turn);
	playerInputComponent->BindAxis("LookUp", this, &APawn::AddControllerPitchInput);
	playerInputComponent->BindAxis("LookUpRate", this, &AVRTemplateCPPCharacter::LookUp);
}

/**
 * Gets called once every "tick" (clock cycle).
 *
 * @param deltaTime The delta time.
 */
void AVRTemplateCPPCharacter::Tick(float deltaTime)
{
	Super::Tick(deltaTime);

	if (myLeftHandIsGrabbing == true)
	{
		myCurrControllerLocation = myVRLeftHand->GetComponentLocation();
		myCurrControllerRotation = myVRLeftHand->GetComponentRotation();
		myControllerTransform = myVRLeftHand->GetComponentTransform();
	}
	if (myRightHandIsGrabbing == true)
	{
		myCurrControllerLocation = myVRRightHand->GetComponentLocation();
		myCurrControllerRotation = myVRRightHand->GetComponentRotation();
		myControllerTransform = myVRRightHand->GetComponentTransform();
	}

	//Update grabbed object location & rotation (if any)
	if (myGrabbedObject != NULL)
	{
		if (myIsLockGrab == true)
		{
			//Get grabbed object & controller transforms and place into cache (save on future calls)
			myGrabbedObjectTransform = myGrabbedObject->GetOwner()->GetRootComponent()->GetComponentTransform(); // Grabbed object's original transform

			myNewGrabbedLocation = myCurrControllerLocation + (myCurrControllerRotation.Vector() * myDistanceFromController);
			myDiffGrabbedRotation = myGrabbedObjectTransform.GetRotation() - myControllerTransform.GetRotation();

			if (myIsJustGrabbed == true) 
			{
				myDiffGrabbedLocation = myGrabbedObject->GetOwner()->GetActorLocation() - myNewGrabbedLocation;
				myIsJustGrabbed = false;
			}

			//Move grabbed object to target location and add the original offset
			myGrabbedObject->SetTargetLocation(myNewGrabbedLocation + myDiffGrabbedLocation);
			myGrabbedObjectTransform.SetRotation(myControllerTransform.GetRotation() + myDiffGrabbedRotation);
		}
		else 
		{
			//Do a regular ranged grab (snaps to 0,0,0 of grabbed object)
			myGrabbedObject->SetTargetLocation(myCurrControllerLocation + (myCurrControllerRotation.Vector() * myDistanceFromController));
			myGrabbedObject->SetTargetRotation(myCurrControllerRotation);
		}
	}
}

/**
* Called once play starts for this character.
*/
void AVRTemplateCPPCharacter::BeginPlay()
{
	// Call the base class  
	Super::BeginPlay();

	//Attach gun mesh component to Skeleton, doing it here because the skeleton is not yet created in the constructor
	myFirstPersonLeftHand->AttachToComponent(myFirstPersonMesh, FAttachmentTransformRules(EAttachmentRule::SnapToTarget, true), TEXT("GripPointLeftHand"));
	myFirstPersonRightHand->AttachToComponent(myFirstPersonMesh, FAttachmentTransformRules(EAttachmentRule::SnapToTarget, true), TEXT("GripPointRightHand"));

	// Show or hide the two versions of the gun based on whether or not we're using motion controllers.
	if (myIsUsingMotionControllers)
	{
		myVRLeftHand->SetHiddenInGame(false, true);
		myVRRightHand->SetHiddenInGame(false, true);
		myFirstPersonMesh->SetHiddenInGame(true, true);
	}
	else
	{
		myVRLeftHand->SetHiddenInGame(true, true);
		myVRRightHand->SetHiddenInGame(true, true);
		myFirstPersonMesh->SetHiddenInGame(false, true);
	}
}

/**
* Called when player grabs with the left hand.
*/
void AVRTemplateCPPCharacter::OnGrabLeftHand()
{
	myLeftHandIsGrabbing = true;

	AVRTemplateCPPCharacter::Grab(myReachDistance, false, false, myVRLeftHand);
}

/**
* Called when player grabs with the right hand.
*/
void AVRTemplateCPPCharacter::OnGrabRightHand()
{
	myRightHandIsGrabbing = true;

	AVRTemplateCPPCharacter::Grab(myReachDistance, false, false, myVRRightHand);
}

/**
* Called when player releases with the left hand.
*/
void AVRTemplateCPPCharacter::OnReleaseLeftHand()
{
	myLeftHandIsGrabbing = false;

	if (myGrabbedObject != NULL) 
	{
		//Player has latched on to something, release it
		myGrabbedObject->ReleaseComponent();
		myGrabbedObject = NULL;
	}
}

/**
* Called when player releases with the right hand.
*/
void AVRTemplateCPPCharacter::OnReleaseRightHand()
{
	myRightHandIsGrabbing = false;

	if (myGrabbedObject != NULL)
	{
		//Player has latched on to something, release it
		myGrabbedObject->ReleaseComponent();
		myGrabbedObject = NULL;
	}
}

/**
 * Does the actual grabbing and moving of the object and the hand/controller.
 *
 * @param reach			   The reach distance.
 * @param lockGrab		   true to use lock grab.
 * @param showDebugLine    true to show, false to hide the debug line.
 * @param [in] theHand	   If non-null, the hand.
 */
void AVRTemplateCPPCharacter::Grab(float reach, bool lockGrab, bool showDebugLine, USkeletalMeshComponent* theHand)
{
	FVector lineTraceEnd;
	FVector lineTraceStart;
	AActor* actorHit = NULL;
	UPhysicsHandleComponent* physicsHandle = NULL;
	
	//Set if this grab is a precision grab
	if (lockGrab == true)
	{
		myIsLockGrab = true;
		myIsJustGrabbed = true;
	}
	else 
	{
		myIsLockGrab = false;
		myIsJustGrabbed = false;
	}
	
	//Update the current location and rotation variables.
	myCurrControllerLocation = theHand->GetComponentLocation();
	myCurrControllerRotation = theHand->GetComponentRotation();
	myControllerTransform = theHand->GetComponentTransform();

	//Set Line Trace (Ray-Cast) endpoints
	lineTraceStart = myCurrControllerLocation;
	lineTraceEnd = myCurrControllerLocation + (myCurrControllerRotation.Vector() * reach);

	//Attempt to grab the actor.
	if (showDebugLine == true)
	{
		//Show Debug line (helpful for a visual indicator during testing)
		DrawDebugLine(GetWorld(), lineTraceStart, lineTraceEnd, FColor(255, 0, 0), false, -1, 0, 12.0f);
	}

	//Line trace
	actorHit = CastRay(lineTraceStart, lineTraceEnd);

	//Check if there's a valid object to grab
	if (actorHit != NULL)
	{
		//Only grab an object with a Physics Handle
		physicsHandle = actorHit->FindComponentByClass<UPhysicsHandleComponent>();

		if (physicsHandle != NULL)
		{
			//Attempt to Grab Object
			UPrimitiveComponent* componentToGrab = Cast<UPrimitiveComponent>(actorHit->GetRootComponent());

			physicsHandle->GrabComponentAtLocationWithRotation(componentToGrab,
															   FName("GrabbedPhisicsComponent"),
															   actorHit->GetActorLocation(),
															   FRotator(0.0f, 0.0f, 0.0f));

			//If object is successfully grabbed, move object with Controller
			if (physicsHandle->GrabbedComponent != NULL)
			{
				physicsHandle->SetTargetLocation(lineTraceEnd);
				myGrabbedObject = physicsHandle;
				AVRTemplateCPPCharacter::SetDistanceFromController(FVector::Dist(actorHit->GetActorLocation(), lineTraceStart));
			}
		}
	}
}

/**
 * Ray-cast and get any object hit by the line trace.
 *
 * @param [in] lineTraceStart The line trace start.
 * @param [in] lineTraceEnd   The line trace end.
 *
 * @return The actor if hit happens., null if no collision.
 */
AActor* AVRTemplateCPPCharacter::CastRay(FVector& lineTraceStart, FVector& lineTraceEnd)
{
	FHitResult hit;
	AActor* actorHit = NULL;
	FCollisionQueryParams traceParameters(FName(TEXT("SmashCharacterRayCast")), false, GetOwner());

	//Do line trace / ray-cast
	AVRTemplateCPPCharacter::GetWorld()->LineTraceSingleByObjectType(hit,
															 lineTraceStart,
															 lineTraceEnd,
															 FCollisionObjectQueryParams(ECollisionChannel::ECC_PhysicsBody),
															 traceParameters);

	//Check and see what we hit.
	actorHit = hit.GetActor();

	//Return the actor if any hits occurred.
	if (actorHit != NULL)
	{
		return actorHit;
	}
	else 
	{
		return NULL;
	}
}

/**
* Resets the VR orientation for the user.
*/
void AVRTemplateCPPCharacter::OnResetVR()
{
	UHeadMountedDisplayFunctionLibrary::ResetOrientationAndPosition();
}

/**
 * Move forward.
 *
 * @param value The amount to move forward or backward.
 */
void AVRTemplateCPPCharacter::MoveForward(float value)
{
	if (value != 0.0f)
	{
		AddMovementInput(GetActorForwardVector(), value);
	}
}

/**
 * Move stafing.
 *
 * @param value The amount to move left or right.
 */
void AVRTemplateCPPCharacter::MoveStafing(float value)
{
	if (value != 0.0f)
	{
		AddMovementInput(GetActorRightVector(), value);
	}
}

/**
 * Turns left or right.
 *
 * @param rate The rate to turn.
 */
void AVRTemplateCPPCharacter::Turn(float rate)
{
	AddControllerYawInput(rate * myBaseTurnRate * GetWorld()->GetDeltaSeconds());
}

/**
 * Looks up or down.
 *
 * @param rate The rate to look up or down.
 */
void AVRTemplateCPPCharacter::LookUp(float rate)
{
	AddControllerPitchInput(rate * myBaseLookUpRate * GetWorld()->GetDeltaSeconds());
}

/**
 * Gets the first person mesh.
 *
 * @return null if it fails, else the first person mesh.
 */
USkeletalMeshComponent* AVRTemplateCPPCharacter::GetFirstPersonMesh()
{
	return myFirstPersonMesh;
}

/**
 * Gets the first person left hand mesh.
 *
 * @return null if it fails, else the first person left hand mesh.
 */
USkeletalMeshComponent* AVRTemplateCPPCharacter::GetFirstPersonLeftHand()
{
	return myFirstPersonLeftHand;
}

/**
 * Gets the first person right hand mesh.
 *
 * @return null if it fails, else the first person right hand mesh.
 */
USkeletalMeshComponent* AVRTemplateCPPCharacter::GetFirstPersonRightHand()
{
	return myFirstPersonRightHand;
}

/**
 * Gets VR left hand mesh.
 *
 * @return null if it fails, else the VR left hand mesh.
 */
USkeletalMeshComponent* AVRTemplateCPPCharacter::GetVRLeftHand()
{
	return myVRLeftHand;
}

/**
 * Gets VR right hand mesh.
 *
 * @return null if it fails, else the VR right hand mesh.
 */
USkeletalMeshComponent* AVRTemplateCPPCharacter::GetVRRightHand()
{
	return myVRRightHand;
}

/**
 * Gets base turn rate.
 *
 * @return The base turn rate.
 */
float AVRTemplateCPPCharacter::GetBaseTurnRate()
{
	return myBaseTurnRate;
}

/**
 * Gets base look up rate.
 *
 * @return The base look up rate.
 */
float AVRTemplateCPPCharacter::GetBaseLookUpRate()
{
	return myBaseLookUpRate;
}

/**
 * Gets left motion controller.
 *
 * @return null if it fails, else the left motion controller.
 */
UMotionControllerComponent* AVRTemplateCPPCharacter::GetLeftMotionController()
{
	return myLeftMotionController;
}

/**
 * Gets right motion controller.
 *
 * @return null if it fails, else the right motion controller.
 */
UMotionControllerComponent* AVRTemplateCPPCharacter::GetRightMotionController()
{
	return myRightMotionController;
}

/**
 * Gets distance from controller.
 *
 * @return The distance from controller.
 */
float AVRTemplateCPPCharacter::GetDistanceFromController()
{
	return myDistanceFromController;
}

/**
* Gets the reach distance.
*
* @return The reach distance.
*/
float AVRTemplateCPPCharacter::GetReachDistance()
{
	return myReachDistance;
}

/**
 * Gets minimum distance from controller.
 *
 * @return The minimum distance from controller.
 */
float AVRTemplateCPPCharacter::GetMinDistanceFromController()
{
	return myMinDistanceFromController;
}

/**
 * Gets maximum distance from controller.
 *
 * @return The maximum distance from controller.
 */
float AVRTemplateCPPCharacter::GetMaxDistanceFromController()
{
	return myMaxDistanceFromController;
}

/**
 * Gets is using motion controllers.
 *
 * @return true if using motion controllers, false if not.
 */
bool AVRTemplateCPPCharacter::GetIsUsingMotionControllers()
{
	return myIsUsingMotionControllers;
}

/**
 * Gets current controller location of the grabbing controller.
 *
 * @return The current controller location of the grabbing controller.
 */
FVector AVRTemplateCPPCharacter::GetCurrControllerLocation()
{
	return myCurrControllerLocation;
}

/**
 * Gets current controller rotation of the grabbing controller.
 *
 * @return The current controller rotation of the grabbing controller.
 */
FRotator AVRTemplateCPPCharacter::GetCurrControllerRotation()
{
	return myCurrControllerRotation;
}

/**
 * Gets left hand is grabbing.
 *
 * @return true if left hand is currently grabbing, false if not.
 */
bool AVRTemplateCPPCharacter::GetLeftHandIsGrabbing()
{
	return myLeftHandIsGrabbing;
}

/**
 * Gets right hand is grabbing.
 *
 * @return true if right hand is currently grabbing, false if not.
 */
bool AVRTemplateCPPCharacter::GetRightHandIsGrabbing()
{
	return myRightHandIsGrabbing;
}

/**
 * Gets is lock grab.
 *
 * @return true if using locking grab, false if not.
 */
bool AVRTemplateCPPCharacter::GetIsLockGrab()
{
	return myIsLockGrab;
}

/**
 * Gets is just grabbed.
 *
 * @return true if just grabbed, false if not.
 */
bool AVRTemplateCPPCharacter::GetIsJustGrabbed()
{
	return myIsJustGrabbed;
}

/**
 * Gets grabbed object transform.
 *
 * @return The grabbed object transform.
 */
FTransform AVRTemplateCPPCharacter::GetGrabbedObjectTransform()
{
	return myGrabbedObjectTransform;
}

/**
 * Gets controller transform.
 *
 * @return The controller transform.
 */
FTransform AVRTemplateCPPCharacter::GetControllerTransform()
{
	return myControllerTransform;
}

/**
 * Gets new grabbed location.
 *
 * @return The new grabbed location.
 */
FVector AVRTemplateCPPCharacter::GetNewGrabbedLocation()
{
	return myNewGrabbedLocation;
}

/**
 * Gets the difference between previous and old grabbed object locations.
 *
 * @return The difference between previous and old grabbed object locations.
 */
FVector AVRTemplateCPPCharacter::GetDiffGrabbedLocation()
{
	return myDiffGrabbedLocation;
}

/**
 * Gets the difference between controller rotation and grabbed object rotation.
 *
 * @return The difference between controller rotation and grabbed object rotation.
 */
FQuat AVRTemplateCPPCharacter::GetDiffGrabbedRotation()
{
	return myDiffGrabbedRotation;
}

/**
 * Gets grabbed object.
 *
 * @return null if it fails, else the grabbed object.
 */
UPhysicsHandleComponent* AVRTemplateCPPCharacter::GetGrabbedObject()
{
	return myGrabbedObject;
}

/**
 * Sets the first person mesh.
 *
 * @param [in,out] firstPersonMesh If non-null, the first person mesh.
 */
void AVRTemplateCPPCharacter::SetFirstPersonMesh(USkeletalMeshComponent* firstPersonMesh)
{
	myFirstPersonMesh = firstPersonMesh;
}

/**
 * Sets first person left hand mesh.
 *
 * @param [in,out] firstPersonLeftHandMesh If non-null, the first person left hand mesh.
 */
void AVRTemplateCPPCharacter::SetFirstPersonLeftHand(USkeletalMeshComponent* firstPersonLeftHandMesh)
{
	myFirstPersonLeftHand = firstPersonLeftHandMesh;
}

/**
 * Sets first person right hand mesh.
 *
 * @param [in,out] firstPersonRightHandMesh If non-null, the first person right hand mesh.
 */
void AVRTemplateCPPCharacter::SetFirstPersonRightHand(USkeletalMeshComponent* firstPersonRightHandMesh)
{
	myFirstPersonRightHand = firstPersonRightHandMesh;
}

/**
 * Sets VR left hand mesh.
 *
 * @param [in,out] vrLeftHandMesh If non-null, the VR left hand mesh.
 */
void AVRTemplateCPPCharacter::SetVRLeftHand(USkeletalMeshComponent* vrLeftHandMesh)
{
	myVRLeftHand = vrLeftHandMesh;
}

/**
 * Sets VR right hand mesh.
 *
 * @param [in,out] vrRightHandMesh If non-null, the VR right hand mesh.
 */
void AVRTemplateCPPCharacter::SetVRRightHand(USkeletalMeshComponent* vrRightHandMesh)
{
	myVRRightHand = vrRightHandMesh;
}

/**
 * Sets base turn rate.
 *
 * @param baseTurnRate The base turn rate.
 */
void AVRTemplateCPPCharacter::SetBaseTurnRate(float baseTurnRate)
{
	myBaseTurnRate = baseTurnRate;
}

/**
 * Sets base look up rate.
 *
 * @param baseLookUpRate The base look up rate.
 */
void AVRTemplateCPPCharacter::SetBaseLookUpRate(float baseLookUpRate)
{
	myBaseLookUpRate = baseLookUpRate;
}

/**
* Sets the reach distance.
*
* @param reachDistance The reach distance.
*/
void AVRTemplateCPPCharacter::SetReachDistance(float reachDistance)
{
	myReachDistance = reachDistance;
}

/**
 * Sets minimum distance from controller.
 *
 * @param minDistance The minimum distance.
 */
void AVRTemplateCPPCharacter::SetMinDistanceFromController(float minDistance)
{
	myMinDistanceFromController = minDistance;
}

/**
 * Sets maximum distance from controller.
 *
 * @param maxDistance The maximum distance.
 */
void AVRTemplateCPPCharacter::SetMaxDistanceFromController(float maxDistance)
{
	myMaxDistanceFromController = maxDistance;
}

/**
 * Sets is using motion controllers.
 *
 * @param isUsingMotionControllers true if this object is using motion controllers.
 */
void AVRTemplateCPPCharacter::SetIsUsingMotionControllers(bool isUsingMotionControllers)
{
	myIsUsingMotionControllers = isUsingMotionControllers;
}

/**
 * Sets current controller location.
 *
 * @param currControllerLocation The current controller location.
 */
void AVRTemplateCPPCharacter::SetCurrControllerLocation(FVector currControllerLocation)
{
	myCurrControllerLocation = currControllerLocation;
}

/**
 * Sets current controller rotation.
 *
 * @param currControllerRotation The current controller rotation.
 */
void AVRTemplateCPPCharacter::SetCurrControllerRotation(FRotator currControllerRotation)
{
	myCurrControllerRotation = currControllerRotation;
}

/**
 * Sets left hand is grabbing.
 *
 * @param leftHandIsGrabbing true if left hand is grabbing.
 */
void AVRTemplateCPPCharacter::SetLeftHandIsGrabbing(bool leftHandIsGrabbing)
{
	myLeftHandIsGrabbing = leftHandIsGrabbing;
}

/**
 * Sets right hand is grabbing.
 *
 * @param rightHandIsGrabbing true if right hand is grabbing.
 */
void AVRTemplateCPPCharacter::SetRightHandIsGrabbing(bool rightHandIsGrabbing)
{
	myRightHandIsGrabbing = rightHandIsGrabbing;
}

/**
 * Sets is lock grab.
 *
 * @param isLockGrab true if using lock grab, false otherwise.
 */
void AVRTemplateCPPCharacter::SetIsLockGrab(bool isLockGrab)
{
	myIsLockGrab = isLockGrab;
}

/**
 * Sets is just grabbed.
 *
 * @param isJustGrabbed true if this object is just grabbed.
 */
void AVRTemplateCPPCharacter::SetIsJustGrabbed(bool isJustGrabbed)
{
	myIsJustGrabbed = isJustGrabbed;
}

/**
 * Sets grabbed object transform.
 *
 * @param grabbedObjectTransform The grabbed object transform.
 */
void AVRTemplateCPPCharacter::SetGrabbedObjectTransform(FTransform grabbedObjectTransform)
{
	myGrabbedObjectTransform = grabbedObjectTransform;
}

/**
 * Sets controller transform.
 *
 * @param controllerTransform The controller transform.
 */
void AVRTemplateCPPCharacter::SetControllerTransform(FTransform controllerTransform)
{
	myControllerTransform = controllerTransform;
}

/**
 * Sets grabbed object.
 *
 * @param [in] grabbedObject If non-null, the grabbed object.
 */
void AVRTemplateCPPCharacter::SetGrabbedObject(UPhysicsHandleComponent* grabbedObject)
{
	myGrabbedObject = grabbedObject;
}

/**
 * Sets distance from controller.
 *
 * @param newDistance The new distance.
 */
void AVRTemplateCPPCharacter::SetDistanceFromController(float newDistance)
{
	if (newDistance > myMinDistanceFromController && newDistance < myMaxDistanceFromController)
	{
		myDistanceFromController = newDistance;
	}
}