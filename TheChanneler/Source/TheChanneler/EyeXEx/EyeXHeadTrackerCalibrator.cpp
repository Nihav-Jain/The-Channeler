#include "TheChanneler.h"
#include "EyeXHeadTrackerCalibrator.h"


UEyeXHeadTrackerCalibrator::UEyeXHeadTrackerCalibrator() :
	EyeXEx(&UEyeXPluginEx::Get()),
	Origin(EyeXEx->GetEyePosition()), AccumulatedOrigin(FEyeXEyePosition::Invalid()),
	NumTicks(0.0f),
	Filter(nullptr)
{
	// TODO: Sync the filter with the Character's filter
	if (Filter == nullptr) {
		Filter = NewObject<UEyeXAxisFilter>(UEyeXAxisFilter::StaticClass());
		Filter->Init(60, 1.0f, 0.007f, 1.0f, 1000);
	}
}

UEyeXHeadTrackerCalibrator::~UEyeXHeadTrackerCalibrator(void)
{

}

void UEyeXHeadTrackerCalibrator::PostInitProperties()
{
	Super::PostInitProperties();
}

void UEyeXHeadTrackerCalibrator::BeginDestroy()
{
	Super::BeginDestroy();

	if (TickDelegateHandle.IsValid()) {
		FTicker::GetCoreTicker().RemoveTicker(TickDelegateHandle);
		TickDelegateHandle.Reset();
	}
}

void UEyeXHeadTrackerCalibrator::FilterEyePositionData(FEyeXEyePosition & position)
{
	check(Filter != nullptr);

	position.LeftEye.X = Filter->filter(position.LeftEye.X, NumTicks);
	position.LeftEye.Y = Filter->filter(position.LeftEye.Y, NumTicks);
	position.LeftEye.Z = Filter->filter(position.LeftEye.Z, NumTicks);

	position.RightEye.X = Filter->filter(position.RightEye.X, NumTicks);
	position.RightEye.Y = Filter->filter(position.RightEye.Y, NumTicks);
	position.RightEye.Z = Filter->filter(position.RightEye.Z, NumTicks);
}

void UEyeXHeadTrackerCalibrator::BeginCalibration()
{
	Origin = EyeXEx->GetEyePosition();
	FilterEyePositionData(Origin);

	AccumulatedOrigin = FEyeXEyePosition::Invalid();
	NumTicks = 0.0f;
	NumFrame = 0.0f;

	TickDelegate = FTickerDelegate::CreateUObject(this, &UEyeXHeadTrackerCalibrator::CalibrationTick);
	TickDelegateHandle = FTicker::GetCoreTicker().AddTicker(TickDelegate);

	UE_LOG(LogTemp, Warning, TEXT("BeginCalibration, LeftEye X: %f Y: %f Z: %f , RightEye X: %f Y: %f Z: %f "), 
		Origin.LeftEye.X, Origin.LeftEye.X, Origin.LeftEye.Z,
		Origin.RightEye.X, Origin.RightEye.X, Origin.RightEye.Z);
}

void UEyeXHeadTrackerCalibrator::EndCalibration()
{
	Origin = AccumulatedOrigin / NumFrame;

	if (TickDelegateHandle.IsValid()) {
		FTicker::GetCoreTicker().RemoveTicker(TickDelegateHandle);
		TickDelegateHandle.Reset();
	}

	UE_LOG(LogTemp, Warning, TEXT("BeginCalibration, LeftEye X: %f Y: %f Z: %f , RightEye X: %f Y: %f Z: %f "),
		Origin.LeftEye.X, Origin.LeftEye.X, Origin.LeftEye.Z,
		Origin.RightEye.X, Origin.RightEye.X, Origin.RightEye.Z);
}

void UEyeXHeadTrackerCalibrator::GetHeadOrigin(FVector & LeftEye, FVector& RightEye)
{
	LeftEye = Origin.LeftEye;
	RightEye = Origin.RightEye;
}

bool UEyeXHeadTrackerCalibrator::CalibrationTick(float DeltaSeconds)
{
	auto currentPos = EyeXEx->GetEyePosition();

	FilterEyePositionData(currentPos);

	if (currentPos.bIsLeftEyeValid && currentPos.bIsRightEyeValid) {
		AccumulatedOrigin = currentPos + AccumulatedOrigin;
		NumFrame += 1.0f;
	}

	NumTicks += DeltaSeconds;

	return true;
}


