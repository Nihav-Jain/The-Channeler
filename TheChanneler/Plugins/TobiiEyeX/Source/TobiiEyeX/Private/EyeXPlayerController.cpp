// Copyright 2014 Tobii Technology AB. All Rights Reserved.

#include "EyeXPluginPrivatePCH.h"
#include "EyeXPlayerController.h"
#include "EyeXActorBase.h"
#include "EyeXMathHelpers.h"
#include "EyeXUtils.h"

/*
 * Visualization functions for use during development.
 * These functions are #defined to nothing in shipping builds.
 */
#if !UE_BUILD_SHIPPING
static void VisualizeGazePoint(bool bVisualizeDetection, UWorld *World, const FVector& Point)
{
	if (bVisualizeDetection)
	{
		DrawDebugPoint(World, Point, 3, FColor::Red);
	}
}

static void VisualizeHitTestPoint(bool bVisualizeDetection, UWorld *World, const FVector& Point)
{
	if (bVisualizeDetection)
	{
		DrawDebugPoint(World, Point, 3, FColor::Green);
	}
}

static void VisualizeHit(bool bVisualizeDetection, UWorld *World, const FHitResult& HitResult, float Radius = 0)
{
	if (bVisualizeDetection)
	{
		if (HitResult.bBlockingHit)
		{
			DrawDebugSphere(World, HitResult.Location, FMath::Max(10.0f, Radius), 32, FColor::Blue);
		}
	}
}
#else
#define VisualizeGazePoint(...)
#define VisualizeHitTestPoint(...)
#define VisualizeHit(...)
#endif


AEyeXPlayerController::AEyeXPlayerController(const class FObjectInitializer& ObjectInitializer)
: Super(ObjectInitializer)
{
	AEyeXPlayerController::StaticClass();
	PrimaryActorTick.bCanEverTick = true;

	// Default values
	DetectionMode = EEyeXDetectionMode::LineTrace;
	GazePointMode = EEyeXGazePointDataMode::LightlyFiltered;
	MaxDistance = 2000.0f;
	CollisionChannels.Add(ECollisionChannel::ECC_WorldDynamic);
	MinUpdateDistance = 20.0f;
	bVisualizeDetection = false;
	SweepSphereRadius = 20.0f;
	SweepIntervals = 4;
	BoxSize = FVector2D(20.0f, 20.0f);

	// Initialize properties
	EyeX = nullptr;
	LastUsedGazePoint = FEyeXGazePoint::Invalid();
	LastFocusedActor = nullptr;
}

void AEyeXPlayerController::BeginPlay()
{
	Super::BeginPlay();

	if (!IEyeXPlugin::IsAvailable())
	{
		UE_LOG(LogEyeX, Error, TEXT("TobiiEyeX Module is unavailable."));
		return;
	}

	EyeX = &IEyeXPlugin::Get();
}

void AEyeXPlayerController::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (DetectionMode == EEyeXDetectionMode::None)
	{
		// Detection is disabled, so reset and return.
		LastUsedGazePoint = FEyeXGazePoint::Invalid();
		if (LastFocusedActor)
		{
			LastFocusedActor->bTraceHit = false;
			LastFocusedActor = nullptr;
		}

		return;
	}

	if (!EyeX) return;
	auto GazePoint = EyeX->GetGazePoint(GazePointMode);
	if (!GazePoint.bHasValue)
	{
		// No gaze point: skip.
		return;
	}

	if (MinUpdateDistance > 0 &&
		LastUsedGazePoint.bHasValue &&
		DistanceInMM(GazePoint.Value, LastUsedGazePoint.Value, 1 / GetApproximatePixelsPerMillimeter()) < MinUpdateDistance)
	{
		// Too close to previous point: ignore.
		return;
	}

	LastUsedGazePoint = GazePoint;
	if (LastFocusedActor)
	{
		LastFocusedActor->bTraceHit = false;
	}

	FHitResult HitResult;
	AEyeXActorBase *FocusedActor = FindFocusedActor(HitResult, GazePoint.Value);
	LastGazeHitResult = HitResult;

	if (FocusedActor)
	{
		LastFocusedActor = FocusedActor;
		LastFocusedActor->bTraceHit = true;
	}
}

void AEyeXPlayerController::GetGazeHitResult(FVector& ImpactPoint, AActor*& HitActor, bool& bIsValid) const
{
	ImpactPoint = LastGazeHitResult.ImpactPoint;
	HitActor = LastGazeHitResult.GetActor();
	bIsValid = HitActor ? true : false;
}

AEyeXActorBase* AEyeXPlayerController::FindFocusedActor(FHitResult& OutHit, const FVector2D& GazePoint)
{
	// Get the scene view to deproject the gazepoint to world space
	ULocalPlayer* LocalPlayer = Cast<ULocalPlayer>(Player);

	if (!LocalPlayer)
		return nullptr;

	FSceneViewFamily ViewFamily(FSceneViewFamily::ConstructionValues(
		LocalPlayer->ViewportClient->Viewport,
		GetWorld()->Scene,
		LocalPlayer->ViewportClient->EngineShowFlags)
		.SetRealtimeUpdate(true));

	FVector ViewLocation;
	FRotator ViewRotation;
	FSceneView* View = LocalPlayer->CalcSceneView(&ViewFamily, /*out*/ ViewLocation, /*out*/ ViewRotation, LocalPlayer->ViewportClient->Viewport);

	// Initiate sweep/trace variables
	const FCollisionObjectQueryParams ObjectParams(FEyeXUtils::ECCArrayToBitField(CollisionChannels));
	const FCollisionQueryParams TraceParams(FName(TEXT("GazeTrace")), true, GetPawn());
	FHitResult HitResult;
	AEyeXActorBase* EyeXActor = nullptr;

	switch (DetectionMode)
	{
	case EEyeXDetectionMode::LineTrace:
		EyeXActor = FindByLineTrace(HitResult, View, GazePoint, ObjectParams, TraceParams);
		break;
	case EEyeXDetectionMode::BoxedLineTrace:
		EyeXActor = FindByBoxedLineTrace(HitResult, View, GazePoint, ObjectParams, TraceParams);
		break;
	case EEyeXDetectionMode::Sweep:
		EyeXActor = FindBySweep(HitResult, View, GazePoint, ObjectParams, TraceParams);
		break;
	case EEyeXDetectionMode::FrustrumIntersection:
		EyeXActor = FindByFrustumIntersection(HitResult, View, GazePoint, ObjectParams, TraceParams);
		break;
	default:
		break;
	}

	OutHit = HitResult; 
	return EyeXActor; // use out param for actor as well, alternatively use hit actor in hit result (with cast). make the method const too.
}

AEyeXActorBase* AEyeXPlayerController::FindByLineTrace(FHitResult& OutHit, const FSceneView* const View, const FVector2D& GazePoint, 
	const FCollisionObjectQueryParams& ObjectParams, const FCollisionQueryParams& TraceParams)
{
	UWorld* World = GetWorld();
	if (!World) return nullptr;

	AEyeXActorBase* EyeXActor = nullptr;
	FVector Start, End;
	FEyeXUtils::GetStartAndEndOfLineTrace(View, MaxDistance, GazePoint, /*out*/ Start, /*out*/ End);
	if (World->LineTraceSingleByObjectType(OutHit, Start, End, ObjectParams, TraceParams))
	{
		EyeXActor = Cast<AEyeXActorBase>(OutHit.GetActor());
	}

	VisualizeHit(bVisualizeDetection, World, OutHit);
	VisualizeGazePoint(bVisualizeDetection, World, Start);

	return EyeXActor;
}

AEyeXActorBase* AEyeXPlayerController::FindByBoxedLineTrace(FHitResult& OutHit, const FSceneView* const View, const FVector2D& GazePoint,
	const FCollisionObjectQueryParams& ObjectParams, const FCollisionQueryParams& TraceParams)
{
	UWorld* World = GetWorld();
	if (!World) return nullptr;

	// Set up a box around the gaze point
	FVector2D Corners[4];
	GetBoxCorners(GazePoint, BoxSize * GetApproximatePixelsPerMillimeter(), Corners);

	// First check center point
	AEyeXActorBase* EyeXActor = nullptr;
	FVector Start, End;
	FEyeXUtils::GetStartAndEndOfLineTrace(View, MaxDistance, GazePoint, /*out*/ Start, /*out*/ End);
	if (World->LineTraceSingleByObjectType(OutHit, Start, End, ObjectParams, TraceParams))
	{
		EyeXActor = Cast<AEyeXActorBase>(OutHit.GetActor());
	}
	else
	{
		// If center point missed we perform traces in a box around the gaze point
		// and choose the closest EyeXActor hit by the traces
		TArray<AEyeXActorBase*> EyeXActors;
		for (int i = 0; i < 4; ++i)
		{
			FVector BoxStart, BoxEnd;
			FEyeXUtils::GetStartAndEndOfLineTrace(View, MaxDistance, Corners[i], /*out*/ BoxStart, /*out*/ BoxEnd);
			if (World->LineTraceSingleByObjectType(OutHit, BoxStart, BoxEnd, ObjectParams, TraceParams))
			{
				AEyeXActorBase* Actor = Cast<AEyeXActorBase>(OutHit.GetActor());
				if (!Actor) continue;
				EyeXActors.Add(Actor);
			}

			VisualizeHitTestPoint(bVisualizeDetection, World, BoxStart);
		}

		if (EyeXActors.Num() > 0)
		{
			FEyeXUtils::ActorDistanceComparer Comparer(PlayerCameraManager);
			EyeXActors.Sort(Comparer);
			EyeXActor = EyeXActors[0];
		}
	}

	VisualizeHit(bVisualizeDetection, World, OutHit);
	VisualizeGazePoint(bVisualizeDetection, World, Start);

	return EyeXActor;
}

AEyeXActorBase* AEyeXPlayerController::FindBySweep(FHitResult& OutHit, const FSceneView* const View, const FVector2D& GazePoint,
	const FCollisionObjectQueryParams& ObjectParams, const FCollisionQueryParams& TraceParams)
{
	if (SweepIntervals <= 1)
	{
		UE_LOG(LogEyeX, Warning, TEXT("Invalid value for SweepIntervals: %i. Must be greater than 0."), SweepIntervals);
		return nullptr;
	}

	UWorld* World = GetWorld();
	if (!World) return nullptr;

	FVector Start, Direction;
	View->DeprojectFVector2D(GazePoint, Start, Direction);

	const float TanFOVScaled = GetTanOfFOVAngleScaled();

	// Perform sweeps
	const float DeltaDistance = MaxDistance / SweepIntervals;
	const FVector DeltaDirection = DeltaDistance * Direction;
	float CurrentDistance = DeltaDistance / 2;
	FCollisionShape Shape;
	AEyeXActorBase* EyeXActor = nullptr;
	for (int i = 0; i < SweepIntervals; ++i)	
	{
		const FVector End = Start + DeltaDirection;
		const float Radius = (i == 0) ? 0.0f : TanFOVScaled * CurrentDistance; // Depends on the view frustrum, size of the screen and the distance.
		Shape.SetSphere(Radius);

		if (World->SweepSingleByObjectType(OutHit, Start, End, FQuat::Identity, ObjectParams, Shape, TraceParams))
		{
			EyeXActor = Cast<AEyeXActorBase>(OutHit.GetActor());
			break;
		}

		Start = End;
		CurrentDistance += DeltaDistance;
	}
	
	VisualizeHit(bVisualizeDetection, World, OutHit, Shape.GetSphereRadius());
	VisualizeGazePoint(bVisualizeDetection, World, Start);

	return EyeXActor;
}

AEyeXActorBase* AEyeXPlayerController::FindByFrustumIntersection(FHitResult& OutHit, const FSceneView* const View, const FVector2D& GazePoint,
	const FCollisionObjectQueryParams& ObjectParams, const FCollisionQueryParams& TraceParams)
{
	UWorld* World = GetWorld();
	if (!World) return nullptr;

	FVector2D Corners[4];
	GetBoxCorners(GazePoint, BoxSize * GetApproximatePixelsPerMillimeter(), Corners);

	// First do a ray cast from the gaze point to determine if something is blocking.
	// If we happen to find an AEyeXActorBase, we're done
	FVector Start, End;
	FEyeXUtils::GetStartAndEndOfLineTrace(View, MaxDistance, GazePoint, /*out*/ Start, /*out*/ End);

	AEyeXActorBase* EyeXActor = nullptr;
	if (World->LineTraceSingleByObjectType(OutHit, Start, End, ObjectParams, TraceParams))
	{
		EyeXActor = Cast<AEyeXActorBase>(OutHit.GetActor());
	}
	else
	{
		// Calculate frustum using the SceneView
		FConvexVolume Frustum;
		FEyeXMathHelpers::CalculateFrustum(Corners, View, Frustum);

		// Check frustum intersection with EyeX Actors
		TArray<AEyeXActorBase*> EyeXActors;
		GetEyeXActorsSortedByDistance(EyeXActors);
		for (AEyeXActorBase* Actor : EyeXActors)
		{
			if (FEyeXMathHelpers::IntersectsFrustum(OutHit, Actor, Frustum))
			{
				EyeXActor = Actor;
				break;
			}
		}
	}

	VisualizeHit(bVisualizeDetection, World, OutHit);
	VisualizeGazePoint(bVisualizeDetection, World, Start);

	return EyeXActor;
}

void AEyeXPlayerController::GetBoxCorners(const FVector2D& Center, const FVector2D& Size, FVector2D Corners[])
{
	auto HalfSize = Size / 2;
	Corners[0] = FVector2D(Center.X - HalfSize.X, Center.Y - HalfSize.Y); // Upper Left Corner	
	Corners[1] = FVector2D(Center.X + HalfSize.X, Center.Y - HalfSize.Y); // Upper Right Corner
	Corners[2] = FVector2D(Center.X + HalfSize.X, Center.Y + HalfSize.Y); // Lower Right Corner
	Corners[3] = FVector2D(Center.X - HalfSize.X, Center.Y + HalfSize.Y); // Lower Left Corner
}

void AEyeXPlayerController::GetEyeXActorsSortedByDistance(TArray<AEyeXActorBase*>& EyeXActorsOut) const
{
	UWorld* World = GetWorld();
	const float Time = World->GetTimeSeconds() - World->GetDeltaSeconds() - 0.1f; // Allow render time to be at least one frame behind.
	const float MaxDistanceSquared = MaxDistance * MaxDistance;
	FEyeXUtils::ActorDistanceComparer Comparer(PlayerCameraManager);
	for (TActorIterator<AEyeXActorBase> ActorItr(World); ActorItr; ++ActorItr)
	{
		if (ActorItr->GetLastRenderTime() < Time) // Ignore if the actor is not rendered to the screen
			continue;

		if (Comparer.DistanceSquaredToCamera(*ActorItr) > MaxDistanceSquared) // Ignore if further away than max distance
			continue;

		if (!FEyeXUtils::ActorBlocksOnChannel(*ActorItr, CollisionChannels)) // Ignore actors that aren't blocking any of the channels
			continue;

		EyeXActorsOut.Add(*ActorItr);
	}

	EyeXActorsOut.Sort(Comparer);
}

float AEyeXPlayerController::GetTanOfFOVAngleScaled() const
{
	FViewport* Viewport = FEyeXUtils::GetViewport();
	const FVector2D ViewportSize = Viewport->GetSizeXY();
	float ViewPortHeightInMm;
	float ScaleFactor;
	ViewPortHeightInMm = ViewportSize.Y / GetApproximatePixelsPerMillimeter();
	ScaleFactor = 2.0f * SweepSphereRadius / ViewPortHeightInMm;

	const float FOVRadians = FMath::DegreesToRadians(PlayerCameraManager->GetFOVAngle());
	const float TanFOV = FMath::Tan(0.5f * FOVRadians);

	return TanFOV * ScaleFactor;
}

float AEyeXPlayerController::DistanceInMM(const FVector2D& V, const FVector2D& W, float MmPerPixel)
{
	return FMath::Sqrt(FVector2D::DistSquared(V * MmPerPixel, W * MmPerPixel));
}

float AEyeXPlayerController::GetApproximatePixelsPerMillimeter() const
{
	if (EyeX)
	{
		auto DisplaySize = EyeX->GetDisplaySize();
		auto ScreenBounds = EyeX->GetScreenBounds();
		if (DisplaySize.bHasValue && DisplaySize.bHasValue)
		{
			return ScreenBounds.Value.Width / (float)DisplaySize.Value.X;
		}
	}

	return 4.0f; // Roughly 100 DPI: a reasonable default.
}
