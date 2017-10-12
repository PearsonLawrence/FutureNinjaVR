// Copyright 1998-2016 Epic Games, Inc. All Rights Reserved.
#pragma once
#include "GameFramework/Actor.h"
#include "VRTemplateCPPProjectile.generated.h"

class USphereComponent;
class UProjectileMovementComponent;

UCLASS(config=Game)
class AVRTemplateCPPProjectile : public AActor
{
	GENERATED_BODY()

	//Sphere collision component.
	UPROPERTY(VisibleDefaultsOnly, Category=Projectile)
	USphereComponent* mySphereCollisionComp;

	//Projectile movement component.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Movement, meta = (AllowPrivateAccess = "true"))
	UProjectileMovementComponent* myProjectileMovementComp;

public:
	///Constructor
	AVRTemplateCPPProjectile();

	///Functions
	UFUNCTION()
	void OnHit(UPrimitiveComponent* hitComp, AActor* otherActor, UPrimitiveComponent* otherComp, FVector normalImpulse, const FHitResult& hit);

};

