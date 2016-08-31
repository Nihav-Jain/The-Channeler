#include "TheChanneler.h"
#include "WidgetFocusDetector.h"
#include "Blueprint/WidgetLayoutLibrary.h"

UWidgetFocusDetector::UWidgetFocusDetector() :
	bCachedFocused(false)
{
}

void UWidgetFocusDetector::PostInitProperties()
{
	Super::PostInitProperties();

	EyeXEx = &UEyeXPluginEx::Get();

	TickDelegate = FTickerDelegate::CreateUObject(this, &UWidgetFocusDetector::Tick);
	TickDelegateHandle = FTicker::GetCoreTicker().AddTicker(TickDelegate);
}

void UWidgetFocusDetector::BeginDestroy()
{
	Super::BeginDestroy();

	FTicker::GetCoreTicker().RemoveTicker(TickDelegateHandle);
}

void UWidgetFocusDetector::SetWidgetInfo(const FWidgetInfo & widgetInfo)
{
	WidgetInfo = widgetInfo;
}

bool UWidgetFocusDetector::Tick(float DeltaSeconds)
{
	if (WidgetInfo.Widget != nullptr) {
		const UWorld * world = GEngine->GetWorldFromContextObject(WidgetInfo.Widget);
		if (world != nullptr && (world->WorldType == EWorldType::Type::Game || world->WorldType == EWorldType::Type::PIE)) {
			UCanvasPanelSlot* canvasSlot = Cast<UCanvasPanelSlot>(WidgetInfo.Widget->Slot);
			if (canvasSlot != nullptr) {
				FVector2D vewportSize = UWidgetLayoutLibrary::GetViewportSize(WidgetInfo.Widget);
				float scale = UWidgetLayoutLibrary::GetViewportScale(WidgetInfo.Widget);
				auto anchors = canvasSlot->GetAnchors();
				auto margin = canvasSlot->GetOffsets();

				bool focused = false;

				auto eyePos = EyeXEx->GetGazePoint().Value / scale;
				vewportSize /= scale;
				FVector2D pos;
				pos.X = anchors.Minimum.X * vewportSize.X + margin.Left;
				pos.Y = anchors.Minimum.Y * vewportSize.Y + margin.Top;

				float width = FMath::IsNearlyZero(anchors.Maximum.X) ? margin.Right : FMath::Abs(anchors.Maximum.X * vewportSize.X - margin.Right - pos.X);
				float height = FMath::IsNearlyZero(anchors.Maximum.Y) ? margin.Bottom : FMath::Abs(anchors.Maximum.Y * vewportSize.Y - margin.Bottom - pos.Y);

				if (eyePos.X < pos.X + width &&
					eyePos.X > pos.X &&
					eyePos.Y < pos.Y + height &&
					eyePos.Y > pos.Y) {
					focused = true;
				}


				if (focused != bCachedFocused) {
					bCachedFocused = focused;
					WidgetInfo.FocusChanged(bCachedFocused);

					if (focused) {
						WidgetInfo.Widget->SetKeyboardFocus();
					}
				}
			}
		}
	}
	return true;
}
