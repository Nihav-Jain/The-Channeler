#pragma once
#include "EagleEyeVolume.generated.h"


UCLASS(Blueprintable, BlueprintType)
class THECHANNELER_API AEagleEyeVolume : public AActor
{
	friend class UEagleEyeUtil;
	GENERATED_BODY()

public:
	AEagleEyeVolume();

	virtual void BeginPlay() override;
	virtual void BeginDestroy() override;
public:
	
	/**
	* Set All EagleEye Material Intensity
	*/
	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Set All EagleEye Material Intensity"), Category = "EagleEye")
	void SetAllEagleEyeMaterialIntensity(float intensity, float group);

private:
	TArray<UMaterialInstanceDynamic*> EagleEyeMaterials;

	static AEagleEyeVolume * CurrentInstance;
};

UCLASS()
class THECHANNELER_API UEagleEyeUtil : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
public:

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Set the active eagle eye volume"), Category = "EagleEye")
	static void SetActiveInstance(AActor * actor, AEagleEyeVolume * instance) {
		if (Cast<ACharacter>(actor) != nullptr)
		{
			AEagleEyeVolume::CurrentInstance = instance;
		}
	}

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Get the active eagle eye volume"), Category = "EagleEye")
	static AEagleEyeVolume * GetActiveInstance() {
		return AEagleEyeVolume::CurrentInstance;
	}
};