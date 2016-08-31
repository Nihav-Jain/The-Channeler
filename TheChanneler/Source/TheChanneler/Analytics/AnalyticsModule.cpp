#include "TheChanneler.h"
#include "AnalyticsModule.h"
#include "AnalyticsSession.h"

bool UAnalyticsModule::LogNameValue(const FString & name, const FString & value, bool overwrite)
{
	check(AnalyticsSession != nullptr);
	return AnalyticsSession->LogNameValueToSection(GetClass()->GetName(), name, value, overwrite);
}

bool UAnalyticsModule::LogNameValueToSection(const FString & section, const FString & name, const FString & value, bool overwrite)
{
	check(AnalyticsSession != nullptr);
	return AnalyticsSession->LogNameValueToSection("/" + GetClass()->GetName() + "/" + section, name, value, overwrite);
}

const FString & UAnalyticsModule::GetSessionRootPath() const
{
	check(AnalyticsSession != nullptr);
	return AnalyticsSession->GetRootPath();
}

const FString & UAnalyticsModule::GetLevelName()
{
	check(AnalyticsSession != nullptr);
	return AnalyticsSession->GetLevelName();
}

void UAnalyticsModule::AddExternalDependency(const FString & absPath)
{
	ExternalDependencies.Add(absPath);
}

void UAnalyticsModule::ShellExec(const FString & cmd) const
{
	FString exec;
	FString args;

	cmd.Split(TEXT(" "), &exec, &args);
	FString ext = FPaths::GetExtension(exec);

	if (ext != TEXT("exe")) {
		if (ext == TEXT("py")) {
			exec = TEXT("python");
		}
		else {
			exec = TEXT("explorer");
		}
		args = cmd;
	}
	else {
		exec = FPaths::ConvertRelativePathToFull(DependencyPath + exec);
	}

	FString workDir = FPaths::ConvertRelativePathToFull(DependencyPath);

	auto handle = FPlatformProcess::CreateProc(*exec, *cmd, true, false, false, nullptr, 0,
		*workDir, nullptr);
}

UAnalyticsModule::UAnalyticsModule() :
	bIsEnabled(false),
	bShouldBeInitiated(true),
	AnalyticsSession(nullptr)
{
}

void UAnalyticsModule::InitModule()
{
	check(AnalyticsSession != nullptr);

	RootPath = GetSessionRootPath() + "/" + GetClass()->GetName() + "/";
	LogPath = RootPath + "Log/";
	DependencyPath = RootPath + "Dependency/";

	auto & platformFile = FPlatformFileManager::Get().GetPlatformFile();

	if (!platformFile.DirectoryExists(*LogPath))
	{
		platformFile.CreateDirectoryTree(*LogPath);
	}

	if (!platformFile.DirectoryExists(*DependencyPath))
	{
		platformFile.CreateDirectoryTree(*DependencyPath);
	}
}

void UAnalyticsModule::PackageExternalDependency()
{
	auto & platformFile = FPlatformFileManager::Get().GetPlatformFile();
	for (auto & dependency : ExternalDependencies) {

		FString name = FPaths::GetCleanFilename(dependency);
		if (!platformFile.CopyFile(*(DependencyPath + "/" + name), *dependency)) {
			LOG("Dependency %s copying failed.", *dependency);
		}
	}
}
