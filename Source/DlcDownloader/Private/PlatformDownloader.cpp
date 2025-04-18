#include "PlatformDownloader.h"

#include "HttpModule.h"
#include "LogDeclarations.h"
#include "Interfaces/IHttpRequest.h"
#include "Interfaces/IHttpResponse.h"

FPlatformDownloader::FPlatformDownloader(const FString& InFileUrl, const FString& InTargetFile)
{
	FileUrl = InFileUrl;
	TargetFile = InTargetFile;
}

FDownloadCompleteDelegate& FPlatformDownloader::OnDownloadComplete()
{
	return OnDownloadCompleteDelegate;
}

FDownloadProgressDelegate& FPlatformDownloader::OnProgressUpdated()
{
	return OnProgressUpdatedDelegate;
}

bool FPlatformDownloader::StartDownloading()
{
	if (Request)
	{
		Request->OnProcessRequestComplete().Unbind();
		Request->OnRequestProgress().Unbind();
		Request->CancelRequest();
	}
	FHttpModule& HttpModule = FModuleManager::LoadModuleChecked<FHttpModule>("HTTP");
	Request = HttpModule.Get().CreateRequest();
	Request->SetURL(FileUrl);
	Request->SetVerb(TEXT("GET"));
	Request->OnRequestProgress().BindRaw(this, &FPlatformDownloader::HandleProgressReceived);
	Request->OnProcessRequestComplete().BindRaw(this, &FPlatformDownloader::HandleRequestComplete);
	return Request->ProcessRequest();
}

void FPlatformDownloader::CancelDownloading()
{
	if (Request && Request->GetStatus() == EHttpRequestStatus::Processing)
	{
		Request->OnProcessRequestComplete().Unbind();
		Request->OnRequestProgress().Unbind();
		Request->CancelRequest();
		OnDownloadCompleteDelegate.ExecuteIfBound(EDownloadStatus::Canceled);
	}
}

void FPlatformDownloader::HandleProgressReceived(FHttpRequestPtr HttpRequest, int32 BytesSent,
                                                 int32 BytesReceived) const
{
	OnProgressUpdatedDelegate.ExecuteIfBound(BytesReceived);
}

void FPlatformDownloader::HandleRequestComplete(FHttpRequestPtr HttpRequest, FHttpResponsePtr HttpResponse,
                                                bool bConnectedSuccessfully)
{
	// Explicitly initialize status just to ensure we return expected status by default
	EDownloadStatus DownloadStatus = EDownloadStatus::Failed;
	if (HttpResponse.IsValid())
	{
		const int32 HttpStatusCode = HttpResponse->GetResponseCode();
		if (EHttpResponseCodes::IsOk(HttpStatusCode))
		{
			const TArray<uint8>& Content = HttpResponse->GetContent();
			if (SaveBinaryDataToFile(Content))
			{
				UE_LOG(LogDlcDownloaderPlugin, Verbose,
				       TEXT("[PlatformDownloader] Successfully saved data to file '%s'"), *TargetFile);
				DownloadStatus = EDownloadStatus::Completed;
			}
			else
			{
				UE_LOG(LogDlcDownloaderPlugin, Error,
				       TEXT("[PlatformDownloader] Failed to save binary data to file '%s'"), *TargetFile);
				DeleteFileIfExists();
				DownloadStatus = EDownloadStatus::Failed;
			}
		}
		else
		{
			UE_LOG(LogDlcDownloaderPlugin, Error, TEXT("[PlatformDownloader] HTTP %d returned from '%s'"),
			       HttpStatusCode, *HttpRequest->GetURL());		
			DownloadStatus = EDownloadStatus::Failed;
		}
	}
	else
	{
		UE_LOG(LogDlcDownloaderPlugin, Error, TEXT("[PlatformDownloader] HTTP connection issue downloading '%s'"),
		       *HttpRequest->GetURL());
		DownloadStatus = EDownloadStatus::Failed;
	}
	UE_LOG(LogDlcDownloaderPlugin, Verbose, TEXT("[PlatformDownloader] Download pak file completed"));
	OnDownloadCompleteDelegate.ExecuteIfBound(DownloadStatus);
}

bool FPlatformDownloader::SaveBinaryDataToFile(const TArray<uint8>& InBinaryData) const
{
	bool bSuccess = false;
	IFileHandle* ManifestFile = IPlatformFile::GetPlatformPhysical().OpenWrite(*TargetFile, false);
	if (ManifestFile != nullptr)
	{
		bSuccess = ManifestFile->Write(InBinaryData.GetData(), InBinaryData.Num());
		delete ManifestFile;
	}
	return bSuccess;
}

bool FPlatformDownloader::DeleteFileIfExists() const
{
	bool bSuccess = false;
	IPlatformFile& PlatformFile = IPlatformFile::GetPlatformPhysical();
	if (PlatformFile.FileExists(*TargetFile))
	{
		bSuccess = PlatformFile.DeleteFile(*TargetFile);
	}
	return bSuccess;
}
