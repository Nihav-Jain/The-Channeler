// Copyright 2014 Tobii Technology AB. All Rights Reserved.

#include "EyeXPluginPrivatePCH.h"
#include "EyeXClientLibraryLoader.h"

// Macro to import DLL functions from Tobii.EyeX.Client.dll
// Use this if you need to bind additional functions of the EyeX DLL.
#if !defined(EYEX_DLL_IMPORT)
	#define EYEX_DLL_IMPORT(FunctionName) {															\
		const FString EntryPoint = (FString)"tx" + (FString)#FunctionName;							\
		FunctionName = (FunctionName##Hook)FPlatformProcess::GetDllExport(DllHandle, *EntryPoint);	\
		if (!FunctionName) {																		\
			UE_LOG(LogEyeX, Error, TEXT("Failed to import DLL function %s!"), *EntryPoint);			\
			InitializeEyeX = nullptr;																\
			}																						\
		} 
#endif

FEyeXClientLibraryLoader::FEyeXClientLibraryLoader()
: InitializeEyeX(nullptr)
{
#if PLATFORM_32BITS
	const FString DLL_PATH = FPaths::ConvertRelativePathToFull(FPaths::Combine(
		*FPaths::GamePluginsDir(),
		TEXT("TobiiEyeX/ThirdParty/EyeX/lib/"),
		TEXT("x86"),
		TEXT("Tobii.EyeX.Client.dll")));

#else
	const FString DLL_PATH = FPaths::ConvertRelativePathToFull(FPaths::Combine(
		*FPaths::GamePluginsDir(),
		TEXT("TobiiEyeX/ThirdParty/EyeX/lib/"),
		TEXT("x64"),
		TEXT("Tobii.EyeX.Client.dll")));
#endif



	DllHandle = FPlatformProcess::GetDllHandle(*DLL_PATH);
	if (!DllHandle)
	{
		UE_LOG(LogEyeX, Error, TEXT("Tobii.EyeX.Client.dll missing from %s !"), *DLL_PATH);
		return;
	}

	// Importing of all the EyeX functions needed by the plugin.

	EYEX_DLL_IMPORT(InitializeEyeX);
	EYEX_DLL_IMPORT(UninitializeEyeX);

	EYEX_DLL_IMPORT(CreateContext);
	EYEX_DLL_IMPORT(ReleaseContext);
	EYEX_DLL_IMPORT(ShutdownContext);
	EYEX_DLL_IMPORT(EnableConnection);

	EYEX_DLL_IMPORT(RegisterConnectionStateChangedHandler);
	EYEX_DLL_IMPORT(UnregisterConnectionStateChangedHandler);
	EYEX_DLL_IMPORT(RegisterStateChangedHandler);
	EYEX_DLL_IMPORT(UnregisterStateChangedHandler);
	EYEX_DLL_IMPORT(RegisterEventHandler);
	EYEX_DLL_IMPORT(UnregisterEventHandler);

	EYEX_DLL_IMPORT(CreateSnapshot);
	EYEX_DLL_IMPORT(CreateGlobalInteractorSnapshot);
	EYEX_DLL_IMPORT(CreateSnapshotBounds);
	EYEX_DLL_IMPORT(AddSnapshotWindowId);
	EYEX_DLL_IMPORT(CommitSnapshotAsync);
	EYEX_DLL_IMPORT(CreateInteractor);
	EYEX_DLL_IMPORT(CreateInteractorBounds);
	EYEX_DLL_IMPORT(CreateInteractorBehavior);
	EYEX_DLL_IMPORT(SetInteractorDeleted);
	EYEX_DLL_IMPORT(SetGazePointDataBehaviorParams);
	EYEX_DLL_IMPORT(GetGazePointDataEventParams);
	EYEX_DLL_IMPORT(SetFixationDataBehaviorParams);
	EYEX_DLL_IMPORT(GetFixationDataEventParams);
	EYEX_DLL_IMPORT(GetEyePositionDataEventParams);

	EYEX_DLL_IMPORT(GetAsyncDataResultCode);
	EYEX_DLL_IMPORT(GetAsyncDataContent);

	EYEX_DLL_IMPORT(GetEventBehavior);
	EYEX_DLL_IMPORT(GetEventInteractorId);

	EYEX_DLL_IMPORT(GetStateAsync);
	EYEX_DLL_IMPORT(GetStateValueAsInteger);
	EYEX_DLL_IMPORT(GetStateValueAsSize2);
	EYEX_DLL_IMPORT(GetStateValueAsRectangle);

	EYEX_DLL_IMPORT(ReleaseObject);
	EYEX_DLL_IMPORT(FormatObjectAsText);

}

FEyeXClientLibraryLoader::~FEyeXClientLibraryLoader()
{
	if (DllHandle)
	{
		FPlatformProcess::FreeDllHandle(DllHandle);
	}
}
