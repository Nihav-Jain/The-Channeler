// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Object.h"
#include "EyeXAxisFilter.generated.h"

/**
 * 
 */
UCLASS(BlueprintType)
class THECHANNELER_API UEyeXAxisFilter : public UObject
{
	GENERATED_BODY()

private:
	// -----------------------------------------------------------------
	// Utilities
	static const float UndefinedTime;	// = -1.0;	// TimeStamp
	static const float M_PI;			// = 3.141529f;

										// -----------------------------------------------------------------

	class LowPassFilter
	{

		double y, a, s;
		bool initialized;

		void setAlpha(double alpha);

	public:
		LowPassFilter(double alpha, double initval = 0.0);
		double filter(double value);
		double filterWithAlpha(double value, double alpha);
		bool hasLastRawValue(void);
		double lastRawValue(void);

	};

	// -----------------------------------------------------------------

	double freq;
	double mincutoff;
	double beta_;
	double dcutoff;
	LowPassFilter *x;
	LowPassFilter *dx;
	float lasttime;		// TimeStamp

	float thresholdCheck;
	float lastValue;

	double alpha(double cutoff);
	void setFrequency(double f);
	void setMinCutoff(double mc);
	void setBeta(double b);
	void setDerivateCutoff(double dc);

public:
	UEyeXAxisFilter() {}
	virtual ~UEyeXAxisFilter();

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Initialize", CompactNodeTitle = "Filter"), Category = "MyAxisFilter")
	void Init(float freq, float mincutoff = 1.0f, float beta_ = 0.0f, float dcutoff = 1.0f, float threshold = 0.0f);

	UFUNCTION(BlueprintCallable, BlueprintPure, meta = (DisplayName = "One Euro Filter", CompactNodeTitle = "Filter"), Category = "MyAxisFilter")
	float filter(float value, float timestamp);

	UFUNCTION(BlueprintCallable, BlueprintPure, meta = (DisplayName = "Instantiate", CompactNodeTitle = "Create"), Category = "MyAxisFilter")
	static UEyeXAxisFilter* Create();
};
