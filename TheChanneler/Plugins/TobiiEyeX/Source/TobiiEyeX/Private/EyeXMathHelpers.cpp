// Copyright 2014 Tobii Technology AB. All Rights Reserved.

#include "EyeXPluginPrivatePCH.h"
#include "EyeXMathHelpers.h"

void FEyeXMathHelpers::CalculateFrustum(FVector2D Corners[], const FSceneView* const View, FConvexVolume& OutFrustum)
{
	// Add near and far plane first
	FPlane NearPlane;
	FPlane FarPlane;
	if (View->ViewProjectionMatrix.GetFrustumNearPlane(NearPlane))
	{
		OutFrustum.Planes.Add(NearPlane);
	}
	if (View->ViewProjectionMatrix.GetFrustumFarPlane(FarPlane))
	{
		OutFrustum.Planes.Add(FarPlane);
	}

	// Deproject the four corners of the selection box
	FVector WorldPoint[4];
	FVector WorldDir;
	for (int i = 0; i < 4; ++i)
	{
		View->DeprojectFVector2D(Corners[i], WorldPoint[i], WorldDir);
	}

	// Use the camera position and the selection box to create and add the bounding planes
	FVector CamPoint = View->ViewLocation;
	for (int i = 0; i < 4; i++)
	{
		OutFrustum.Planes.Add(FPlane(WorldPoint[i], WorldPoint[(i + 1) % 4], CamPoint));
	}
	OutFrustum.Init();
}

bool FEyeXMathHelpers::IntersectsFrustum(FHitResult& HitOut, AActor* InActor, const FConvexVolume& InFrustum)
{
	// Iterate over all actor components, selecting out primitive components
	TArray<UPrimitiveComponent*> Components;
	InActor->GetComponents(Components);

	for (int32 ComponentIndex = 0; ComponentIndex < Components.Num(); ++ComponentIndex)
	{
		UPrimitiveComponent* PrimitiveComponent = Components[ComponentIndex];

		if (!PrimitiveComponent->IsRegistered())
			continue;

		if (PrimitiveComponent->IsA(UDrawSphereComponent::StaticClass()) ||
			PrimitiveComponent->IsA(UNavLinkRenderingComponent::StaticClass()) ||
			PrimitiveComponent->IsA(UArrowComponent::StaticClass()))
		{
			continue;
		}

		FVector Origin = PrimitiveComponent->Bounds.Origin;
		FVector Extent;
		if (PrimitiveComponent->IsA(UBillboardComponent::StaticClass()) && Cast<UBillboardComponent>(PrimitiveComponent)->Sprite != nullptr)
		{
			// Use the size of the sprite itself rather than its box extent
			UBillboardComponent* SpriteComponent = Cast<UBillboardComponent>(PrimitiveComponent);
			const float Scale = SpriteComponent->ComponentToWorld.GetMaximumAxisScale();
			Extent = FVector(Scale * SpriteComponent->Sprite->GetSizeX() / 2.0f, Scale * SpriteComponent->Sprite->GetSizeY() / 2.0f, 0.0f);
		}
		else
		{
			Extent = PrimitiveComponent->Bounds.BoxExtent;
		}

		UBrushComponent* BrushComponent = Cast<UBrushComponent>(PrimitiveComponent);
		if (BrushComponent && BrushComponent->Brush)
		{
			// Need to check each vert of the brush separately
			auto Vertices = BrushComponent->Brush->VertexBuffer.Vertices;
			for (int32 VertexIndex = 0; VertexIndex < Vertices.Num(); ++VertexIndex)
			{
				const FModelVertex& Vertex = Vertices[VertexIndex];
				const FVector Location = BrushComponent->ComponentToWorld.TransformPosition(Vertex.Position);
				if (InFrustum.IntersectBox(Location, FVector::ZeroVector))
				{
					InitHitResult(HitOut, InActor, PrimitiveComponent, Location, FVector::ZeroVector, VertexIndex);
					return true;
				}
			}
		}
		else
		{
			if (InFrustum.IntersectBox(Origin, Extent))
			{
				if (!(PrimitiveComponent->IsA(UStaticMeshComponent::StaticClass()) || PrimitiveComponent->IsA(USkeletalMeshComponent::StaticClass())))
				{
					// Use the bounding box check for all other components
					InitHitResult(HitOut, InActor, PrimitiveComponent, FVector::ZeroVector, FVector::ZeroVector, 0);
					return true;
				}
				if (IntersectsVertices(HitOut, InActor, PrimitiveComponent, InFrustum))
				{
					return true;
				}
			}
		}
	}
	return false;
}

bool FEyeXMathHelpers::IntersectsVertices(FHitResult& HitOut, AActor* InActor, UPrimitiveComponent* InComponent, const FConvexVolume& InFrustum)
{
	UStaticMeshComponent* StaticMeshComponent = Cast<UStaticMeshComponent>(InComponent);
	USkeletalMeshComponent* SkeletalMeshComponent = Cast<USkeletalMeshComponent>(InComponent);

	if (StaticMeshComponent && StaticMeshComponent->StaticMesh)
	{
		check(StaticMeshComponent->StaticMesh->RenderData);
		if (StaticMeshComponent->StaticMesh->RenderData->LODResources.Num() > 0)
		{
			const FStaticMeshLODResources& LODModel = StaticMeshComponent->StaticMesh->RenderData->LODResources[0];
			uint32 NumVertices = LODModel.VertexBuffer.GetNumVertices();
			for (uint32 VertexIndex = 0; VertexIndex < NumVertices; ++VertexIndex)
			{
				const FVector& LocalPosition = LODModel.PositionVertexBuffer.VertexPosition(VertexIndex);
				const FVector& WorldPosition = StaticMeshComponent->ComponentToWorld.TransformPosition(LocalPosition);
				if (InFrustum.IntersectBox(WorldPosition, FVector::ZeroVector))
				{
					//FVector Normal = LODModel.VertexBuffer.VertexTangentZ(VertexIndex);
					InitHitResult(HitOut, InActor, InComponent, WorldPosition, FVector::ZeroVector, VertexIndex);
					return true;
				}
			}
		}
	}
	else if (SkeletalMeshComponent && SkeletalMeshComponent->MeshObject)
	{
		FSkeletalMeshResource* SkelMeshResource = SkeletalMeshComponent->GetSkeletalMeshResource();
		check(SkelMeshResource);
		check(SkelMeshResource->LODModels.Num() > 0);

		const FStaticLODModel& LODModel = SkelMeshResource->LODModels[0];
		for (int32 ChunkIndex = 0; ChunkIndex < LODModel.Chunks.Num(); ++ChunkIndex)
		{
			const FSkelMeshChunk& Chunk = LODModel.Chunks[ChunkIndex];
			for (int32 VertexIndex = 0; VertexIndex < Chunk.RigidVertices.Num(); ++VertexIndex)
			{
				FRigidSkinVertex Vertex = Chunk.RigidVertices[VertexIndex];
				const FVector Location = SkeletalMeshComponent->ComponentToWorld.TransformPosition(Vertex.Position);
				if (InFrustum.IntersectBox(Location, FVector::ZeroVector))
				{
					FName BoneName = SkeletalMeshComponent->GetBoneName(Vertex.Bone);
					//FVector Normal = Vertex.TangentZ;
					InitHitResult(HitOut, InActor, InComponent, Location, FVector::ZeroVector, VertexIndex, BoneName);
					return true;
				}
			}

			for (int32 VertexIndex = 0; VertexIndex < Chunk.SoftVertices.Num(); ++VertexIndex)
			{
				FSoftSkinVertex Vertex = Chunk.SoftVertices[VertexIndex];
				const FVector Location = SkeletalMeshComponent->ComponentToWorld.TransformPosition(Vertex.Position);
				if (InFrustum.IntersectBox(Location, FVector::ZeroVector))
				{
					//FVector Normal = Vertex.TangentZ;
					InitHitResult(HitOut, InActor, InComponent, Location, FVector::ZeroVector, VertexIndex);
					return true;
				}
			}
		}
	}

	return false;
}

void FEyeXMathHelpers::InitHitResult(FHitResult& HitOut, AActor* Actor, UPrimitiveComponent* Component, const FVector& Location, const FVector& Normal, int32 FaceIndex, const FName& BoneName)
{
	HitOut.bBlockingHit = true;
	HitOut.Actor = Actor;
	HitOut.Component = Component;
	HitOut.ImpactPoint = Location;
	HitOut.Location = Location;
	HitOut.ImpactNormal = Normal;
	HitOut.FaceIndex = FaceIndex;
	HitOut.BoneName = BoneName;
}