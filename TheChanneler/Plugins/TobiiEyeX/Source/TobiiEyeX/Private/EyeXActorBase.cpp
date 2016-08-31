// Copyright 2014 Tobii Technology AB. All Rights Reserved.

#include "EyeXPluginPrivatePCH.h"
#include "EyeXActorBase.h"
#include "EyeXPlayerController.h" // for the EyeXPlayerController check

AEyeXActorBase::AEyeXActorBase(const class FObjectInitializer& ObjectInitializer)
: Super(ObjectInitializer)
{
	AEyeXActorBase::StaticClass();
	PrimaryActorTick.bCanEverTick = true;

	// Default values
	MinDistance = 0.0f;
	MaxDistance = 0.0f;
	bIgnoreHeightInDistanceCalculation = false;
	bVisualizeDistance = false;
	DwellTimeActivate = 0.3f;
	DwellTimeDeactivate = 0.3f;

	// Initialize properties
	bTraceHit = false;
	bHasFocus = false;
	bResetGazeTimeImmediately = true;
	CurrentTimeIn = 0.0f;
	CurrentTimeOut = 0.0f;
}

void AEyeXActorBase::BeginPlay()
{
	Super::BeginPlay();

#if !UE_BUILD_SHIPPING
	if (!GetWorld())
		return;

	for (FConstPlayerControllerIterator Itr = GetWorld()->GetPlayerControllerIterator(); Itr; Itr++)
	{
		if (Cast<AEyeXPlayerController>(*Itr))
			return;
	}

	UE_LOG(LogEyeX, Warning, TEXT("Scene does not contain an EyeXPlayerController! EyeXActorBase cannot be used without it!"));
#endif

}

void AEyeXActorBase::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
#if !UE_BUILD_SHIPPING
	if (bVisualizeDistance)
	{
		VisualizeDistance();
	}
#endif

	UpdateDwellTime(DeltaSeconds);
}

void AEyeXActorBase::UpdateDwellTime(float DeltaSeconds)
{
	if (bTraceHit && IsWithinDistance())
	{
		if (!bHasFocus)
		{
			if (ClampTowardsTarget(CurrentTimeIn, DeltaSeconds, DwellTimeActivate))
			{
				bHasFocus = true;
				CurrentTimeIn = 0.0f;
				GotGazeFocus();
			}
		}
		else
		{
			bResetGazeTimeImmediately ? CurrentTimeOut = 0.0f : ClampTowardsTarget(CurrentTimeOut, -DeltaSeconds, DwellTimeDeactivate);
		}
	}
	else
	{
		if (bHasFocus)
		{
			if (ClampTowardsTarget(CurrentTimeOut, DeltaSeconds, DwellTimeDeactivate))
			{
				bHasFocus = false;
				CurrentTimeOut = 0.0f;
				LostGazeFocus();
			}
		}
		else
		{
			bResetGazeTimeImmediately ? CurrentTimeIn = 0.0f : ClampTowardsTarget(CurrentTimeIn, -DeltaSeconds, DwellTimeActivate);
		}
	}
}

bool AEyeXActorBase::ClampTowardsTarget(float& CurrentTime, const float DeltaSeconds, const float Target)
{
	// NOTE: The original implementation uses float == float, that smells to me. By Yuhsiang
	// Returns true if target is reached.
	if (DeltaSeconds > 0.f && FMath::IsNearlyEqual(CurrentTime, Target))
		return true;
	else if (DeltaSeconds < 0.f && FMath::IsNearlyEqual(CurrentTime, 0.0f))
		return true;

	CurrentTime = FMath::Clamp(CurrentTime + DeltaSeconds, 0.0f, Target);
	return false;
}

void AEyeXActorBase::VisualizeDistance() const
{
	if (MaxDistance != 0.0f)
	{
		DrawShape(MaxDistance, FColor::Green);
	}
	if (MinDistance != 0.0f)
	{
		DrawShape(MinDistance, FColor::Blue);
	}
}

void AEyeXActorBase::DrawShape(float Distance, FColor Color) const
{
	if (!GetWorld()) return;

	if (bIgnoreHeightInDistanceCalculation)
	{
		DrawDebugCapsule(GetWorld(), GetActorLocation(), WORLD_MAX / 2, Distance, FQuat::Identity, Color);
	}
	else
	{
		DrawDebugSphere(GetWorld(), GetActorLocation(), Distance, 32, Color);
	}
}

bool AEyeXActorBase::IsWithinDistance() const
{
	if (MaxDistance == 0.0f && MinDistance == 0.0f)
		return true;

	if (!GetWorld() || !GetWorld()->GetFirstPlayerController() || !GetWorld()->GetFirstPlayerController()->PlayerCameraManager)
		return false;

	FVector CameraLocation = GetWorld()->GetFirstPlayerController()->PlayerCameraManager->GetCameraLocation();
	FVector ActorLocation = GetActorLocation();
	if (bIgnoreHeightInDistanceCalculation)
	{
		CameraLocation.Z = ActorLocation.Z = 0.0f;
	}

	const float DistanceToCamera = FVector::Dist(CameraLocation, ActorLocation);
	const bool bMaxDistance = MaxDistance == 0.0f ? true : DistanceToCamera <= MaxDistance;
	const bool bMinDistance = MinDistance == 0.0f ? true : DistanceToCamera >= MinDistance;
	return bMaxDistance && bMaxDistance;
}

bool AEyeXActorBase::HasGazeFocus() const
{
	return bHasFocus;
}

void AEyeXActorBase::LostGazeFocus()
{
	LostGazeFocusBP();
}

void AEyeXActorBase::GotGazeFocus()
{
	GotGazeFocusBP();
}
