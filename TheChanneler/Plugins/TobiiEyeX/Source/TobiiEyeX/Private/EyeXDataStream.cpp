// Copyright 2014 Tobii Technology AB. All Rights Reserved.

#include "EyeXPluginPrivatePCH.h"
#include "EyeXDataStream.h"

#if !PLATFORM_PS4
#pragma warning(disable: 4996) // suppress warning: Function call with parameters that may be unsafe
#else
#define _snprintf snprintf
#endif


void FEyeXDataStream::AddToSnapshot(TX_HANDLE Snapshot, bool MarkAsDeleted) const
{
	FEyeXScopedHandle interactor(EyeXClient);
	EyeXClient.CreateInteractor(Snapshot, &interactor.Handle, GetInteractorId(), TX_LITERAL_ROOTID, TX_LITERAL_GLOBALINTERACTORWINDOWID);
	
	FEyeXScopedHandle bounds(EyeXClient);
	EyeXClient.CreateInteractorBounds(interactor.Handle, &bounds.Handle, TX_BOUNDSTYPE_NONE);

	AddBehavior(interactor.Handle);

	if (MarkAsDeleted)
	{
		EyeXClient.SetInteractorDeleted(interactor.Handle, TX_TRUE);
	}
}

// -------------------------------------------------------------------
//  Implementation of FEyeXGazePointDataStream
// -------------------------------------------------------------------

void FEyeXGazePointDataStream::GetInteractorId(char *buffer, EEyeXGazePointDataMode::Type Mode)
{
	_snprintf(buffer, MAX_DATA_STREAM_INTERACTOR_ID_SIZE, "ds_gazepoint_%d", Mode);
}

FEyeXGazePointDataStream::FEyeXGazePointDataStream(EEyeXGazePointDataMode::Type Mode, FEyeXClientLibraryLoader& EyeXClient) :
	FEyeXDataStream(EyeXClient),
	Mode(Mode), 
	Data(FEyeXGazePoint::Invalid())
{
	GetInteractorId(Id, Mode);
}

void FEyeXGazePointDataStream::HandleEvent(TX_CONSTHANDLE Event)
{
	FEyeXScopedHandle behavior(EyeXClient);
	if (TX_RESULT_OK != EyeXClient.GetEventBehavior(Event, &behavior.Handle, TX_BEHAVIORTYPE_GAZEPOINTDATA))
	{
		return;
	}

	TX_GAZEPOINTDATAEVENTPARAMS eventParams;
	if (TX_RESULT_OK != EyeXClient.GetGazePointDataEventParams(behavior.Handle, &eventParams))
	{
		return;
	}

	Data = FEyeXGazePoint(FVector2D((float)eventParams.X, (float)eventParams.Y), (float)eventParams.Timestamp, true);
}

void FEyeXGazePointDataStream::AddBehavior(TX_HANDLE Interactor) const
{
	FEyeXScopedHandle behavior(EyeXClient);
	EyeXClient.CreateInteractorBehavior(Interactor, &behavior.Handle, TX_BEHAVIORTYPE_GAZEPOINTDATA);

	TX_GAZEPOINTDATAPARAMS dataParams;
	switch (Mode)
	{
	case EEyeXGazePointDataMode::Unfiltered:
		dataParams.GazePointDataMode = TX_GAZEPOINTDATAMODE_UNFILTERED;
		break;

	default:
		dataParams.GazePointDataMode = TX_GAZEPOINTDATAMODE_LIGHTLYFILTERED;
		break;
	}
	EyeXClient.SetGazePointDataBehaviorParams(behavior.Handle, &dataParams);
}

// -------------------------------------------------------------------
//  Implementation of FEyeXFixationDataStream
// -------------------------------------------------------------------

void FEyeXFixationDataStream::GetInteractorId(char *buffer, EEyeXFixationDataMode::Type Mode)
{
	_snprintf(buffer, MAX_DATA_STREAM_INTERACTOR_ID_SIZE, "ds_fixation_%d", Mode);
}

FEyeXFixationDataStream::FEyeXFixationDataStream(EEyeXFixationDataMode::Type Mode, FEyeXClientLibraryLoader& EyeXClient) :
	FEyeXDataStream(EyeXClient),
	Mode(Mode),
	Data(FEyeXFixationDataPoint::Invalid()),
	CurrentData(FEyeXFixationDataPoint::Invalid())
{
	GetInteractorId(Id, Mode);
}

void FEyeXFixationDataStream::HandleEvent(TX_CONSTHANDLE Event)
{
	FEyeXScopedHandle behavior(EyeXClient);
	if (TX_RESULT_OK != EyeXClient.GetEventBehavior(Event, &behavior.Handle, TX_BEHAVIORTYPE_FIXATIONDATA))
	{
		return;
	}

	TX_FIXATIONDATAEVENTPARAMS eventParams;
	if (TX_RESULT_OK != EyeXClient.GetFixationDataEventParams(behavior.Handle, &eventParams))
	{
		return;
	}

	if (TX_FIXATIONDATAEVENTTYPE_BEGIN == eventParams.EventType)
	{
		CurrentData = FEyeXFixationDataPoint(
			FVector2D((float)eventParams.X, (float)eventParams.Y),
			(float)eventParams.Timestamp,
			0.0f,
			true);
	}
	else
	{
		auto data = FEyeXFixationDataPoint(
			FVector2D((float)eventParams.X, (float)eventParams.Y),
			CurrentData.BeginTimeStamp,
			(float)eventParams.Timestamp - CurrentData.BeginTimeStamp,
			true);

		if (TX_FIXATIONDATAEVENTTYPE_DATA == eventParams.EventType)
		{
			CurrentData = data;
		}
		else if (TX_FIXATIONDATAEVENTTYPE_END == eventParams.EventType)
		{
			Data = data;
			CurrentData = FEyeXFixationDataPoint::Invalid();
		}
		else
		{
			CurrentData = FEyeXFixationDataPoint::Invalid();
		}
	}
}

void FEyeXFixationDataStream::AddBehavior(TX_HANDLE Interactor) const
{
	FEyeXScopedHandle behavior(EyeXClient);
	EyeXClient.CreateInteractorBehavior(Interactor, &behavior.Handle, TX_BEHAVIORTYPE_FIXATIONDATA);

	TX_FIXATIONDATAPARAMS dataParams;
	switch (Mode)
	{
	case EEyeXFixationDataMode::Sensitive:
		dataParams.FixationDataMode = TX_FIXATIONDATAMODE_SENSITIVE;
		break;

	default:
		dataParams.FixationDataMode = TX_FIXATIONDATAMODE_SLOW;
		break;
	}
	EyeXClient.SetFixationDataBehaviorParams(behavior.Handle, &dataParams);
}

// -------------------------------------------------------------------
//  Implementation of FEyeXEyePositionDataStream
// -------------------------------------------------------------------

#define DEFAULT_EYEPOSITION_ID "ds_eyeposition"
const char *FEyeXEyePositionDataStream::Id = DEFAULT_EYEPOSITION_ID;

void FEyeXEyePositionDataStream::GetInteractorId(char *buffer)
{
	_snprintf(buffer, MAX_DATA_STREAM_INTERACTOR_ID_SIZE, DEFAULT_EYEPOSITION_ID);
}

FEyeXEyePositionDataStream::FEyeXEyePositionDataStream(FEyeXClientLibraryLoader& EyeXClient) :
	FEyeXDataStream(EyeXClient),
	Data(FEyeXEyePosition::Invalid())
{
}

void FEyeXEyePositionDataStream::HandleEvent(TX_CONSTHANDLE Event)
{
	FEyeXScopedHandle behavior(EyeXClient);
	if (TX_RESULT_OK != EyeXClient.GetEventBehavior(Event, &behavior.Handle, TX_BEHAVIORTYPE_EYEPOSITIONDATA))
	{
		return;
	}

	TX_EYEPOSITIONDATAEVENTPARAMS eventParams;
	if (TX_RESULT_OK != EyeXClient.GetEyePositionDataEventParams(behavior.Handle, &eventParams))
	{
		return;
	}

	Data = FEyeXEyePosition(
		FVector((float)eventParams.LeftEyeX, (float)eventParams.LeftEyeY, (float)eventParams.LeftEyeZ),
		FVector((float)eventParams.RightEyeX, (float)eventParams.RightEyeY, (float)eventParams.RightEyeZ),
		(float)eventParams.Timestamp, 
		TX_TRUE == eventParams.HasLeftEyePosition,
		TX_TRUE == eventParams.HasRightEyePosition);
}

void FEyeXEyePositionDataStream::AddBehavior(TX_HANDLE Interactor) const
{
	FEyeXScopedHandle behavior(EyeXClient);
	EyeXClient.CreateInteractorBehavior(Interactor, &behavior.Handle, TX_BEHAVIORTYPE_EYEPOSITIONDATA);
}

