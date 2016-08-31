// Copyright 2014 Tobii Technology AB. All Rights Reserved.

#pragma once

#include "EyeXClientLibraryLoader.h"

/**
 * Interface of an EyeX Engine state accessor. This is used by the EyeXPlugin to access engine states.
 */
class IEyeXStateAccessor
{
public:
	virtual ~IEyeXStateAccessor() {}

	// Notifies the state accessor that a connection to the EyeX Engine has been established.
	virtual void OnConnected() = 0;

	// Notifies the state accessor that the connection to the EyeX Engine has been lost.
	virtual void OnDisconnected() = 0;

	// Gets a value indicating whether the current state value is known.
	virtual bool bHasValue() const = 0;

};


/**
 * Provides access to an EyeX Engine state.
 */
template<typename ValueType>
class TEyeXStateAccessor : public IEyeXStateAccessor
{
public:

	/**
	 * Creates a new instance.
	 *
	 * @param Context - EyeX context handle.
	 * @param bIsConnected - A value indicating whether a connection to the EyeX Engine has been established.
	 * @param StatePath - Identifies the engine state to be monitored.
	 * @param EyeXClient - Wraps the EyeX client library.
	 */
	TEyeXStateAccessor(TX_CONTEXTHANDLE Context, bool bIsConnected, const char *StatePath, FEyeXClientLibraryLoader& EyeXClient);

	/** IEyeXStateAccessor implementation */
	virtual ~TEyeXStateAccessor() override;
	virtual void OnConnected() override;
	virtual void OnDisconnected() override;
	virtual bool bHasValue() const override { return bHasCurrentValue; }

	// Gets the current state value.
	const ValueType& Value() const { return CurrentValue; }

private:

	static void StateChangedHandlerTrampoline(TX_CONSTHANDLE AsyncData, TX_USERPARAM UserParam);

	void HandleStateChanged(TX_CONSTHANDLE AsyncData);

private:

	TX_CONTEXTHANDLE Context;
	FEyeXClientLibraryLoader& EyeXClient;
	const char *StatePath;
	TX_TICKET Ticket;
	ValueType CurrentValue;
	bool bHasCurrentValue;

private:

	// avoid accidental copying by declaring these as private
	TEyeXStateAccessor(const TEyeXStateAccessor&);
	TEyeXStateAccessor& operator = (const TEyeXStateAccessor&);
};


template<typename ValueType>
TEyeXStateAccessor<ValueType>::TEyeXStateAccessor(TX_CONTEXTHANDLE Context, bool bIsConnected, const char *StatePath, FEyeXClientLibraryLoader& EyeXClient) :
	Context(Context),
	EyeXClient(EyeXClient),
	StatePath(StatePath),
	Ticket(TX_INVALID_TICKET),
	bHasCurrentValue(false)
{
	if (Context == TX_EMPTY_HANDLE)
	{
		UE_LOG(LogEyeX, Warning, TEXT("State accessor initialization ignored because of invalid context handle."));
		return;
	}

	TX_RESULT result = EyeXClient.RegisterStateChangedHandler(Context, &Ticket, StatePath, StateChangedHandlerTrampoline, this);
	if (TX_RESULT_OK != result)
	{
		UE_LOG(LogEyeX, Error, TEXT("Could not register state changed callback. Error code: %d."), (int32)result);
		return;
	}

	if (bIsConnected)
	{
		OnConnected();
	}
}

template<typename ValueType>
TEyeXStateAccessor<ValueType>::~TEyeXStateAccessor()
{
	if (Ticket != TX_INVALID_TICKET)
	{
		TX_RESULT result = EyeXClient.UnregisterStateChangedHandler(Context, Ticket);
		if (TX_RESULT_OK != result)
		{
			UE_LOG(LogEyeX, Error, TEXT("Could not unregister state changed callback. Error code: %d."), (int32)result);
		}
	}
}

template<typename ValueType>
void TEyeXStateAccessor<ValueType>::OnConnected()
{
	TX_RESULT result = EyeXClient.GetStateAsync(Context, StatePath, StateChangedHandlerTrampoline, this);
	if (TX_RESULT_OK != result)
	{
		UE_LOG(LogEyeX, Error, TEXT("Request for current state value failed. Error code: %d."), (int32)result);
	}
}

template<typename ValueType>
void TEyeXStateAccessor<ValueType>::OnDisconnected()
{
	bHasCurrentValue = false;
}


template<typename ValueType>
void TEyeXStateAccessor<ValueType>::StateChangedHandlerTrampoline(TX_CONSTHANDLE AsyncData, TX_USERPARAM UserParam)
{
	static_cast<TEyeXStateAccessor*>(UserParam)->HandleStateChanged(AsyncData);
}
