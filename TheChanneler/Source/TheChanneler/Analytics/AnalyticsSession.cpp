#include "TheChanneler.h"
#include "JsonSerializer.h"
#include "ChannelerGameVersion.h"
#include "AnalyticsSession.h"

UAnalyticsSession::UAnalyticsSession() :
	bIsPaused(false),
	Data(new FJsonObject()), // TODO: Don't like this. But the TSharedRef doesn't allow us to pass nullptr
	EyeXEx(nullptr)
{
}

UAnalyticsSession::~UAnalyticsSession()
{
	Stop();
}

void UAnalyticsSession::PostInitProperties()
{
	Super::PostInitProperties();

	if (!UEyeXPluginEx::IsAvailable())
	{
		LOG("TobiiEyeX Module is unavailable.");
		return;
	}

	EyeXEx = &UEyeXPluginEx::Get();
}

void UAnalyticsSession::Update(float DeltaSeconds)
{
	static float cnt = 0;
	if (!bIsPaused) {
		cnt += DeltaSeconds;
		if (cnt > 10.0) {
			// TODO: This is ugly... need to support streaming and buffer writing into log files
			cnt = 0;
			WriteToLog();
		}
	}
}

bool UAnalyticsSession::LogNameValueToSection(const FString & section, const FString & name, const FString & value, bool overwrite)
{
	FScopeLock f(&Mutex);

	bool result = false;

	TArray<FString> sectionsArray;
	section.ParseIntoArray(sectionsArray, TEXT("/"));

	TSharedPtr<FJsonObject> sectionObj = Data;
	for (auto & sec : sectionsArray)
	{
		if (!sectionObj->HasField(sec)) {
			sectionObj->SetObjectField(sec, TSharedRef<FJsonObject>(new FJsonObject()));
		}

		sectionObj = sectionObj->GetObjectField(sec);
	}

	if (!sectionObj->HasField(name) || overwrite)
	{
		sectionObj->SetStringField(name, value);
		bDirty = true;
		result = true;
	}
	return result;
}

bool UAnalyticsSession::LogNameValue(const FString & name, const FString & value, bool overwrite)
{
	return LogNameValueToSection("/", name, value, overwrite);
}

void UAnalyticsSession::Start(const FString & userId)
{
	FScopeLock f(&Mutex);

	Data = TSharedRef<FJsonObject>(new FJsonObject());

	UserId = userId;
	SessionTime = FDateTime::Now();
	SessionId = FGuid::NewGuid();
	RootPath = FPaths::GameSavedDir() + "/ChannelerAnalytics/" + UserId + "_" + SessionTime.ToString();

	LogNameValue(TEXT("session_id"), SessionId.ToString(), true);
	LogNameValue(TEXT("user_id"), userId, true);
	LogNameValue(TEXT("start_time"), SessionTime.ToString(), true);

	LogNameValueToSection(TEXT("machine"), TEXT("os"), FPlatformProperties::PlatformName(), true);
	LogNameValueToSection(TEXT("machine"), TEXT("cpu"), FPlatformMisc::GetCPUVendor() + " " + FPlatformMisc::GetCPUBrand(), true);
	LogNameValueToSection(TEXT("machine"), TEXT("gpu"), FPlatformMisc::GetPrimaryGPUBrand(), true); // TODO: This will return the Intel one

	LogNameValueToSection(TEXT("game"), TEXT("version"), TEXT(CH_GAME_VERSION), true);  // TODO: This need to be integrated with p4 plugin
	LogNameValueToSection(TEXT("game"), TEXT("build"), TEXT(CH_GAME_BUILD), true);  // TODO: This need to be integrated with p4 plugin

	FString profileName = EyeXEx->GetProfileName();

	FString name;
	FString type;
	if (!profileName.Split(TEXT(" - "), &name, &type)) {
		name = profileName;
		type = TEXT("");
	}

	LogNameValueToSection(TEXT("eyex/profile"), TEXT("name"), name, true);
	LogNameValueToSection(TEXT("eyex/profile"), TEXT("type"), type, true);


	const auto & bound = EyeXEx->GetScreenBounds().Value;
	LogNameValueToSection(TEXT("eyex/screen/bound"), TEXT("x"), FString::FromInt(bound.X), true);
	LogNameValueToSection(TEXT("eyex/screen/bound"), TEXT("y"), FString::FromInt(bound.Y), true);
	LogNameValueToSection(TEXT("eyex/screen/bound"), TEXT("width"), FString::FromInt(bound.Width), true);
	LogNameValueToSection(TEXT("eyex/screen/bound"), TEXT("height"), FString::FromInt(bound.Height), true);

	const auto & display = EyeXEx->GetDisplaySize().Value;
	LogNameValueToSection(TEXT("eyex/screen/display"), TEXT("x"), FString::FromInt(display.X), true);
	LogNameValueToSection(TEXT("eyex/screen/display"), TEXT("y"), FString::FromInt(display.Y), true);
}

void UAnalyticsSession::Stop()
{
	Duration = FDateTime::Now() - SessionTime;

	LogNameValue(TEXT("duration"), Duration.ToString(), true);

	if (LeveName != TEXT("")) {
		LevelDuration = FDateTime::Now() - LevelTime;
		LogNameValueToSection("game/level/" + LeveName, TEXT("duration"), LevelDuration.ToString(), true);
		LeveName = TEXT("");
	}

	WriteToLog();
}

void UAnalyticsSession::OnLevelChanged(UWorld * world, ULevel * level, const FString & name)
{
	FScopeLock f(&Mutex);
	if (LeveName != TEXT("")) {
		LevelDuration = FDateTime::Now() - LevelTime;
		LogNameValueToSection("game/level/" + LeveName, TEXT("duration"), LevelDuration.ToString(), true);
	}

	LeveName = name;
	LevelTime = FDateTime::Now();
	LogNameValueToSection("game/level/" + LeveName, TEXT("start_time"), LevelTime.ToString(), true);
	
}

const FString & UAnalyticsSession::GetLevelName()
{
	return LeveName;
}

void UAnalyticsSession::WriteToLog()
{
	FScopeLock f(&Mutex);
	// TODO: Now it just rewrite the whole file.. need to redo this
	if (bDirty) {
		bDirty = false;

		auto & platformFile = FPlatformFileManager::Get().GetPlatformFile();

		if (!platformFile.DirectoryExists(*RootPath))
		{
			platformFile.CreateDirectory(*RootPath);
		}

		FString JSONStr;

		TSharedRef<TJsonWriter<TCHAR>> writer = TJsonWriterFactory<>::Create(&JSONStr);
		FJsonSerializer::Serialize<TCHAR>(Data, writer, true);

		FString fileToWrite = RootPath + "/session.json";

		FFileHelper::SaveStringToFile(JSONStr, *fileToWrite);
	}
}
