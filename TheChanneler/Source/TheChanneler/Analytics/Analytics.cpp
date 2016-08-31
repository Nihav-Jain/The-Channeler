#include "TheChanneler.h"
#include "AnalyticsModule.h"
#include "AnalyticsSession.h"
#include "Analytics.h"

UAnalytics::FStaticInitializer UAnalytics::Initializer;

UAnalytics::UAnalytics() :
	AnalyticsSession(nullptr),
	AnalyticsWorld(Initializer.AnalyticsWorld),
	CurrentLevel(Initializer.CurrentLevel)
{
}

void UAnalytics::PostInitProperties()
{
	Super::PostInitProperties();

	TickDelegate = FTickerDelegate::CreateUObject(this, &UAnalytics::Tick);
	TickDelegateHandle = FTicker::GetCoreTicker().AddTicker(TickDelegate);
	WorldCreatedDelegateHandle = FWorldDelegates::OnPostWorldInitialization.AddUObject(this, &UAnalytics::OnWorldChanged); // TODO: This might need to be modified
}

void UAnalytics::BeginDestroy()
{
	Super::BeginDestroy();

	FTicker::GetCoreTicker().RemoveTicker(TickDelegateHandle);
	FWorldDelegates::OnPostWorldInitialization.Remove(WorldCreatedDelegateHandle);

	StopSession();
}

UAnalyticsModule* UAnalytics::GetAnalyticsModule(UClass* objClass) const
{
	UAnalyticsModule* rt = nullptr;
	for (auto module : Modules) {
		if (module->GetClass() == objClass) {
			rt = module;
			break;
		}
	}
	return rt;
}

void UAnalytics::StartSession(const FString & userId)
{
	if (AnalyticsSession != nullptr) {
		AnalyticsSession->Stop();
	}
	AnalyticsSession = NewObject<UAnalyticsSession>(this, TEXT("UAnalytics_AnalyticsSession"));
	AnalyticsSession->Start(userId);
	AnalyticsSession->OnLevelChanged(AnalyticsWorld, CurrentLevel, AnalyticsWorld->GetName() + "_" + CurrentLevel->GetName());
	InitAll();
}

void UAnalytics::StopSession()
{
	if (AnalyticsSession != nullptr) {
		AnalyticsSession->Stop();
	}
	AnalyticsSession = nullptr;
	CleanUpAll();
}

void UAnalytics::ActivateModule(UClass * objClass)
{
	for (auto module : Modules) {
		if (module->GetClass() == objClass) {
			module->Activate();
			break;
		}
	}
}

void UAnalytics::DeactivateModule(UClass * objClass)
{
	for (auto module : Modules) {
		if (module->GetClass() == objClass) {
			module->Deactivate();
			break;
		}
	}
}

void UAnalytics::DeactivateAllModules()
{
	for (auto module : Modules) {
		module->Deactivate();
	}
}

void UAnalytics::ActivateAllModules()
{
	for (auto module : Modules) {
		module->Activate();
	}
}

void UAnalytics::InitAll()
{
	CleanUpAll();

	// Initialize the analytics modules
	LOG("Initializing analytics modules ...");

	TObjectRange<UClass> range;
	for (auto obj : range)
	{
		if (obj->IsChildOf<UAnalyticsModule>())
		{
			LOG("Activating %s ...", *obj->GetName());

			UAnalyticsModule* module = NewObject<UAnalyticsModule>(
				this,
				obj,
				FName(*obj->GetName())
				);

			if (module->bShouldBeInitiated) {

				module->SetAnalyticsSession(AnalyticsSession);
				module->InitModule();
				module->Init();
				module->PackageExternalDependency();
				module->OnLevelChanged(AnalyticsWorld, CurrentLevel, AnalyticsWorld->GetName() + "_" + CurrentLevel->GetName());
				Modules.Add(module);
			}

			module = nullptr;
		}
	}

	LOG("Initializing analytics modules is done. Size of the Modules is %d", Modules.Num());
}

void UAnalytics::CleanUpAll()
{
	// CleanUp each analytics module
	for (auto module : Modules) {
		LOG("Cleaning up... %s ...", *module->GetClass()->GetName());
		module->CleanUp();
	}

	Modules.Empty();
}

void UAnalytics::OnWorldCreated(UWorld * World, const UWorld::InitializationValues IVS)
{
	Initializer.AnalyticsWorld = World;
	Initializer.CurrentLevel = World->GetCurrentLevel();
}

bool UAnalytics::Tick(float DeltaSeconds)
{
	if (AnalyticsSession != nullptr) {
		AnalyticsSession->Update(DeltaSeconds);
	}
	// Update each analytics module
	for (auto module : Modules) {
		if (module->bIsEnabled) {
			module->Update(DeltaSeconds);
		}
	}
	return true;
}

void UAnalytics::OnWorldChanged(UWorld* World, const UWorld::InitializationValues IVS)
{
	if (World != nullptr) {
		LOG("World is changed");
		AnalyticsWorld = World;
		CurrentLevel = World->GetCurrentLevel();

		if (AnalyticsSession != nullptr) {
			AnalyticsSession->OnLevelChanged(AnalyticsWorld, CurrentLevel, AnalyticsWorld->GetName() + "_" + CurrentLevel->GetName());
		}

		for (auto module : Modules) {
			module->OnLevelChanged(AnalyticsWorld, CurrentLevel, AnalyticsWorld->GetName() + "_" + CurrentLevel->GetName());
		}
	}
}