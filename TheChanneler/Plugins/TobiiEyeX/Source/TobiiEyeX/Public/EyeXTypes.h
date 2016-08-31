// Copyright 2014 Tobii Technology AB. All Rights Reserved.

#pragma once

#include "EyeXTypes.generated.h"

/** 
 * Describes the available gaze data streams.
 * Unfiltered: Unfiltered data from the eye tracker. The most responsive mode but can jitter a lot.
 * LightlyFiltered: A light filtering applied by the EyeX Engine. Less jittering but also slightly less responsive.
 */
UENUM(BlueprintType)
namespace EEyeXGazePointDataMode
{
	enum Type
	{
		Unfiltered			UMETA(DisplayName = "Unfiltered"),
		LightlyFiltered		UMETA(DisplayName = "LightlyFiltered")
	};
}

/** 
 * Struct to hold gaze point data, including screen position, timestamp and validity status.
 */
USTRUCT(BlueprintType)
struct FEyeXGazePoint
{
	GENERATED_USTRUCT_BODY()

	/** The gaze point screen coordinate relative to the viewport. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "EyeXGazePoint")
	FVector2D Value;

	/** The timestamp of when this data was created by the EyeX Engine in milliseconds. Can vary
	depending on the gaze point type. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "EyeXGazePoint")
	float TimeStamp;

	/** Value indicating whether this instance actually represents a data point. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "EyeXGazePoint")
	bool bHasValue;

	FEyeXGazePoint() {}

	FEyeXGazePoint(const FVector2D& Value, float TimeStamp, bool bHasValue) :
		Value(Value),
		TimeStamp(TimeStamp),
		bHasValue(bHasValue)
	{}
	
	FORCEINLINE static FEyeXGazePoint Invalid()
	{
		return FEyeXGazePoint(FVector2D::ZeroVector, 0.0f, false);
	}
};


/**
 * Describes the available fixation data streams.
 */
UENUM(BlueprintType)
namespace EEyeXFixationDataMode
{
	enum Type
	{
		Sensitive	UMETA(DisplayName = "Sensitive"),
		Slow		UMETA(DisplayName = "Slow"),
	};
}

/** 
 * Struct to hold fixation data.
 */
USTRUCT(BlueprintType)
struct FEyeXFixationDataPoint
{
	GENERATED_USTRUCT_BODY()

	/** The gaze point screen coordinate relative to the viewport. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "EyeXFixationDataPoint")
	FVector2D GazePoint;

	/** The timestamp of when this fixation began. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "EyeXFixationDataPoint")
	float BeginTimeStamp;

	/** The duration of this fixation since it began. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "EyeXFixationDataPoint")
    float Duration;

	/** Value indicating whether this instance actually represents a data point. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "EyeXFixationDataPoint")
	bool bHasValue;

	FEyeXFixationDataPoint() {}

	FEyeXFixationDataPoint(const FVector2D& GazePoint, float BeginTimeStamp, float Duration, bool bHasValue) :
		GazePoint(GazePoint),
		BeginTimeStamp(BeginTimeStamp),
		Duration(Duration),
		bHasValue(bHasValue)
	{}
	
	FORCEINLINE static FEyeXFixationDataPoint Invalid()
	{
		return FEyeXFixationDataPoint(FVector2D::ZeroVector, 0.0f, 0.0f, false);
	}
};


/**
* Struct to hold eye position data, including world position for both eyes in unreal coordinates
* and the coordinate system used by the EyeX engine, timestamp and validity status.
*/
USTRUCT(BlueprintType)
struct FEyeXEyePosition
{
	GENERATED_USTRUCT_BODY()

	/** Position of the left eye in 3D-coordinates in relation to the screen. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "EyeXEyePosition")
	FVector LeftEye;

	/** Position of the right eye in 3D-coordinates in relation to the screen. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "EyeXEyePosition")
	FVector RightEye;

	/** A timestamp of when the eye position was calculated in milliseconds. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "EyeXEyePosition")
	float TimeStamp;

	/** If the left eye position is valid. Will be invalid if the tracker lost track of the left eye. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "EyeXEyePosition")
	bool bIsLeftEyeValid;

	/** If the right eye position is valid. Will be invalid if the tracker lost track of the right eye. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "EyeXEyePosition")
	bool bIsRightEyeValid;

	FEyeXEyePosition() {}

	FEyeXEyePosition(const FVector& LeftEye, const FVector& RightEye, float TimeStamp, bool bIsLeftEyeValid, bool bIsRightEyeValid) :
		LeftEye(LeftEye),
		RightEye(RightEye),
		TimeStamp(TimeStamp),
		bIsLeftEyeValid(bIsLeftEyeValid),
		bIsRightEyeValid(bIsRightEyeValid)
	{}

	FORCEINLINE static FEyeXEyePosition Invalid()
	{
		return FEyeXEyePosition(FVector::ZeroVector, FVector::ZeroVector, 0.0f, false, false);
	}
};


/**
* Holds the bounds of a screen in physical pixels.
*/
USTRUCT(BlueprintType)
struct FEyeXScreenBounds
{
	GENERATED_USTRUCT_BODY()

	/** X coordinate of the screen relative to the primary screen in physical pixels. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "EyeXScreenBounds")
	int32 X;

	/** Y coordinate of the screen relative to the primary screen in physical pixels. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "EyeXScreenBounds")
	int32 Y;

	/** Width of the screen in physical pixels. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "EyeXScreenBounds")
	int32 Width;

	/** Height of the screen in physical pixels. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "EyeXScreenBounds")
	int32 Height;

	FEyeXScreenBounds() {}

	FEyeXScreenBounds(int32 X, int32 Y, int32 Width, int32 Height) :
		X(X),
		Y(Y),
		Width(Width),
		Height(Height)
	{}

	FORCEINLINE static FEyeXScreenBounds Invalid()
	{
		return FEyeXScreenBounds(0, 0, 0, 0);
	}
};


/** 
 * Describes the possible device statuses.
*/
UENUM(BlueprintType)
namespace EEyeXDeviceStatus
{
	enum Type
	{
		Pending			UMETA(DisplayName = "Pending"),
		Tracking		UMETA(DisplayName = "Tracking"),
		Disabled		UMETA(DisplayName = "Disabled"),
		NotAvailable	UMETA(DisplayName = "NotAvailable"),
		Unknown			UMETA(DisplayName = "Unknown"),
	};
}


/** 
 * Describes the possible user presence states.
*/
UENUM(BlueprintType)
namespace EEyeXUserPresence
{
	enum Type
	{
		Present         UMETA(DisplayName = "Present"),
		NotPresent      UMETA(DisplayName = "NotPresent"),
		Unknown         UMETA(DisplayName = "Unknown"),
	};
}


/**
 * Represents a value that may or may not be known, such as, for example, an EyeX Engine state.
 */
template <typename ValueType>
struct TEyeXMaybeValue
{
	/** Indicates whether the value is known. */
	bool bHasValue;

	/** Value, if bHasValue. Otherwise undefined. */
	ValueType Value;

	TEyeXMaybeValue(const ValueType& Value, bool bHasValue = true) :
		bHasValue(bHasValue),
		Value(Value)
	{}
};

/**
* Describes the available modes for mouse emulation of the gaze point.
*/
UENUM(BlueprintType)
namespace EEyeXEmulationMode
{
	enum Type
	{
		Disabled		UMETA(DisplayName = "Disabled"),
		Enabled			UMETA(DisplayName = "Enabled")
	};
}

struct FMyInteractor
{
	int32 X;
	int32 Y;
	uint32 Z;

	uint32 Width;
	uint32 Height;

	uint32 Id;
};

struct FMyBitmask
{
	uint32 NrRows;
	uint32 NrColumns;

	TArray<uint8> Data;
};

struct FMyMaskInteractor
{
	FMyInteractor Interactor;
	FMyBitmask Bitmask;
};
