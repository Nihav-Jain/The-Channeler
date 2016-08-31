// Copyright 2014 Tobii Technology AB. All Rights Reserved.

#include "EyeXPluginPrivatePCH.h"
#include "EyeXUtils.h"

float FEyeXUtils::CalculateRotationAngle(const FVector2D& Left, const FVector2D& Right)
{
	static const FVector2D Y = FVector2D(0.0f, 1.0f);
	FVector2D LeftToRight = Right - Left;
	LeftToRight.Normalize();
	const float Radians = FMath::Acos(LeftToRight | Y);
	return FMath::RadiansToDegrees(Radians) - 90.0f;
}

UGameViewportClient *FEyeXUtils::GetGameViewport()
{
	if (GEngine != nullptr)
	{
		return GEngine->GameViewport;
	}

	return nullptr;
}

FViewport *FEyeXUtils::GetViewport()
{
	auto gameViewport = GetGameViewport();
	if (gameViewport != nullptr)
	{
		return gameViewport->Viewport;
	}

	return nullptr;
}

FVector FEyeXUtils::EyeX3DPointToUnrealWorld(FVector EyeX3DPoint)
{
	// convert from millimeters to centimeters and juggle around the axes.
	// the EyeX coordinate system is right-handed with y pointing up, x to the right, and z pointing towards the user.
	// the Unreal coodinate system is left-handed with z pointing up, y to the right, and x pointing away from the user.
	return FVector(-0.1f * EyeX3DPoint.Z, 0.1f * EyeX3DPoint.X, 0.1f * EyeX3DPoint.Y);
}

TEyeXMaybeValue<FVector2D> FEyeXUtils::VirtualDesktopPixelToViewportPixel(FVector2D Point, FEyeXScreenBounds ScreenBounds)
{
	auto viewport = GetViewport();
	if (viewport == nullptr)
	{
		return TEyeXMaybeValue<FVector2D>(FVector2D::ZeroVector, false);
	}
	
	if (viewport->IsFullscreen())
	{
		auto viewportSize = viewport->GetSizeXY();
		FVector2D viewportPixels((Point.X / ScreenBounds.Width) * viewportSize.X, (Point.Y / ScreenBounds.Height) * viewportSize.Y);
		return TEyeXMaybeValue<FVector2D>(viewportPixels);
	}
	else
	{
		FIntPoint intPoint((int32)Point.X, (int32)Point.Y);
		FVector2D viewportPoint = viewport->VirtualDesktopPixelToViewport(intPoint);

		auto viewportSize = viewport->GetSizeXY();
		FVector2D viewportPixels(viewportPoint.X * viewportSize.X, viewportPoint.Y * viewportSize.Y);
		return TEyeXMaybeValue<FVector2D>(viewportPixels);
	}
}

FVector FEyeXUtils::VirtualDesktopPixelToEyeX3DPoint(FVector2D VirtualDesktopPoint, FVector2D DisplaySizeMm, FEyeXScreenBounds ScreenBoundsPixels)
{
	auto millimetersPerPixelX = DisplaySizeMm.X / ScreenBoundsPixels.Width;
	auto millimetersPerPixelY = DisplaySizeMm.Y / ScreenBoundsPixels.Height;

	// translate from origo-in-upper-left-corner to origo-in-center-of-screen and transform from pixels to millimeters.
	// the point is in the screen plane, so z == 0.
	return FVector(
		(VirtualDesktopPoint.X - ScreenBoundsPixels.Width / 2) * millimetersPerPixelX,
		-(VirtualDesktopPoint.Y - ScreenBoundsPixels.Height / 2) * millimetersPerPixelY,
		0);
}

int32 FEyeXUtils::ECCArrayToBitField(const TArray<TEnumAsByte<ECollisionChannel> >& ChannelList)
{
	int32 ChannelBitField = 0;
	for (ECollisionChannel Channel : ChannelList)
	{
		ChannelBitField |= ECC_TO_BITFIELD(Channel);
	}

	return ChannelBitField;
}

bool FEyeXUtils::ActorBlocksOnChannel(AActor* Actor, const TArray<TEnumAsByte<ECollisionChannel> >& Channels)
{
	for (ECollisionChannel Channel : Channels)
	{
		if (Actor->GetComponentsCollisionResponseToChannel(Channel) == ECollisionResponse::ECR_Block)
			return true;
	}

	return false;
}

void FEyeXUtils::GetStartAndEndOfLineTrace(const FSceneView *View, float MaxDistance, const FVector2D& GazePoint, FVector& OutStart, FVector& OutEnd)
{
	FVector Direction;
	View->DeprojectFVector2D(GazePoint, OutStart, Direction);
	OutEnd = OutStart + MaxDistance * Direction;
}
