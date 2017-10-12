#include "VRTemplateCPP.h"
#include "VRTemplateCPPProjectile.h"
#include "GameFramework/ProjectileMovementComponent.h"

/**
* Constructor
*/
AVRTemplateCPPProjectile::AVRTemplateCPPProjectile()
{
	//Use a sphere as a simple collision representation
	mySphereCollisionComp = CreateDefaultSubobject<USphereComponent>(TEXT("SphereCollisionComp"));
	mySphereCollisionComp->InitSphereRadius(5.0f);
	mySphereCollisionComp->BodyInstance.SetCollisionProfileName("Projectile");
	mySphereCollisionComp->OnComponentHit.AddDynamic(this, &AVRTemplateCPPProjectile::OnHit);		// set up a notification for when this component hits something blocking

	//Players can't walk on it
	mySphereCollisionComp->SetWalkableSlopeOverride(FWalkableSlopeOverride(WalkableSlope_Unwalkable, 0.f));
	mySphereCollisionComp->CanCharacterStepUpOn = ECB_No;

	//Set as root component
	RootComponent = mySphereCollisionComp;

	//Use a ProjectileMovementComponent to govern this projectile's movement
	myProjectileMovementComp = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileMovementComp"));
	myProjectileMovementComp->UpdatedComponent = mySphereCollisionComp;
	myProjectileMovementComp->InitialSpeed = 3000.f;
	myProjectileMovementComp->MaxSpeed = 3000.f;
	myProjectileMovementComp->bRotationFollowsVelocity = true;
	myProjectileMovementComp->bShouldBounce = true;

	//Die after 3 seconds by default
	InitialLifeSpan = 3.0f;
}

/**
 * Called when projectile hits something.
 *
 * @param [in,out] hitComp    If non-null, the hit component.
 * @param [in,out] otherActor If non-null, the other actor.
 * @param [in,out] otherComp  If non-null, the other component.
 * @param normalImpulse		  The normal impulse.
 * @param hit				  The hit.
 */
void AVRTemplateCPPProjectile::OnHit(UPrimitiveComponent* hitComp, AActor* otherActor, UPrimitiveComponent* otherComp, FVector normalImpulse, const FHitResult& hit)
{
	// Only add impulse and destroy projectile if we hit a physics
	if ((otherActor != NULL) && (otherActor != this) && (otherComp != NULL) && otherComp->IsSimulatingPhysics())
	{
		otherComp->AddImpulseAtLocation(GetVelocity() * 100.0f, GetActorLocation());

		Destroy();
	}
}