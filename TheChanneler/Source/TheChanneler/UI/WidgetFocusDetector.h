#pragma once
#include <functional>
#include "UMG.h"
#include "UMGStyle.h"
#include "Slate/SObjectWidget.h"
#include "IUMGModule.h"
#include "Blueprint/UserWidget.h"
#include "EyeXEx/EyeXPluginEx.h"
#include "WidgetFocusDetector.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnEyeFocusedEvent);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnEyeLoseFocusedEvent);

struct FWidgetInfo
{
	FWidgetInfo() : Widget(nullptr) {};
	FWidgetInfo(UWidget * widget, const std::function<void(bool)> & focusChanged) :
		Widget(widget), FocusChanged(focusChanged){};

	UWidget * Widget;
	std::function<void(bool)> FocusChanged;
};

UCLASS()
class THECHANNELER_API UWidgetFocusDetector : public UObject
{
	GENERATED_BODY()
public:
	UWidgetFocusDetector();

	bool Tick(float DeltaSeconds);
	virtual void PostInitProperties() override;
	virtual void BeginDestroy() override;

	/** Set the widget information to be tested
	* @param widgetInfo the widget information
	*/
	void SetWidgetInfo(const FWidgetInfo & widgetInfo);
private:
	/** Delegate for callbacks to Tick */
	FTickerDelegate TickDelegate;
	FDelegateHandle TickDelegateHandle;

	FWidgetInfo WidgetInfo;
	UEyeXPluginEx * EyeXEx;
	bool bCachedFocused;
};
