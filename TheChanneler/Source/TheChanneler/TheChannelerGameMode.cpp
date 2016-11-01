// Fill out your copyright notice in the Description page of Project Settings.

#include "TheChanneler.h"
#include "Kernel/GameInstanceBase.h"
#include "TheChannelerGameMode.h"

ATheChannelerGameMode::ATheChannelerGameMode() :
	bSimulateEyeX(false)
{}

void ATheChannelerGameMode::BeginPlay()
{
	Super::BeginPlay();
	
	IEyeXPlugin& eyeX = IEyeXPlugin::Get();
	EEyeXDeviceStatus::Type deviceStatus = eyeX.GetEyeTrackingDeviceStatus();

	//switch (deviceStatus)
	//{
	//case EEyeXDeviceStatus::Disabled:		// intentional fall through
	//case EEyeXDeviceStatus::NotAvailable:	// intentional fall through
	//case EEyeXDeviceStatus::Unknown:
	//	UE_LOG(LogTemp, Warning, TEXT("Not Tracking"));
	//	eyeX.SetEmulationMode(EEyeXEmulationMode::Enabled);
	//	eyeX.SetEmulationPointType(EEyeXEmulationPoint::ScreenCenter);
	//	break;
	//case EEyeXDeviceStatus::Tracking:
	//	UE_LOG(LogTemp, Warning, TEXT("Tracking"));
	//	eyeX.SetEmulationMode(EEyeXEmulationMode::Disabled);
	//	break;
	//}

	if (bSimulateEyeX)
	{
		eyeX.SetEmulationMode(EEyeXEmulationMode::Enabled);
		eyeX.SetEmulationPointType(EEyeXEmulationPoint::ScreenCenter);
	}
	else
	{
		eyeX.SetEmulationMode(EEyeXEmulationMode::Disabled);
	}

	ChangeMenuWidget(StartingWidgetClass);
	EyeXEx = nullptr;
}

void ATheChannelerGameMode::ChangeMenuWidget(TSubclassOf<UUserWidget> NewWidgetClass)
{
	if (CurrentWidget != nullptr)
	{
		CurrentWidget->RemoveFromViewport();
		CurrentWidget = nullptr;
	}
	if (NewWidgetClass != nullptr)
	{
		CurrentWidget = CreateWidget<UUserWidget>(GetWorld(), NewWidgetClass);
		if (CurrentWidget != nullptr)
		{
			CurrentWidget->AddToViewport();
		}
	}
}

UUserWidget* ATheChannelerGameMode::GetCurrentMenuWidget()
{
	return CurrentWidget;
}

void ATheChannelerGameMode::ReadFile(FString& StringData, FString FileNameWithExtension)
{
	FileContent = &StringData;
	FString FolderPath = FPaths::GameDir() + "/ChannelerFiles/" + FileNameWithExtension;
	FFileHelper::LoadFileToString(StringData, *FolderPath);
}


void ATheChannelerGameMode::SaveFile(FString StringData, FString FileNameWithExtension)
{
	FString FolderPath = FPaths::GameDir() + "/ChannelerFiles/" + FileNameWithExtension;
	FFileHelper::SaveStringToFile(StringData, *FolderPath);
}



void ATheChannelerGameMode::StringAtLineNumber(FString&  Data, int32 LineNumber)
{
	FString aData = *FileContent;
	TArray<FString> OutArray;
	aData.ParseIntoArrayLines(OutArray);
	Data = OutArray[LineNumber];
}

void ATheChannelerGameMode::StringAtColumnNumber(FString InputData, FString&  Data, int32 ColumnNumber)
{

	FString aData = InputData;
	TArray<FString> OutArray;
	aData.ParseIntoArray(OutArray, TEXT(","));
	Data = OutArray[ColumnNumber];
}


void ATheChannelerGameMode::ReplaceStringAtColumnNumber(FString InputData, FString StringToReplace, FString&  Data, int32 ColumnNumber)
{
	FString aData = InputData;
	TArray<FString> OutArray;
	aData.ParseIntoArray(OutArray, TEXT(","));
	OutArray[ColumnNumber] = StringToReplace;
	aData = "";
	for (int32 i = 0; i < OutArray.Max() - 1; ++i)
	{
		aData.Append(OutArray[i] + ",");
	}
	aData.RemoveFromEnd(",");
	Data = aData;
}



void ATheChannelerGameMode::ReplaceStringAtLineNumber(FString InputData, FString StringToReplace, FString&  Data, int32 LineNumber)
{
	FString aData;/* = InputData;// kept here, read the comments.*/
	aData = *FileContent;
	TArray<FString> OutArray;
	aData.ParseIntoArrayLines(OutArray);

	OutArray[LineNumber] = StringToReplace;

	aData = "";
	for (int32 i = 0; i < OutArray.Max(); ++i)
	{
		aData.Append(OutArray[i] + "\n");
	}

	aData.RemoveFromEnd("\n");
	Data = aData;
}

void ATheChannelerGameMode::LaunchCalibration(const FString& OutStringData)
{
	//EyeXEx->setp
	EyeXEx = &UEyeXPluginEx::Get();
	EyeXEx->LaunchCalibrationTool([this](int result) {
		UE_LOG(LogTemp, Log, TEXT("LaunchCalibration result code: %d."), (int32)result);

		UWorld* world = GetWorld();
		if (world != nullptr)
		{
			UGameInstance* gameInstance = world->GetGameInstance();
			if (gameInstance != nullptr)
			{
				UGameInstanceBase* gameInstanceBase = Cast<UGameInstanceBase>(gameInstance);
				gameInstanceBase->ReinforceScreenResolution();
			}
		}

		if (AppRegainedFocus.IsBound())
			AppRegainedFocus.Broadcast();
	});
}

FString ATheChannelerGameMode::GetPlayerName()
{
	return PlayerName;
	//	GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Red, FString::Printf(TEXT("///////////////////////////////////////////////.")));//debug
}

void ATheChannelerGameMode::UpdateProfile_Implementation()
{

}


TArray<FString> ATheChannelerGameMode::GetProfiles()
{
	//EyeXEx->setp
	EyeXEx = &UEyeXPluginEx::Get();
	TArray<FString> profiles = EyeXEx->GetProfiles();
	return profiles;
	//	GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Red, FString::Printf(TEXT("///////////////////////////////////////////////.")));//debug
}



void ATheChannelerGameMode::SetProfile(const FString& Profile)
{
	//EyeXEx->setp
	EyeXEx = &UEyeXPluginEx::Get();
	EyeXEx->SetProfile(Profile);
}


FString ATheChannelerGameMode::GetCurrentProfile()
{
	EyeXEx = &UEyeXPluginEx::Get();
	return EyeXEx->GetProfileName();
}

bool ATheChannelerGameMode::IsEyeXSimulating() const
{
	return bSimulateEyeX;
}