// Copyright 2014 Tobii Technology AB. All Rights Reserved.

#pragma once

class FEyeXUtils
{
public:
	static float CalculateRotationAngle(const FVector2D& Left, const FVector2D& Right);

	static UGameViewportClient *GetGameViewport();
	static FViewport *GetViewport();

	static FVector EyeX3DPointToUnrealWorld(FVector EyeX3DPoint);
	static TEyeXMaybeValue<FVector2D> VirtualDesktopPixelToViewportPixel(FVector2D Point, FEyeXScreenBounds ScreenBounds);
	static FVector VirtualDesktopPixelToEyeX3DPoint(FVector2D VirtualDesktopPoint, FVector2D DisplaySizeMm, FEyeXScreenBounds ScreenBoundsPixels);

	static int32 ECCArrayToBitField(const TArray<TEnumAsByte<ECollisionChannel> >& ChannelList);
	static bool ActorBlocksOnChannel(AActor* Actor, const TArray<TEnumAsByte<ECollisionChannel> >& Channels);

	static void GetStartAndEndOfLineTrace(const FSceneView *View, float MaxDistance, const FVector2D& GazePoint, FVector& StartOut, FVector& EndOut);

	class ActorDistanceComparer
	{
	public:
		ActorDistanceComparer(const APlayerCameraManager *CameraManager)
			: CameraManager(CameraManager)
		{}

		float DistanceSquaredToCamera(const AActor *Actor) const
		{
			const FVector CameraLocation = CameraManager->GetCameraLocation();
			return FVector::DistSquared(Actor->GetActorLocation(), CameraLocation);
		}

		bool operator ()(const AActor& Actor1, const AActor& Actor2) const
		{
			return DistanceSquaredToCamera(&Actor1) < DistanceSquaredToCamera(&Actor2);
		}

	private:
		const APlayerCameraManager *CameraManager;
	};
};
