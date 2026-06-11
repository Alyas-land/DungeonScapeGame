// Fill out your copyright notice in the Description page of Project Settings.


#include "TriggerComponent.h"


UTriggerComponent::UTriggerComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	

}


void UTriggerComponent::BeginPlay()
{
	Super::BeginPlay();
	if (MoverActor) {
		Mover = MoverActor->FindComponentByClass<UMover>();
		if (Mover) {
			UE_LOG(LogTemp, Display, TEXT("Mover Pointer is OK."))
			

		}
		else {
			UE_LOG(LogTemp, Display, TEXT("Mover Actor is not OK."))

		}
	}
	else {
		UE_LOG(LogTemp, Display, TEXT("Mover Actor is nullptr, pleade Initialize."))
	}
	
	if (IsPressurePlate) {
		OnComponentBeginOverlap.AddDynamic(this, &UTriggerComponent::OnOverlapBegin);
		OnComponentEndOverlap.AddDynamic(this, &UTriggerComponent::OnOverlapEnd);
	}
	
	
}



void UTriggerComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}

void UTriggerComponent::OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (OtherActor && OtherActor->ActorHasTag("Player"))
		PlassureCounter++;

		if(!IsTrigger)
			Trigger(true);
	
}

void UTriggerComponent::OnOverlapEnd(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (OtherActor && OtherActor->ActorHasTag("Player"))
		PlassureCounter--;
		if(IsTrigger && PlassureCounter == 0)
			Trigger(false);
}

void UTriggerComponent::Trigger(bool newTriggerValue)
{
	IsTrigger = newTriggerValue;
	if (Mover)
		Mover->SetShouldMove(newTriggerValue);

	else {
		UE_LOG(LogTemp, Display, TEXT("%s, Hasn't Mover AActor*"), *GetOwner()->GetActorNameOrLabel());

	}
}


