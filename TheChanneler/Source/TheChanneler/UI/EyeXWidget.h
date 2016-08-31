#pragma once
#include "WidgetFocusDetector.h"
#include "EyeXWidget.generated.h"

#define LOCTEXT_NAMESPACE "UMG"

// TODO: Write a macro to generate the class automatically

UCLASS(Blueprintable, BlueprintType)
class THECHANNELER_API UEyeXButton : public UButton
{
	GENERATED_BODY()
public:
	UEyeXButton() :
		bEyeFocused(false),
		Detector(nullptr)
	{
		Detector = CreateDefaultSubobject<UWidgetFocusDetector>("Detector");

		auto focuseChanged =
			[this](bool fosued) {
			this->bEyeFocused = fosued;
			fosued ? OnEyeFocused.Broadcast() : OnEyeLoseFocused.Broadcast();
		};

		FWidgetInfo widgetInfo(this, focuseChanged);
		Detector->SetWidgetInfo(widgetInfo);
	};

	UFUNCTION(BlueprintCallable, Category = "Button")
	bool IsEyeFocused() const { return bEyeFocused; };

	UPROPERTY(BlueprintAssignable, Category = "Button|Event")
	FOnEyeFocusedEvent OnEyeFocused;

	UPROPERTY(BlueprintAssignable, Category = "Button|Event")
	FOnEyeLoseFocusedEvent OnEyeLoseFocused;
#if WITH_EDITOR
	virtual const FText GetPaletteCategory() override { return LOCTEXT("EyeX", "EyeX"); };
#endif
private:
	UPROPERTY()
	UWidgetFocusDetector * Detector;

	bool bEyeFocused;
};

UCLASS(Blueprintable, BlueprintType)
class THECHANNELER_API UEyeXImage : public UImage
{
	GENERATED_BODY()
public:
	UEyeXImage() :
		bEyeFocused(false),
		Detector(nullptr)
	{
		Detector = CreateDefaultSubobject<UWidgetFocusDetector>("Detector");

		auto focuseChanged =
			[this](bool fosued) {
			this->bEyeFocused = fosued;
			fosued ? OnEyeFocused.Broadcast() : OnEyeLoseFocused.Broadcast();
		};

		FWidgetInfo widgetInfo(this, focuseChanged);
		Detector->SetWidgetInfo(widgetInfo);
	};

	UFUNCTION(BlueprintCallable, Category = "Image")
		bool IsEyeFocused() const { return bEyeFocused; };

	UPROPERTY(BlueprintAssignable, Category = "Image|Event")
		FOnEyeFocusedEvent OnEyeFocused;

	UPROPERTY(BlueprintAssignable, Category = "Image|Event")
		FOnEyeLoseFocusedEvent OnEyeLoseFocused;
#if WITH_EDITOR
	virtual const FText GetPaletteCategory() override { return LOCTEXT("EyeX", "EyeX"); };
#endif
private:
	UPROPERTY()
		UWidgetFocusDetector * Detector;

	bool bEyeFocused;
};