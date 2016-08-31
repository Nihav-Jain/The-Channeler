// Copyright 2014 Tobii Technology AB. All Rights Reserved.

#pragma once

#define TX_NODEBUGOBJECT
#include <eyex/EyeX.h>

// Produces function definition: FunctionNameHook FunctionName
// Example: InitializeEyeXHook InitializeEyeX
// The types FunctionNameHook are already defined in the EyeX header.
// Use this if you need to bind additional functions in the EyeX DLL.
#if !defined(EYEX_DLL_HOOK)
	#define EYEX_DLL_HOOK(FunctionName) FunctionName##Hook FunctionName
#endif

/**
 * Handles the dynamic loading of and access to functions in the Tobii EyeX Client library.
 */
class FEyeXClientLibraryLoader
{
public:

	FEyeXClientLibraryLoader();
	~FEyeXClientLibraryLoader();

	bool IsAvailable() const { return InitializeEyeX != nullptr; }

	// Definitions of all the EyeX API functions used by the plugin.

	EYEX_DLL_HOOK(InitializeEyeX); 
	EYEX_DLL_HOOK(UninitializeEyeX);

	EYEX_DLL_HOOK(CreateContext);
	EYEX_DLL_HOOK(ReleaseContext);
	EYEX_DLL_HOOK(ShutdownContext);
	EYEX_DLL_HOOK(EnableConnection);

	EYEX_DLL_HOOK(RegisterConnectionStateChangedHandler);
	EYEX_DLL_HOOK(UnregisterConnectionStateChangedHandler);
	EYEX_DLL_HOOK(RegisterStateChangedHandler);
	EYEX_DLL_HOOK(UnregisterStateChangedHandler);
	EYEX_DLL_HOOK(RegisterEventHandler);
	EYEX_DLL_HOOK(UnregisterEventHandler);

	EYEX_DLL_HOOK(CreateSnapshot);
	EYEX_DLL_HOOK(CreateGlobalInteractorSnapshot);
	EYEX_DLL_HOOK(CreateSnapshotBounds);
	EYEX_DLL_HOOK(AddSnapshotWindowId);
	EYEX_DLL_HOOK(CommitSnapshotAsync);
	EYEX_DLL_HOOK(CreateInteractor);
	EYEX_DLL_HOOK(CreateInteractorBounds);
	EYEX_DLL_HOOK(CreateInteractorBehavior);
	EYEX_DLL_HOOK(SetInteractorDeleted);
	EYEX_DLL_HOOK(SetGazePointDataBehaviorParams);
	EYEX_DLL_HOOK(GetGazePointDataEventParams);
	EYEX_DLL_HOOK(SetFixationDataBehaviorParams);
	EYEX_DLL_HOOK(GetFixationDataEventParams);
	EYEX_DLL_HOOK(GetEyePositionDataEventParams);

	EYEX_DLL_HOOK(GetAsyncDataResultCode);
	EYEX_DLL_HOOK(GetAsyncDataContent);

	EYEX_DLL_HOOK(GetEventBehavior);
	EYEX_DLL_HOOK(GetEventInteractorId);

	EYEX_DLL_HOOK(GetStateAsync);
	EYEX_DLL_HOOK(GetStateValueAsInteger);
	EYEX_DLL_HOOK(GetStateValueAsSize2);
	EYEX_DLL_HOOK(GetStateValueAsRectangle);

	EYEX_DLL_HOOK(ReleaseObject);
	EYEX_DLL_HOOK(FormatObjectAsText);

private:

	void* DllHandle;
};

class FEyeXScopedHandle
{
public:

	FEyeXScopedHandle(FEyeXClientLibraryLoader& EyeXClient)
		: Handle(TX_EMPTY_HANDLE), EyeXClient(EyeXClient)
	{}

	~FEyeXScopedHandle()
	{
		if (Handle != TX_EMPTY_HANDLE)
		{
			EyeXClient.ReleaseObject(&Handle);
		}
	}

	TX_HANDLE Handle;

private:
	FEyeXClientLibraryLoader& EyeXClient;
};
