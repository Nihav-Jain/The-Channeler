// Fill out your copyright notice in the Description page of Project Settings.

#include "TheChanneler.h"
#include "Kernel/ChannelerEyeXPlayerController.h"
#include "BlinkWinkLogger.h"

UBlinkWinkLogger::UBlinkWinkLogger() :
	mLogFileName(""), mTotalDeltaTime(0), MaxBufferBytes(1024), mTotalBytes(0),
	mController(nullptr), mFileHandle(nullptr), mWorld(nullptr),
	mArchive(nullptr), mEyeStatusLogData(nullptr), mBlinkLogData(nullptr), mWinkLogData(nullptr), mBothOpenLogData(nullptr),
	mBlinkDelegate(), mLeftEyeClosedDelegate(), mRightEyeClosedDelegate(), mBothEyesOpenDelegate(),
	mCacheCheckForClosedEye(false), bIsLevelChange(false)
{
	mBlinkDelegate.BindUFunction(this, "UserBlinked");
	mLeftEyeClosedDelegate.BindUFunction(this, "LeftEyeClosed");
	mRightEyeClosedDelegate.BindUFunction(this, "RightEyeClosed");
	mBothEyesOpenDelegate.BindUFunction(this, "BothEyesOpen");
}

void UBlinkWinkLogger::Init()
{
	UE_LOG(LogTemp, Warning, TEXT("BlinkWinkLogger initialized!"));
}

void UBlinkWinkLogger::Update(float DeltaSeconds)
{
	if (!bIsEnabled)
		return;

	mTotalDeltaTime += DeltaSeconds;

	*mEyeStatusLogData << mTotalDeltaTime;

	uint8 eyeByte = 1;
	if (mController->IsLeftEyeClosed())
		eyeByte = 0;
	*mEyeStatusLogData << eyeByte;

	eyeByte = 1;
	if (mController->IsRightEyeClosed())
		eyeByte = 0;
	*mEyeStatusLogData << eyeByte;

	mTotalBytes += 6;

	if (mTotalBytes + 15 >= MaxBufferBytes)
	{
		WriteBufferToFile();
		mTotalBytes = 0;
	}
}

void UBlinkWinkLogger::OnLevelChanged(UWorld* world, ULevel* level, const FString& name)
{
	mWorld = world;
	bIsLevelChange = true;
	Deactivate();
	bIsLevelChange = false;
}

void UBlinkWinkLogger::StartLoggerFromPuzzle(const FString& puzzleName)
{
	Deactivate();
	PuzzleName = puzzleName;
	Activate();
}

void UBlinkWinkLogger::Activate()
{
	if (bIsEnabled)
		return;

	UAnalyticsModule::Activate();

	mArchive = new FBufferArchive();
	mEyeStatusLogData = new FBufferArchive();
	mBlinkLogData = new FBufferArchive();
	mWinkLogData = new FBufferArchive();
	mBothOpenLogData = new FBufferArchive();

	UE_LOG(LogTemp, Warning, TEXT("BlinkWinkLogger activated!"));
	check(mWorld != nullptr);
	check(mWorld->GetFirstPlayerController() != nullptr);

	mController = Cast<AChannelerEyeXPlayerController>(mWorld->GetFirstPlayerController());

	check(mController != nullptr);
	mCacheCheckForClosedEye = mController->CheckForClosedEye;
	mController->CheckForClosedEye = true;

	mController->UserBlinked.AddUnique(mBlinkDelegate);
	mController->LeftEyeClosed.AddUnique(mLeftEyeClosedDelegate);
	mController->RightEyeClosed.AddUnique(mRightEyeClosedDelegate);
	mController->BothEyeOpened.AddUnique(mBothEyesOpenDelegate);

	MakeLogFileName();
	CreateLogFileHeader();

	if (!OpenFileHandle())
	{
		Deactivate();
	}
	else
	{
		mTotalBytes += mArchive->TotalSize();
	}
}

void UBlinkWinkLogger::Deactivate()
{
	if (!bIsEnabled)
		return;
	UE_LOG(LogTemp, Warning, TEXT("BlinkWinkLogger deactivated!"));
	UAnalyticsModule::Deactivate();
	if (mTotalBytes > 0)
		WriteBufferToFile();
	CloseFileHandle();

	delete mArchive;
	delete mEyeStatusLogData;
	delete mBlinkLogData;
	delete mWinkLogData;
	delete mBothOpenLogData;

	if (!bIsLevelChange && mController != nullptr)
	{
		mController->CheckForClosedEye = mCacheCheckForClosedEye;

		mController->UserBlinked.Remove(mBlinkDelegate);
		mController->LeftEyeClosed.Remove(mLeftEyeClosedDelegate);
		mController->RightEyeClosed.Remove(mRightEyeClosedDelegate);
		mController->BothEyeOpened.Remove(mBothEyesOpenDelegate);
	}
}

void UBlinkWinkLogger::MakeLogFileName()
{
	const FString filePrefix = "BlinkWink";
	const FString fileExtension = ".bin";
	mLogFileName = GetModuleLogPath() + PuzzleName + "_" + filePrefix + fileExtension;
}

void UBlinkWinkLogger::CreateLogFileHeader()
{
	uint32 endianness = 1;
	*mArchive << endianness;
	*mArchive << mController->MinEyeClosedDuration;
	*mArchive << mController->MinBlinkDuration;
	*mArchive << mController->MaxBlinkDuration;
}

bool UBlinkWinkLogger::OpenFileHandle()
{
	if (mFileHandle != nullptr)
	{
		CloseFileHandle();
	}

	IPlatformFile& file = FPlatformFileManager::Get().GetPlatformFile();
	mFileHandle = file.OpenWrite(*mLogFileName, true, false);
	if (!mFileHandle)
	{
		UE_LOG(LogTemp, Error, TEXT("ERROR: BlinkWinkLogger failed to open file %s. Logging data will not be recorded!"), *mLogFileName);
		GEngine->AddOnScreenDebugMessage(-1, 20.f, FColor::Red, TEXT("ERROR: BlinkWinkLogger failed to open file. Logging data will not be recorded!"));
		return false;
	}

	return true;
}

void UBlinkWinkLogger::CloseFileHandle()
{
	delete mFileHandle;
	mFileHandle = nullptr;
}

void UBlinkWinkLogger::WriteBufferToFile()
{
	mEyeStatusLogData = SaveSection(*mEyeStatusLogData, ELogSection::SECTION_EYE_STATUS);
	mBlinkLogData = SaveSection(*mBlinkLogData, ELogSection::SECTION_BLINK);
	mWinkLogData = SaveSection(*mWinkLogData, ELogSection::SECTION_WINK);
	mBothOpenLogData = SaveSection(*mBothOpenLogData, ELogSection::SECTION_BOTH_OPEN);

	CloseFileHandle();
	if (!OpenFileHandle())
		Deactivate();
}

FBufferArchive* UBlinkWinkLogger::SaveSection(FBufferArchive& archive, ELogSection section)
{
	uint32 totalSectionBytes = (uint32)archive.TotalSize();
	uint8 sectionId = (uint8)section;
	*mArchive << sectionId;
	*mArchive << totalSectionBytes;
	
	WriteAndFlushArchive(*mArchive);
	delete mArchive;
	mArchive = new FBufferArchive();

	if (totalSectionBytes > 0)
	{
		WriteAndFlushArchive(archive);
		delete &archive;
		return new FBufferArchive();
	}
	return &archive;
}

void UBlinkWinkLogger::WriteAndFlushArchive(FBufferArchive& archive)
{
	int64 count = 0;
	for (const auto& item : archive)
	{
		mFileHandle->Write(&item, 1);
		++count;
		if (count == archive.TotalSize())
			break;
	}
	archive.FlushCache();
	archive.Empty();
}

void UBlinkWinkLogger::UserBlinked()
{
	*mBlinkLogData << mTotalDeltaTime;
	mTotalBytes += sizeof(mTotalDeltaTime);
}

void UBlinkWinkLogger::LeftEyeClosed()
{
	uint8 leftEye = 0;
	*mWinkLogData << mTotalDeltaTime;
	*mWinkLogData << leftEye;
	mTotalBytes += sizeof(mTotalDeltaTime);
}

void UBlinkWinkLogger::RightEyeClosed()
{
	uint8 rightEye = 1;
	*mWinkLogData << mTotalDeltaTime;
	*mWinkLogData << rightEye;
	mTotalBytes += sizeof(mTotalDeltaTime);
}

void UBlinkWinkLogger::BothEyesOpen()
{
	*mBothOpenLogData << mTotalDeltaTime;
	mTotalBytes += sizeof(mTotalDeltaTime);
}
