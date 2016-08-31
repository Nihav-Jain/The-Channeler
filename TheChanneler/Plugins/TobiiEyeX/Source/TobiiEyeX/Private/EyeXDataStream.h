// Copyright 2014 Tobii Technology AB. All Rights Reserved.

#pragma once

#include "EyeXTypes.h"
#include "EyeXClientLibraryLoader.h"

#define MAX_DATA_STREAM_INTERACTOR_ID_SIZE 20

/**
 * Handles a generic data stream from the EyeX Engine.
 */
class FEyeXDataStream
{
public:

	/*
	 * Creates a new instance.
	 *
	 * @param EyeXClient - Wraps the EyeX client library.
	 */
	FEyeXDataStream(FEyeXClientLibraryLoader& EyeXClient) : EyeXClient(EyeXClient) {}

	virtual ~FEyeXDataStream() {}

	// Gets a unique interactor ID.
	virtual const char *GetInteractorId() const = 0;

	// Adds a global interactor representing the data stream to the given snapshot.
	void AddToSnapshot(TX_HANDLE Snapshot, bool MarkAsDeleted) const;

	// Handles an event intended for this data stream.
	virtual void HandleEvent(TX_CONSTHANDLE Event) = 0;

protected:

	// Adds a behavior describing the data stream to the given interactor.
	virtual void AddBehavior(TX_HANDLE Interactor) const = 0;

protected:

	FEyeXClientLibraryLoader& EyeXClient;

private:

	// avoid accidental copying by declaring these as private
	FEyeXDataStream(const FEyeXDataStream&);
	FEyeXDataStream& operator = (const FEyeXDataStream&);
};


/**
 * Specializaton of FEyeXDataStream for gaze point data streams.
 */
class FEyeXGazePointDataStream : public FEyeXDataStream
{
public:

	// Writes an interactor ID for a data stream of this type and mode to the given buffer.
	// The buffer is assumed to be at least MAX_DATA_STREAM_INTERACTOR_ID_SIZE bytes.
	static void GetInteractorId(char *buffer, EEyeXGazePointDataMode::Type Mode);

	FEyeXGazePointDataStream(EEyeXGazePointDataMode::Type Mode, FEyeXClientLibraryLoader& EyeXClient);

	virtual const char *GetInteractorId() const override { return Id; }

	virtual void HandleEvent(TX_CONSTHANDLE Event) override;

	const FEyeXGazePoint& Last() const { return Data; }

protected:

	virtual void AddBehavior(TX_HANDLE Interactor) const override;

private:

	char Id[MAX_DATA_STREAM_INTERACTOR_ID_SIZE];
	EEyeXGazePointDataMode::Type Mode;
	FEyeXGazePoint Data;
};


/**
 * Specializaton of FEyeXDataStream for fixation data streams.
 */
class FEyeXFixationDataStream : public FEyeXDataStream
{
public:

	// Writes an interactor ID for a data stream of this type and mode to the given buffer.
	// The buffer is assumed to be at least MAX_DATA_STREAM_INTERACTOR_ID_SIZE bytes.
	static void GetInteractorId(char *buffer, EEyeXFixationDataMode::Type Mode);

	FEyeXFixationDataStream(EEyeXFixationDataMode::Type Mode, FEyeXClientLibraryLoader& EyeXClient);

	virtual const char *GetInteractorId() const override { return Id; }

	virtual void HandleEvent(TX_CONSTHANDLE Event) override;

	const FEyeXFixationDataPoint& Last() const { return Data; }

	const FEyeXFixationDataPoint& Ongoing() const { return CurrentData; }

protected:

	virtual void AddBehavior(TX_HANDLE Interactor) const override;

private:

	char Id[MAX_DATA_STREAM_INTERACTOR_ID_SIZE];
	EEyeXFixationDataMode::Type Mode;
	FEyeXFixationDataPoint Data;
	FEyeXFixationDataPoint CurrentData;
};


/**
 * Specializaton of FEyeXDataStream for eye position data streams.
 */
class FEyeXEyePositionDataStream : public FEyeXDataStream
{
public:

	// Writes an interactor ID for a data stream of this type to the given buffer.
	// The buffer is assumed to be at least MAX_DATA_STREAM_INTERACTOR_ID_SIZE bytes.
	static void GetInteractorId(char *buffer);

	FEyeXEyePositionDataStream(FEyeXClientLibraryLoader& EyeXClient);

	virtual const char *GetInteractorId() const override { return Id; }

	virtual void HandleEvent(TX_CONSTHANDLE Event) override;

	const FEyeXEyePosition& Last() const { return Data; }

protected:

	virtual void AddBehavior(TX_HANDLE Interactor) const override;

private:

	static const char *Id;
	FEyeXEyePosition Data;
};
