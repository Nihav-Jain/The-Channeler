// Copyright 2014 Tobii Technology AB. All Rights Reserved.

#include "EyeXPluginPrivatePCH.h"
#include "EyeXStateAccessor.h"

#if PLATFORM_PS4
template<>
#endif
void TEyeXStateAccessor<TX_SIZE2>::HandleStateChanged(TX_CONSTHANDLE AsyncData)
{
	FEyeXScopedHandle stateBag(EyeXClient);
	if (TX_RESULT_OK == EyeXClient.GetAsyncDataContent(AsyncData, &stateBag.Handle) &&
		TX_RESULT_OK == EyeXClient.GetStateValueAsSize2(stateBag.Handle, StatePath, &CurrentValue))
	{
		bHasCurrentValue = true;
	}
}

#if PLATFORM_PS4
template<>
#endif
void TEyeXStateAccessor<TX_RECT>::HandleStateChanged(TX_CONSTHANDLE AsyncData)
{
	FEyeXScopedHandle stateBag(EyeXClient);
	if (TX_RESULT_OK == EyeXClient.GetAsyncDataContent(AsyncData, &stateBag.Handle) &&
		TX_RESULT_OK == EyeXClient.GetStateValueAsRectangle(stateBag.Handle, StatePath, &CurrentValue))
	{
		bHasCurrentValue = true;
	}
}

#if PLATFORM_PS4
template<>
#endif
void TEyeXStateAccessor<TX_INTEGER>::HandleStateChanged(TX_CONSTHANDLE AsyncData)
{
	FEyeXScopedHandle stateBag(EyeXClient);
	if (TX_RESULT_OK == EyeXClient.GetAsyncDataContent(AsyncData, &stateBag.Handle) &&
		TX_RESULT_OK == EyeXClient.GetStateValueAsInteger(stateBag.Handle, StatePath, &CurrentValue))
	{
		bHasCurrentValue = true;
	}
}
