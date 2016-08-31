// Copyright 2014 Tobii Technology AB. All Rights Reserved.

#pragma once

#include "IEyeXPlugin.h"
#include "EyeXTypes.h"
#include "EyeXPlayerController.generated.h"

class AEyeXActorBase;

/**
 * Describes the available strategies for detecting actors being looked at.
 * You may want to use different strategies in different scenes based on factors such as actor 
 * geometry and how crowded the scene is.
 * There are also performance trade-offs to be made.
 *
 * None: Actor detection is disabled.
 * LineTrace: Performs a line trace to find a focused actor. Fast, but actors may need to be 
 *   surrounded in volumes, otherwise the gaze point will often miss the object.
 * BoxedLineTrace: Starts with a basic line trace. If no actors are found, does additional line 
 *   traces in a box shape around the gaze point.
 * Sweep: Performs multiple sweeps, trying to keep the size of the swept sphere to a fixed size in 
 *   mm. Has an easier time hitting the object the user is looking at but is more expensive.
 * FrustrumIntersection: Calculates a view frustrum defined by GazeBoxExtents (a box in screen 
 *   space) and checks for intersection with the frustum. Should be pretty accurate but can be slow
 *   if the scene contains many EyeXActors with a lot of vertices. Since Frustum Intersection only 
 *   checks EyeXActorBases it can't be used to get a Hit from other actor types.
 */
UENUM(BlueprintType)
namespace EEyeXDetectionMode
{
	enum Type
	{
		None					UMETA(DisplayName = "None"),
		LineTrace				UMETA(DisplayName = "LineTrace"),
		BoxedLineTrace			UMETA(DisplayName = "BoxedLineTrace"),
		Sweep					UMETA(DisplayName = "Sweep"),
		FrustrumIntersection	UMETA(DisplayName = "FrustrumIntersection")
	};
}

/**
 * The EyeX player controller is used to enable interaction with actors derived from AEyeXActorBase. 
 * If you only want to use gaze data or eye position data, etc., you can ignore this class.
 *
 * There are several strategies for detecting actors being looked at:
 *
 * The simplest is Line Trace which simply shoots a ray from the gaze point. This is fast
 * but actors should be surrounded with collision volumes in order to get better interaction.
 *
 * When using Sweep mode the size of the sphere is calculated to fit a given dimension
 * in millimeters, effectively triggering objects in an area around the gaze point. This enables
 * better interaction since the gaze point often times contain noise or offsets.
 * This can however be expensive performance wise so if there are a lot of objects
 * in the scene a simple line trace might be better. 
 * 
 * Frustum Intersection calculates a frustum from a box in screen space given in millimeters and
 * checks for intersection with actors. This can give good interaction but can be costly with
 * many actors in the scene, especially if these actors have many vertices.
 */
UCLASS(Blueprintable, BlueprintType)
class TOBIIEYEX_API AEyeXPlayerController : public APlayerController
{
	GENERATED_UCLASS_BODY()
public:
	/** 
	 * Choice of method for detecting gaze actors being looked at. The default is LineTrace.
	 * None: Actor detection is disabled.
	 * LineTrace: Performs a simple line trace to find a focused actor. Fast, but actors may need to be
	 *   surrounded in volumes, otherwise the gaze point will often miss the object.
	 * BoxedLineTrace: Starts with a basic line trace. If no actors are found, does additional line
	 *   traces in a box shape around the gaze point.
	 * Sweep: Performs multiple sweeps, trying to keep the size of the swept sphere to a fixed size in
	 *   mm. Has an easier time hitting the object the user is looking at but is more expensive.
	 * FrustrumIntersection: Calculates a view frustrum defined by GazeBoxExtents (a box in screen
	 *   space) and checks for intersection with the frustum. Should be pretty accurate but can be slow
	 *   if the scene contains many EyeXActors with a lot of vertices. Since Frustum Intersection only
	 *   checks EyeXActorBases it can't be used to get a Hit from other actor types.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "EyeX")
	TEnumAsByte<EEyeXDetectionMode::Type> DetectionMode;

	/** 
	* Choice of data mode for gaze interaction. The default is LightlyFiltered.
	* Unfiltered: Unfiltered data from the eye tracker. The most responsive mode but can jitter a lot.
	* LightlyFiltered: A light filtering applied by the EyeX Engine. Less jittering but also slightly less responsive.
	*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "EyeX")
	TEnumAsByte<EEyeXGazePointDataMode::Type> GazePointMode;

	/** The maximum distance from the camera that an object can be detected. Lower values can increase performance.*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "EyeX")
	float MaxDistance;

	/** Determines which channels to perform the detection on. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "EyeX")
	TArray<TEnumAsByte<ECollisionChannel> > CollisionChannels;

	/** The minimum distance in mm the latest gaze point has to differ from the previous
	gaze point in order to perform a new detection. Set to 0 to always perform a detection. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "EyeX")
	float MinUpdateDistance;

	/** Set to true to visualize the detection process. Disabled for shipping builds. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "EyeX")
	bool bVisualizeDetection;

	/** Used when Detection Mode is set to Sweep. The radius of the sphere used to perform sweeps. The value is given in real world mm's. The
	sweep system tries to keep the size of the sphere constant on the screen regardless of distance. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "EyeX")
	float SweepSphereRadius;

	/** Used when Detection Mode is set to Sweep. The number of intervals that the distance is split into. At each new interval the size of
	the sphere is recalculated to try to keep the sphere at the same size on the screen. A higher value
	will lead to a more constant gaze area size but may be costly performance wise. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "EyeX")
	int32 SweepIntervals;

	/** Used when DetectionMode is set to BoxedLineTrace or FrustrumIntersection.
	Determines how large the box around the gaze point is in mm. This box is used to calculate the frustum. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "EyeX")
	FVector2D BoxSize;

public:

	virtual void Tick(float DeltaSeconds) override;
	virtual void BeginPlay() override;

	/**
	 * Returns the Impact Point, Hit Actor and validity of the HitResult of the latest sweep from the gaze point. 
	 * When using Focal Intersection as detection mode the result will not be as accurate.
	 */
	UFUNCTION(BlueprintPure, Category = "EyeX")
	void GetGazeHitResult(FVector& ImpactPoint, AActor*& HitActor, bool& bIsValid) const;

private:

	AEyeXActorBase* FindFocusedActor(FHitResult& OutHit, const FVector2D& GazePoint);
	AEyeXActorBase* FindByLineTrace(FHitResult& OutHit, const FSceneView* const View, const FVector2D& GazePoint, const FCollisionObjectQueryParams& ObjectParams,
		const FCollisionQueryParams& TraceParams);
	AEyeXActorBase* FindByBoxedLineTrace(FHitResult& OutHit, const FSceneView* const View, const FVector2D& GazePoint, const FCollisionObjectQueryParams& ObjectParams, 
		const FCollisionQueryParams& TraceParams);
	AEyeXActorBase* FindBySweep(FHitResult& OutHit, const FSceneView* const View, const FVector2D& GazePoint, const FCollisionObjectQueryParams& ObjectParams,
		const FCollisionQueryParams& TraceParams);
	AEyeXActorBase* FindByFrustumIntersection(FHitResult& OutHit, const FSceneView* const View, const FVector2D& GazePoint, const FCollisionObjectQueryParams& ObjectParams,
		const FCollisionQueryParams& TraceParams);

	// Used with sweep
	float GetTanOfFOVAngleScaled() const;

	// Used with frustum intersection
	void GetEyeXActorsSortedByDistance(TArray<AEyeXActorBase*>& EyeXActorsOut) const;

	static void GetBoxCorners(const FVector2D& Center, const FVector2D& Size, FVector2D Corners[]);
	static float DistanceInMM(const FVector2D& V, const FVector2D& W, float MmPerPixel);
	float GetApproximatePixelsPerMillimeter() const;

private:

	IEyeXPlugin* EyeX;
	FEyeXGazePoint LastUsedGazePoint;
	FHitResult LastGazeHitResult;
	AEyeXActorBase* LastFocusedActor;
};
