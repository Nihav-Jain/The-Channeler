// Fill out your copyright notice in the Description page of Project Settings.

#pragma once
#include "GameFramework/Pawn.h"
#include "Engine/TargetPoint.h"
#include "MoveToTargetActor.generated.h"

USTRUCT(BlueprintType)
struct FTargetPoints
{
	GENERATED_USTRUCT_BODY();

	UPROPERTY(EditAnywhere)// , BlueprintReadWrite, Category = "Analytics")
		ATargetPoint *Start;
	FTargetPoints()
	{
		Start = nullptr;
	}

};

UCLASS()
class THECHANNELER_API AMoveToTargetActor : public APawn
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AMoveToTargetActor();

	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	
	// Called every frame
	virtual void Tick( float DeltaSeconds ) override;

// the end! You will get odd compile errors otherwise

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Move To Target")
		TArray<FTargetPoints> ArrayOfTarget;// (AActor::GetWorld());
	UPROPERTY(EditAnywhere)
		int32 publicInt;
};

//	USTRUCT()
//	struct FJoyStruct
//	{
//		GENERATED_USTRUCT_BODY()
//
//			//Always make USTRUCT variables into UPROPERTY()
//			//    any non-UPROPERTY() struct vars are not replicated
//
//			// So to simplify your life for later debugging, always use UPROPERTY()
//			UPROPERTY()
//			int32 SampleInt32;
//
//		UPROPERTY()
//			AActor* TargetActor;
//
//		//Set
//		void SetInt(const int32 NewValue)
//		{
//			SampleInt32 = NewValue;
//		}
//
//		//Get
//		AActor* GetActor()
//		{
//			return TargetActor;
//		}
//
//		//Check
//		bool ActorIsValid() const
//		{
//			if (!TargetActor) return false;
//			return TargetActor->IsValidLowLevel();
//		}
//
//		//Constructor
//		FJoyStruct()
//		{
//			//Always initialize your USTRUCT variables!
//			//   exception is if you know the variable type has its own default constructor
//			SampleInt32 = 5;
//			TargetActor = NULL;
//		}
//	};  //Always remember this ;  at