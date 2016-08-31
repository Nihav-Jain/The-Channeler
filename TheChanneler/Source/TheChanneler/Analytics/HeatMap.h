// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Object.h"

#include "AnalyticsModule.h"

#include "HeatMap.generated.h"

class AChannelerCharacter;
/**
 * 
 */
UCLASS(BlueprintType, BlueprintType)
class THECHANNELER_API UHeatMap : public UAnalyticsModule
{
	GENERATED_BODY()

private:
	struct FActorInfo
	{
		const AActor* ActorRef;
		float TimeViewed;
		bool TrackActor;
		bool IsDrawnThisFrame;

		FActorInfo() : TimeViewed(0.0f), TrackActor(false), IsDrawnThisFrame(false), ActorRef(nullptr) {}
	};
	
	static TMap<FString, FString> VLogFileTracker;

	TMap<FString, FActorInfo> HeatMapDataTable;
	FString FolderPath;
	FString LevelName;
	const AChannelerCharacter* Player;

	bool bIsRecordingToFile;			// if set we are recording to file
	float StartRecordingToFileTime;		// start recording time
	 
	// Save / Clean-up the HeatMap Results
	bool SaveHeatMapResults(bool DoForceClear = false);
	// Extract Mesh Data (vertices and indices) from a Static Mesh
	static bool MeshData(UStaticMeshComponent* StaticMeshComponent, TArray<FVector>& OutVertices, TArray<int32>& OutIndices);
	// Saves the string to a text file
	static bool SaveStringTextToFile(const FString& SaveDirectory, const FString& JoyfulFileName, const FString& SaveText, bool AllowOverWriting = true);
	// Update the VLogFileTracker to map the new VLog Files that have been generated
	static void TrackVLogFiles(const FString& CurrentLevelName);
	// Set The Recording state of the File
	void SetIsRecordingToFile(bool InIsRecording);
	// Helper function for StoreHeatMapData and LogActorInHeatMap
	void DrawMeshInVLog(const AActor * Actor, const FString& ActorName, const FColor & Color);
public:
	UHeatMap();
	virtual ~UHeatMap();

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Assign Channeler for HeatMap"), Category = "Channeler Heat Map Tool")
	void AssignChannelerCharacter(const AChannelerCharacter* Channeler);

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Store Heat Map Data"), Category = "Channeler Heat Map Tool")
	void StoreHeatMapData(const AActor* ActorHit, float DeltaTime);

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Is Heat Map Recording"), Category = "Channeler Heat Map Tool")
	bool IsHeatMapRecording() { return bIsRecordingToFile; }

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Add Actor to Track"), Category = "Channeler Heat Map Tool")
	void TrackActorInHeatMap(const AActor* ActorToTrack);

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Remove Actor to Track"), Category = "Channeler Heat Map Tool")
	void UntrackActorInHeatMap(const AActor* ActorToNotTrack);

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Log Actor for Single Frame"), Category = "Channeler Heat Map Tool")
	void LogActorInHeatMap(const AActor* Actor, const FColor& Color);

	// Activate the Heat Map module, perform initialization
	virtual void Activate() override;

	// Deactivate the Heat Map module, perform the clean up work
	virtual void Deactivate() override;

	// On Clean Up, move all the visual logger data into their respective folders
	virtual void CleanUp() override;

	// Store Level Name and generate Folder names based on that
	virtual void OnLevelChanged(UWorld * world, ULevel * level, const FString & name) override;

	// Update logging info of Actors that are being kept track of
	virtual void Update(float deltaTime) override;
};
