#include "TheChanneler.h"
#include "TestAnalyticsModule.h"

UTestAnalyticsModule::UTestAnalyticsModule()
{
	bShouldBeInitiated = false;
}

void UTestAnalyticsModule::Init()
{
	GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, TEXT("UTestAnalyticsModule enabled"));

	LOG("Root path of the the module is %s", *GetModuleRootPath());
	LOG("Log path of the the module is %s", *GetModuleLogPath());
	LOG("Dependency path of the the module is %s", *GetModuleDependencyPath());
	LOG("Root path of the common module is %s", *GetSessionRootPath());

	LogNameValue(TEXT("test0"), TEXT("value"));
	LogNameValueToSection(TEXT("test"), TEXT("test0"), TEXT("value"));
	LogNameValueToSection(TEXT("test/sec1"), TEXT("test1"), TEXT("value"));
	LogNameValueToSection(TEXT("/test/sec2"), TEXT("test2"), TEXT("value"));
	LogNameValueToSection(TEXT("//test///sec3"), TEXT("test3"), TEXT("value"));

	AddExternalDependency(FPaths::GameSavedDir() + "/test.py");
	AddExternalDependency(FPaths::GameSavedDir() + "/AutoScreenshot.png");
}

void UTestAnalyticsModule::CleanUp()
{
	GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, TEXT("UTestAnalyticsModule enabled"));

	ShellExec(TEXT("test.py 3"));
	ShellExec(TEXT("AutoScreenshot.png"));
}

void UTestAnalyticsModule::Update(float DeltaSeconds)
{
}

void UTestAnalyticsModule::OnLevelChanged(UWorld * world, ULevel * level, const FString & name)
{
	LOG("New level name %s:", *name);
}
