#include "TheChanneler.h"
#include "EagleEyeVolume.h"

AEagleEyeVolume * AEagleEyeVolume::CurrentInstance = nullptr;

AEagleEyeVolume::AEagleEyeVolume()
{
	// TODO: This is quick and dirty I shouldn't use this single instance for detection by Yuhsiang.
	// However....I wanna spend more time on ppp tho.
	CurrentInstance = nullptr;
}

void AEagleEyeVolume::BeginPlay()
{
	auto boxVolume = Cast<UBoxComponent>(GetComponentByClass(UBoxComponent::StaticClass()));
	TArray<AActor*> Objects;
	boxVolume->GetOverlappingActors(Objects);
	bool initTo1 = false;
	for (auto & obj : Objects)
	{
		if (Cast<ACharacter>(obj) != nullptr) {
			initTo1 = true;
		}

		auto mesh = Cast<UStaticMeshComponent>(obj->GetComponentByClass(UStaticMeshComponent::StaticClass()));
		if (mesh != nullptr) {
			auto materials = mesh->GetMaterials();
			for (int32 i = 0; i < materials.Num(); ++i) {
				auto material = materials[i];
				if (material != nullptr) {
					float value;
					if (material->GetScalarParameterValue(TEXT("EagleEyeInstanceIntensity"), value))
					{
						auto materialDynamic = mesh->CreateDynamicMaterialInstance(i, material);
						if (materialDynamic != nullptr) {
							EagleEyeMaterials.Add(materialDynamic);
						}
					}
				}
			}
		}
		
		// Decal
		auto decal = Cast<UDecalComponent>(obj->GetComponentByClass(UDecalComponent::StaticClass()));
		if (decal != nullptr) {
			auto material = decal->GetDecalMaterial();
			if (material != nullptr) {
				float value;
				if (material->GetScalarParameterValue(TEXT("EagleEyeInstanceIntensity"), value))
				{
					auto materialDynamic = decal->CreateDynamicMaterialInstance();
					check(material != materialDynamic);
					if (materialDynamic != nullptr) {
						check(decal->GetDecalMaterial() == materialDynamic);
						check(decal->GetDecalMaterial()->GetScalarParameterValue(TEXT("EagleEyeInstanceIntensity"), value));
						EagleEyeMaterials.Add(materialDynamic);
					}
				}
			}
		}
	}

	if (initTo1) {
		for (auto material : EagleEyeMaterials) {
			material->SetScalarParameterValue(TEXT("EagleEyeInstanceIntensity"), 1.0f);
		}
	}

}

void AEagleEyeVolume::SetAllEagleEyeMaterialIntensity(float intensity, float group)
{
	for (auto material : EagleEyeMaterials) {
		float value = 0.0f;
		material->GetScalarParameterValue(TEXT("Group"), value);
		if (FMath::IsNearlyEqual(value, group) || FMath::IsNearlyEqual(group, -1.0f)) {
			material->SetScalarParameterValue(TEXT("EagleEyeInstanceIntensity"), intensity);
		}
	}
}

void AEagleEyeVolume::BeginDestroy() {
	if (CurrentInstance == this)
		CurrentInstance = nullptr;
	Super::BeginDestroy();
}