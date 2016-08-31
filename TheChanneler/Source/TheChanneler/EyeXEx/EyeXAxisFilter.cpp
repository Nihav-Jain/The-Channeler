// Fill out your copyright notice in the Description page of Project Settings.

#include "TheChanneler.h"
#include "EyeXAxisFilter.h"

typedef double TimeStamp; // in seconds
const float UEyeXAxisFilter::UndefinedTime = -1.0;
const float UEyeXAxisFilter::M_PI = 3.141529f;

UEyeXAxisFilter::LowPassFilter::LowPassFilter(double alpha, double initval)
{
	y = s = initval;
	setAlpha(alpha);
	initialized = false;
}


void UEyeXAxisFilter::LowPassFilter::setAlpha(double alpha)
{
	if (alpha <= 0.0 || alpha > 1.0)
	{
		//throw std::exception("alpha should be in (0.0., 1.0]");
		a = 1.0;
	}
	else
	{
		a = alpha;
	}
}

double UEyeXAxisFilter::LowPassFilter::filter(double value)
{
	double result;
	if (initialized)
		result = a*value + (1.0 - a)*s;
	else
	{
		result = value;
		initialized = true;
	}
	y = value;
	s = result;
	return result;
}

double UEyeXAxisFilter::LowPassFilter::filterWithAlpha(double value, double alpha)
{
	setAlpha(alpha);
	return filter(value);
}

bool UEyeXAxisFilter::LowPassFilter::hasLastRawValue(void)
{
	return initialized;
}

double UEyeXAxisFilter::LowPassFilter::lastRawValue(void)
{
	return y;
}




double UEyeXAxisFilter::alpha(double cutoff)
{
	double te = 1.0 / freq;
	double tau = 1.0 / (2 * M_PI*cutoff);
	return 1.0 / (1.0 + tau / te);
}

void UEyeXAxisFilter::setFrequency(double f)
{
	if (f <= 0)
	{
		//throw std::exception("freq should be >0");
		freq = 1.0;
	}
	else
	{
		freq = f;
	}
}

void UEyeXAxisFilter::setMinCutoff(double mc)
{
	if (mc <= 0)
	{
		//throw std::exception("mincutoff should be >0");
		mincutoff = 1.0;
	}
	else
	{
		mincutoff = mc;
	}
}

void UEyeXAxisFilter::setBeta(double b)
{
	beta_ = b;
}

void UEyeXAxisFilter::setDerivateCutoff(double dc)
{
	if (dc <= 0)
	{
		//throw std::exception("dcutoff should be >0");
		dcutoff = 1.0;
	}
	else
	{
		dcutoff = dc;
	}
}

void UEyeXAxisFilter::Init(float freq, float mincutoff, float beta_, float dcutoff, float threshold)
{
	setFrequency(freq);
	setMinCutoff(mincutoff);
	setBeta(beta_);
	setDerivateCutoff(dcutoff);
	x = new LowPassFilter(alpha(mincutoff));
	dx = new LowPassFilter(alpha(dcutoff));
	lasttime = UndefinedTime;

	lastValue = 0.0f;
}

float UEyeXAxisFilter::filter(float value, float timestamp)
{
	// update the sampling frequency based on timestamps
	if (lasttime != UndefinedTime && timestamp != UndefinedTime)
		freq = 1.0 / (timestamp - lasttime);
	lasttime = timestamp;
	// estimate the current variation per second 
	double dvalue = x->hasLastRawValue() ? (value - x->lastRawValue())*freq : 0.0; // FIXME: 0.0 or value?
	double edvalue = dx->filterWithAlpha(dvalue, alpha(dcutoff));
	// use it to update the cutoff frequency
	double cutoff = mincutoff + beta_*fabs(edvalue);
	// filter the given value
	float filtered = x->filterWithAlpha(value, alpha(cutoff));

	float check = lastValue - filtered;
	filtered = (check <= thresholdCheck && (-check) <= thresholdCheck) ? lastValue : filtered;
	lastValue = filtered;

	return filtered;
}

UEyeXAxisFilter* UEyeXAxisFilter::Create()
{
	return NewObject<UEyeXAxisFilter>(UEyeXAxisFilter::StaticClass());
}

UEyeXAxisFilter::~UEyeXAxisFilter(void)
{
	delete x;
	delete dx;
	x = nullptr;
	dx = nullptr;
}


