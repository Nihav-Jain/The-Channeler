#include "AnalyticsModule.h"
#include "AnalyticsCommon.h"
#include "TestAnalyticsModule.generated.h"

UCLASS(Blueprintable, BlueprintType)
class UTestAnalyticsModule : public UAnalyticsModule
{
	GENERATED_BODY()
public:
	UTestAnalyticsModule();

	virtual void Init() override;
	virtual void CleanUp() override;
	virtual void Update(float DeltaSeconds) override;
	virtual void OnLevelChanged(UWorld * world, ULevel * level, const FString & name) override;
};