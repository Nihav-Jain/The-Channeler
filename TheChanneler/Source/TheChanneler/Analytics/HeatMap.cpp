// Fill out your copyright notice in the Description page of Project Settings.

#include "TheChanneler.h"
#include "HeatMap.h"

#include "Components/StaticMeshComponent.h"
#include "Runtime/Engine/Public/StaticMeshResources.h"

// Includes for the Visual Logger
#include "Paths.h"
#include "VisualLogger.h"
#include "Characters/ChannelerCharacter.h"
#include "CoreMisc.h"

// Includes for the PhysX Mesh Data Function
#include "PhysXIncludes.h"
#include "PhysicsPublic.h"
#include "PhysXPublic.h"    //PtoU conversions
#include "ThirdParty/PhysX/PhysX-3.3/include/geometry/PxTriangleMesh.h"
#include "ThirdParty/PhysX/PhysX-3.3/include/foundation/PxSimpleTypes.h"
#include "PhysicsEngine/BodySetup.h"

TMap<FString, FString> UHeatMap::VLogFileTracker;
//UWorld* UHeatMap::worldRef = nullptr;

UHeatMap::UHeatMap() : HeatMapDataTable(), bIsRecordingToFile(false), FolderPath(), LevelName() {}
UHeatMap::~UHeatMap() {}

#pragma region helperfunctions

// Retrieves Vector
bool UHeatMap::MeshData(UStaticMeshComponent* StaticMeshComponent, TArray<FVector>& OutVertices, TArray<int32>& OutIndices)
{
	if (!StaticMeshComponent) return false;
	if (!(StaticMeshComponent->IsValidLowLevel())) return false;

	//Body Setup valid?
	UBodySetup* BodySetup = StaticMeshComponent->GetBodySetup();

	if (!BodySetup || !(BodySetup->IsValidLowLevel()))
	{
		return false;
	}

	//Component Transform
	FTransform RV_Transform = StaticMeshComponent->GetComponentTransform();

	for (PxTriangleMesh* TriMesh : BodySetup->TriMeshes)
	{
		if (!TriMesh)
		{
			return false;
		}

		// Get Vertex Buffer
		auto VertexCount = TriMesh->getNbVertices();
		const PxVec3* Vertices = TriMesh->getVertices();
		for (PxU32 v = 0; v < VertexCount; ++v)
		{
			// For each vertex, transform the position to match the component Transform 
			OutVertices.Add(RV_Transform.TransformPosition(P2UVector(Vertices[v])));
		}

		// Get Index Buffer
		int32 I0, I1, I2;
		int32 TriCount = TriMesh->getNbTriangles();
		const void* Triangles = TriMesh->getTriangles();
		for (int32 TriIndex = 0; TriIndex < TriCount; ++TriIndex)
		{
			if (TriMesh->getTriangleMeshFlags() & PxTriangleMeshFlag::eHAS_16BIT_TRIANGLE_INDICES)
			{
				PxU16* P16BitIndices = (PxU16*)Triangles;
				I0 = P16BitIndices[(TriIndex * 3) + 0];
				I1 = P16BitIndices[(TriIndex * 3) + 1];
				I2 = P16BitIndices[(TriIndex * 3) + 2];
			}
			else
			{
				PxU32* P32BitIndices = (PxU32*)Triangles;
				I0 = P32BitIndices[(TriIndex * 3) + 0];
				I1 = P32BitIndices[(TriIndex * 3) + 1];
				I2 = P32BitIndices[(TriIndex * 3) + 2];
			}

			// For each face/triangle, store the indices that make the vertex into the index buffer
			OutIndices.Add(I0);
			OutIndices.Add(I1);
			OutIndices.Add(I2);
		}
	}
	//UE_LOG(LogTemp, Warning, TEXT("VLOG: Mesh Data Retrieved: Original (%d vertices, %d indices) Actual (%d vertices, %d indices)"), VertexBuffer->GetNumVertices(), IndexBuffer->GetNumIndices(), OutVertices.Num(), OutIndices.Num());
	return true;
}

// Resource for Text File creation:
// https://answers.unrealengine.com/questions/27254/writing-to-text-files.html
// https://wiki.unrealengine.com/index.php?title=File_Management,_Create_Folders,_Delete_Files,_and_More&action=edit
bool UHeatMap::SaveStringTextToFile(const FString& SaveDirectory, const FString& JoyfulFileName, const FString& SaveText, bool AllowOverWriting)
{
	//Does the Directory Exists?
	if (!FPlatformFileManager::Get().GetPlatformFile().DirectoryExists(*SaveDirectory))
	{
		//create directory if it not exist
		bool result = FPlatformFileManager::Get().GetPlatformFile().CreateDirectoryTree(*SaveDirectory);

		if (!result)
		{
			LOG("VLOG: Error, Directory Not Created!");
			return false;
		}
	}

	//Get complete file path
	FString FileDirectory = FString::Printf(TEXT("%s/%s"), *SaveDirectory, *JoyfulFileName);

	//No over-writing?
	if (!AllowOverWriting)
	{
		//Check if file exists already
		if (FPlatformFileManager::Get().GetPlatformFile().FileExists(*FileDirectory))
		{
			//no overwriting
			return false;
		}
	}

	return FFileHelper::SaveStringToFile(SaveText, *FileDirectory);
}


void UHeatMap::AssignChannelerCharacter(const AChannelerCharacter* Channeler)
{
	Player = Channeler;
}

bool UHeatMap::SaveHeatMapResults(bool DoForceClear)
{
	// Create the CSV File
	FString csvText;
	csvText.Append(TEXT("ObjectID,Number Of Hits\n"));

	// Save the data in CSV format
	for (auto& pair : HeatMapDataTable)
	{
		csvText.Append(FString::Printf(TEXT("%s,%.2f\n"), *pair.Key, pair.Value.TimeViewed));
	}

	// Create FileName
	FString JoyfulFileName;
	JoyfulFileName.Append(FDateTime::Now().ToString());
	JoyfulFileName.Append(TEXT(".csv"));

	// Determine If Saved
	bool hasSavedResult = SaveStringTextToFile(FolderPath, JoyfulFileName, csvText);
	if (DoForceClear || hasSavedResult)
	{
		HeatMapDataTable.Empty();
	}
	return hasSavedResult;
}


void UHeatMap::TrackVLogFiles(const FString& CurrentLevelName)
{
#if ENABLE_VISUAL_LOG
	// Get constant values
	static const FString LogFileDirectory = FString::Printf(TEXT("%slogs/"), *FPaths::GameSavedDir());
	static const FString VLogExtension("bvlog");

	// Set Up Directory Iterator
	TArray<FString> directoriesToSkip;
	IPlatformFile &PlatformFile = FPlatformFileManager::Get().GetPlatformFile();
	FLocalTimestampDirectoryVisitor Visitor(PlatformFile, directoriesToSkip, directoriesToSkip, false);
	PlatformFile.IterateDirectory(*LogFileDirectory, Visitor);

	// Iterate through directory
	for (TMap<FString, FDateTime>::TIterator TimestampIt(Visitor.FileTimes); TimestampIt; ++TimestampIt)
	{
		const FString filePath = TimestampIt.Key();
		const FString fileName = FPaths::GetCleanFilename(filePath);

		// Check File Extension
		if (FPaths::GetExtension(fileName).Equals(VLogExtension, ESearchCase::IgnoreCase)
		&& !VLogFileTracker.Contains(filePath))
		{
			// Map level name to vlog file
			VLogFileTracker.Add(filePath, CurrentLevelName);
		}
	}
#endif
}


//Set the Recording state of the HeatMap
void UHeatMap::SetIsRecordingToFile(bool InIsRecording)
{
#if ENABLE_VISUAL_LOG
	FVisualLogger& vlogger = FVisualLogger::Get();
	if (InIsRecording)
	{
		// Clean Up current Visual Logger Recording
		vlogger.SetIsRecording(false);
		vlogger.Cleanup();
	}

	// Set the State of Recording to File
	vlogger.SetIsRecordingToFile(InIsRecording);
	bIsRecordingToFile = InIsRecording;

	if (!InIsRecording)
	{
		// Stop Visual Logger Recording
		vlogger.SetIsRecording(false);
		vlogger.Cleanup();

		// Save Stored Heat Map Results and Clear results
		bool result = SaveHeatMapResults(true);
		if (!result)
		{
			LOG("VLOG: Failed to Save Heat Map");
		}
	}
#endif
}

#pragma endregion helperfunctions

// Start the HeatMap
void UHeatMap::Activate()
{
#if ENABLE_VISUAL_LOG
	// Stop Heat Map Recording if it is currently running
	if (bIsRecordingToFile)
	{
		Deactivate();
		bIsRecordingToFile = false; // Not necessary, but here just in case
	}

	// Begin Visual Logger Recording
	UHeatMap::SetIsRecordingToFile(true);
	bIsRecordingToFile = true;
#endif
}

// Map Visual Loggers generated to the Level
void UHeatMap::OnLevelChanged(UWorld * world, ULevel * level, const FString & name) 
{
	// Stop any current HeatMap recordings
	Deactivate();
	// Associate VLog Files with current level (LevelName is empty string if just starting)
	TrackVLogFiles(LevelName);
	// Save Folder Path and Data
	FolderPath = FString::Printf(TEXT("%s/%s/csv"), *(GetModuleLogPath()), *name);
	LevelName = name;
}

void UHeatMap::DrawMeshInVLog(const AActor * Actor, const FString& ActorName, const FColor & Color)
{
#if ENABLE_VISUAL_LOG
	// Visualize Object being looked at
	UE_VLOG_LOCATION(Player, LogActor, Verbose, Actor->GetActorLocation(), 50, Color, TEXT("Object Name: %s"), *ActorName);

	// Get All Static Meshes
	TArray<UStaticMeshComponent*> Components;
	Actor->GetComponents<UStaticMeshComponent>(Components);
	for (int32 i = 0; i<Components.Num(); i++)
	{
		UStaticMeshComponent* StaticMeshComponent = Components[i];
		UStaticMesh* StaticMesh = StaticMeshComponent->StaticMesh;

		TArray<FVector> vertices;
		TArray<int32> indices;
		UHeatMap::MeshData(StaticMeshComponent, vertices, indices);

		FVisualLogger::GeometryShapeLogf(Player, LogActor, ELogVerbosity::Verbose, INDEX_NONE, vertices, indices, Color, TEXT("Object Name: %s"), *ActorName);
		
		//Above code identical to: 
		//UE_VLOG_MESH(Player, LogActor, Verbose, vertices, indices, FColor::Green, TEXT(""));
	}
#endif
}

void UHeatMap::LogActorInHeatMap(const AActor * Actor, const FColor & Color)
{
#if ENABLE_VISUAL_LOG
	// Assert that a ChannelerCharacter Exists 
	if (Player != nullptr)
	{
		// Get Actor Info
		FString actorName = Actor->GetName();
		FActorInfo& actorInfo = HeatMapDataTable.FindOrAdd(actorName);

		// Draw Actor to VLog
		DrawMeshInVLog(Actor, actorName, Color);
		actorInfo.IsDrawnThisFrame = true;
	}
#endif
}

// Store the HeatMap Data. Should Only be called by the Channeler
void UHeatMap::StoreHeatMapData(const AActor* ActorHit, float DeltaTime)
{
#if ENABLE_VISUAL_LOG
	// Assert that a ChannelerCharacter Exists, An Actor is Hit, and the HeatMap is being recorded
	if (UHeatMap::IsHeatMapRecording() && Player != nullptr && ActorHit != nullptr)
	{
		// Get Respective Object's Time Viewed
		FString actorName = ActorHit->GetName();
		FActorInfo& actorInfo = HeatMapDataTable.FindOrAdd(actorName);

		// Visualize Object being looked at
		DrawMeshInVLog(ActorHit, actorName, FColor::Green);

		// Visualize Player's current position
		FVector hitLocation = Player->GetActorLocation();
		UE_VLOG(Player, LogActor, Verbose, TEXT("Player Location: (%f, %f, %f)"), hitLocation.X, hitLocation.Y, hitLocation.Z);

		// Object ID Exists, update HeatMap data
		actorInfo.TimeViewed += DeltaTime;
		actorInfo.IsDrawnThisFrame = true;
	}
#endif
}

// Add Actor to be Tracked
void UHeatMap::TrackActorInHeatMap(const AActor * ActorToTrack)
{
#if ENABLE_VISUAL_LOG
	FActorInfo& actorInfo = HeatMapDataTable.FindOrAdd(ActorToTrack->GetName());
	actorInfo.ActorRef = ActorToTrack;
	actorInfo.TrackActor = true;
#endif
}

// Remove Actor to be Tracked
void UHeatMap::UntrackActorInHeatMap(const AActor * ActorToNotTrack)
{
#if ENABLE_VISUAL_LOG
	FActorInfo* actorInfo = HeatMapDataTable.Find(ActorToNotTrack->GetName());
	if (actorInfo != nullptr)
	{	// Set Actor to be untracked
		actorInfo->TrackActor = false;
	}
#endif
}


// Update the logging info of tracked actors
void UHeatMap::Update(float deltaTime)
{
	for (auto& pair : HeatMapDataTable)
	{
		FActorInfo& actorInfo = pair.Value;
		if (actorInfo.TrackActor && !actorInfo.IsDrawnThisFrame)
		{	// Log Actor Tracking if not Seen
			LogActorInHeatMap(actorInfo.ActorRef, FColor::Yellow);
		}

		// Reset drawn-on-frame check
		actorInfo.IsDrawnThisFrame = false;
	}
}

// Deactivate the HeatMap
void UHeatMap::Deactivate()
{
	if (bIsRecordingToFile)
	{
		// End Visual Logger Recording
		SetIsRecordingToFile(false);
	}
}

// Clean Up by moving the Visual Logger files to their respective Level Folders
void UHeatMap::CleanUp()
{
	const FString& ModuleLogPath = GetModuleLogPath();
	IFileManager& FileManager = IFileManager::Get();

	// Associate VLog Files with current level (LevelName is empty string if just starting)
	TrackVLogFiles(LevelName);

	// Begin Saving VLog Data into their Respective Folders
	for (auto& VLogInfo : VLogFileTracker)
	{
		// Get Level Name
		const FString& LevelNameForVLog = VLogInfo.Value;
		if (!LevelNameForVLog.IsEmpty())
		{
			// Get Original File Path
			const FString& OriginalVLogFilePath = VLogInfo.Key;

			// Get Analytic Module's File Path
			const FString VLogFileName = FPaths::GetCleanFilename(OriginalVLogFilePath);
			const FString ModuleLogFilePath = FString::Printf(TEXT("%s/%s/vlog/%s"), *ModuleLogPath, *LevelNameForVLog, *VLogFileName);

			LOG("VLOG: %s mapped to %s", *VLogFileName, *LevelNameForVLog);

			// Move Files from Original to Analytic's Path
			if (!FileManager.Move(*ModuleLogFilePath, *OriginalVLogFilePath, true))
			{
				LOG("VLOG: Failed to Move Visual Logger File! \"%s\". To have been moved to \"%s\"", *VLogFileName, *ModuleLogFilePath);
			}
		}
	}
}
