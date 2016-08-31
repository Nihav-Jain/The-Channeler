#pragma once
#include "EyeXPluginEx.h"
#include "EyeXAxisFilter.h"
#include "EyeXHeadTrackerCalibrator.generated.h"

FORCEINLINE	FEyeXEyePosition operator+(const FEyeXEyePosition & first, const FEyeXEyePosition & second)
{
	FEyeXEyePosition result = second;
	result.LeftEye = first.LeftEye + second.LeftEye;
	result.RightEye = first.RightEye + second.RightEye;
	return result;
}


FORCEINLINE	FEyeXEyePosition operator/(const FEyeXEyePosition & dividend, float divisor)
{
	FEyeXEyePosition result = dividend;
	result.LeftEye = dividend.LeftEye / divisor;
	result.RightEye = dividend.RightEye / divisor;
	return result;
}

UCLASS(BlueprintType)
class THECHANNELER_API UEyeXHeadTrackerCalibrator : public UObject
{
	GENERATED_BODY()
public:
	UEyeXHeadTrackerCalibrator();
	virtual ~UEyeXHeadTrackerCalibrator();

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Begin the calibration for head tracking"), Category = "EyeXHeadTracker")
	void BeginCalibration();

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "End the calibration for head tracking"), Category = "EyeXHeadTracker")
	void EndCalibration();

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Get the origin"), Category = "EyeXHeadTracker")
	void GetHeadOrigin(FVector & LeftEye, FVector& RightEye);

	UFUNCTION(BlueprintCallable, BlueprintPure, meta = (DisplayName = "Instantiate", CompactNodeTitle = "Create"), Category = "EyeXHeadTracker")
	static UEyeXHeadTrackerCalibrator* Create() {
		return NewObject<UEyeXHeadTrackerCalibrator>(UEyeXHeadTrackerCalibrator::StaticClass());
	};

	virtual void PostInitProperties() override;
	virtual void BeginDestroy() override;

private:
	UEyeXPluginEx * EyeXEx;

	/** Delegate for callbacks to Tick */
	FTickerDelegate TickDelegate;
	FDelegateHandle TickDelegateHandle;

	bool CalibrationTick(float DeltaSeconds);

	FEyeXEyePosition Origin;

	FEyeXEyePosition AccumulatedOrigin;
	float NumTicks;
	float NumFrame;

	UPROPERTY()
	UEyeXAxisFilter *Filter;

	FORCEINLINE void FilterEyePositionData(FEyeXEyePosition & position);
};
