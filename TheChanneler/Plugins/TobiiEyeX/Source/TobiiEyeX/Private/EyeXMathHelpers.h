// Copyright 2014 Tobii Technology AB. All Rights Reserved.

#pragma once

#include "StaticMeshResources.h"

/** 
 * A static class to provide some math functions.
 */

class FEyeXMathHelpers
{
public:

	static void CalculateFrustum(FVector2D Corners[], const FSceneView* const View, FConvexVolume& OutFrustum);
	static bool IntersectsFrustum(FHitResult& HitOut, AActor* InActor, const FConvexVolume& InFrustum);
	static bool IntersectsVertices(FHitResult& HitOut, AActor* InActor, UPrimitiveComponent* InComponent, const FConvexVolume& InFrustum);

private:

	FEyeXMathHelpers() {};

	static void InitHitResult(FHitResult& HitOut, AActor* Actor, UPrimitiveComponent* Component, const FVector& Location, const FVector& Normal, int32 FaceIndex, const FName& BoneName = TEXT(""));
};