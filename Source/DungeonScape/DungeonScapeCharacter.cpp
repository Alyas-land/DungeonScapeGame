// Copyright Epic Games, Inc. All Rights Reserved.

#include "DungeonScapeCharacter.h"
#include "Animation/AnimInstance.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "EnhancedInputComponent.h"
#include "InputActionValue.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "DungeonScape.h"

#include "CollectableItem.h"
#include "Lock.h"

ADungeonScapeCharacter::ADungeonScapeCharacter()
{
	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(55.f, 96.0f);
	
	// Create the first person mesh that will be viewed only by this character's owner
	FirstPersonMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("First Person Mesh"));

	FirstPersonMesh->SetupAttachment(GetMesh());
	FirstPersonMesh->SetOnlyOwnerSee(true);
	FirstPersonMesh->FirstPersonPrimitiveType = EFirstPersonPrimitiveType::FirstPerson;
	FirstPersonMesh->SetCollisionProfileName(FName("NoCollision"));

	// Create the Camera Component	
	FirstPersonCameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("First Person Camera"));
	FirstPersonCameraComponent->SetupAttachment(FirstPersonMesh, FName("head"));
	FirstPersonCameraComponent->SetRelativeLocationAndRotation(FVector(-2.8f, 5.89f, 0.0f), FRotator(0.0f, 90.0f, -90.0f));
	FirstPersonCameraComponent->bUsePawnControlRotation = true;
	FirstPersonCameraComponent->bEnableFirstPersonFieldOfView = true;
	FirstPersonCameraComponent->bEnableFirstPersonScale = true;
	FirstPersonCameraComponent->FirstPersonFieldOfView = 70.0f;
	FirstPersonCameraComponent->FirstPersonScale = 0.6f;

	// configure the character comps
	GetMesh()->SetOwnerNoSee(true);
	GetMesh()->FirstPersonPrimitiveType = EFirstPersonPrimitiveType::WorldSpaceRepresentation;

	GetCapsuleComponent()->SetCapsuleSize(34.0f, 96.0f);

	// Configure character movement
	GetCharacterMovement()->BrakingDecelerationFalling = 1500.0f;
	GetCharacterMovement()->AirControl = 0.5f;
}

void ADungeonScapeCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{	
	// Set up action bindings
	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		// Jumping
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Started, this, &ADungeonScapeCharacter::DoJumpStart);
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Completed, this, &ADungeonScapeCharacter::DoJumpEnd);

		// Moving
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &ADungeonScapeCharacter::MoveInput);

		// Looking/Aiming
		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &ADungeonScapeCharacter::LookInput);
		EnhancedInputComponent->BindAction(MouseLookAction, ETriggerEvent::Triggered, this, &ADungeonScapeCharacter::LookInput);

		// Interact
		EnhancedInputComponent->BindAction(InteractAction, ETriggerEvent::Started, this, &ADungeonScapeCharacter::Interact);

		// For test and practice
		EnhancedInputComponent->BindAction(TestAction, ETriggerEvent::Triggered, this, &ADungeonScapeCharacter::TestAct);

	}
	else
	{
		UE_LOG(LogDungeonScape, Error, TEXT("'%s' Failed to find an Enhanced Input Component! This template is built to use the Enhanced Input system. If you intend to use the legacy system, then you will need to update this C++ file."), *GetNameSafe(this));
	}
}

void ADungeonScapeCharacter::Interact()
{
	FVector Start, End;
	Start = FirstPersonCameraComponent->GetComponentLocation();
	End = Start + (FirstPersonCameraComponent->GetForwardVector() * MaxInteractionDistance);

	FCollisionShape TestSphere = FCollisionShape::MakeSphere(InteractionSphereRadius);
	DrawDebugSphere(GetWorld(), Start, InteractionSphereRadius, 20.f, FColor::Cyan, false, 5.0f);
	DrawDebugSphere(GetWorld(), End, InteractionSphereRadius, 20.0f, FColor::Red, false, 5.0f);

	DrawDebugLine(GetWorld(), Start, End, FColor::Blue, false, 10.f);

	FHitResult HitResult;
	bool HasHit = GetWorld()->SweepSingleByChannel(HitResult, Start, End, FQuat::Identity, ECC_GameTraceChannel2, TestSphere);
	
	if (HasHit) {
		AActor* HitActor = HitResult.GetActor();
		
		if (HitActor->ActorHasTag("CollectableItem")) {
			ACollectableItem* CollectableItem = Cast<ACollectableItem>(HitActor);
			if (CollectableItem) {
				UE_LOG(LogTemp, Display, TEXT("Collectable Item Name: %s"), *CollectableItem->ItemName);
				ItemNameArray.Add(CollectableItem->ItemName);
				CollectableItem->Destroy();

			}


		}
		else if (HitActor->ActorHasTag("Lock")) {
			ALock* Lock = Cast<ALock>(HitActor);
			if (Lock) {
				if (!Lock->GetIsKeyPlaced()) {
					// Lock Is Empty
					int32 ItemRemove = ItemNameArray.RemoveSingle(Lock->KeyItemName);
					if (ItemRemove) {
						Lock->SetIsKeyPlaced(true);
					}
					else{
						//
						UE_LOG(LogTemp, Display, TEXT("Key Item Not Inventory"));

					}
				}
				else {
					ItemNameArray.Add(Lock->KeyItemName);
					Lock->SetIsKeyPlaced(false);

					
				}

			}
		}
	}
	else {
		UE_LOG(LogTemp, Display, TEXT("No Hited Actor"));
	}




}

void ADungeonScapeCharacter::TestAct()
{
	UE_LOG(LogTemp, Display, TEXT("Test Input Prac"));

}


void ADungeonScapeCharacter::MoveInput(const FInputActionValue& Value)
{
	// get the Vector2D move axis
	FVector2D MovementVector = Value.Get<FVector2D>();

	// pass the axis values to the move input
	DoMove(MovementVector.X, MovementVector.Y);

}

void ADungeonScapeCharacter::LookInput(const FInputActionValue& Value)
{
	// get the Vector2D look axis
	FVector2D LookAxisVector = Value.Get<FVector2D>();

	// pass the axis values to the aim input
	DoAim(LookAxisVector.X, LookAxisVector.Y);

}

void ADungeonScapeCharacter::DoAim(float Yaw, float Pitch)
{
	if (GetController())
	{
		// pass the rotation inputs
		AddControllerYawInput(Yaw);
		AddControllerPitchInput(Pitch);
	}
}

void ADungeonScapeCharacter::DoMove(float Right, float Forward)
{
	if (GetController())
	{
		// pass the move inputs
		AddMovementInput(GetActorRightVector(), Right);
		AddMovementInput(GetActorForwardVector(), Forward);
	}
}

void ADungeonScapeCharacter::DoJumpStart()
{
	// pass Jump to the character
	Jump();
}

void ADungeonScapeCharacter::DoJumpEnd()
{
	// pass StopJumping to the character
	StopJumping();
}


