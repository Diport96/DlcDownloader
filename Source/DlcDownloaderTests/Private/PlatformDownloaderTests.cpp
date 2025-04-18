#include "PlatformDownloader.h"
#include "Misc/AutomationTest.h"

namespace
{
	// Initialize these variables with your data
	inline const FString PakFileUrl;
	inline const FString PakFilePath;
}

DEFINE_LATENT_AUTOMATION_COMMAND_TWO_PARAMETER(FWaitForSecondsCommand, TFunction<void()>, InCallback, float,
                                               InTimeSeconds);

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FCanDownloadFileTest, "Private.PlatformDownloaderTests.CanDownloadFile",
                                 EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FCanCancelDownloadFileTest, "Private.PlatformDownloaderTests.CanCancelDownloadFile",
                                 EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FWaitForSecondsCommand::Update()
{
	const double NewTime = FPlatformTime::Seconds();
	if (NewTime - StartTime >= static_cast<double>(InTimeSeconds))
	{
		InCallback();
		return true;
	}
	return false;
}


bool FCanDownloadFileTest::RunTest(const FString& Parameters)
{
	TSharedPtr<TAtomic<bool>> ResultDataCompleted = MakeShared<TAtomic<bool>>(false);
	TSharedPtr<FPlatformDownloader> Downloader = MakeShared<FPlatformDownloader>(PakFileUrl, PakFilePath);
	Downloader->OnDownloadComplete().BindLambda([this, ResultDataCompleted](const EDownloadStatus DownloadStatus)
	{
		AddInfo("Download completed");
		TestEqual("Should successfully download pak file", DownloadStatus, EDownloadStatus::Completed);
		ResultDataCompleted->Store(true);
	});

	const bool bRequestStarted = Downloader->StartDownloading();
	TestTrue("Request should start successfully", bRequestStarted);
	if (bRequestStarted)
	{
		ADD_LATENT_AUTOMATION_COMMAND(FUntilCommand([this, Downloader, ResultDataCompleted]()
			{
			return ResultDataCompleted->Load();
			},
			[this]()
			{
			AddError("Download timeout has expired");
			return true;
			},
			60.0f
		));
	}

	return true;
}

bool FCanCancelDownloadFileTest::RunTest(const FString& Parameters)
{
	TSharedPtr<TAtomic<bool>> ResultDataCompleted = MakeShared<TAtomic<bool>>(false);
	TSharedPtr<FPlatformDownloader> Downloader = MakeShared<FPlatformDownloader>(PakFileUrl, PakFilePath);
	Downloader->OnDownloadComplete().BindLambda([this, ResultDataCompleted](const EDownloadStatus DownloadStatus)
	{
		AddInfo("Download completed");
		TestEqual("Download pak file should be cancelled", DownloadStatus, EDownloadStatus::Canceled);
		ResultDataCompleted->Store(true);
	});

	const bool bRequestStarted = Downloader->StartDownloading();
	TestTrue("Request should start successfully", bRequestStarted);
	if (bRequestStarted)
	{
		constexpr float DelayTime = 2.0f;
		ADD_LATENT_AUTOMATION_COMMAND(FWaitForSecondsCommand([this, Downloader]()
		{
			AddInfo("Canceling download");
			Downloader->CancelDownloading();
		}, DelayTime));

		ADD_LATENT_AUTOMATION_COMMAND(FUntilCommand([this, Downloader, ResultDataCompleted]()
			{
			return ResultDataCompleted->Load();
			},
			[this]()
			{
			AddError("Download timeout has expired");
			return true;
			},
			60.0f
		));
	}

	return true;
}
