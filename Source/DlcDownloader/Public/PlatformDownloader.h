#pragma once
#include "IPlatformDownloader.h"
#include "Interfaces/IHttpRequest.h"

class DLCDOWNLOADER_API FPlatformDownloader: public IPlatformDownloader
{
public:
	FPlatformDownloader(const FString& InFileUrl, const FString& InTargetFile);
	virtual FDownloadCompleteDelegate& OnDownloadComplete() override;
	virtual FDownloadProgressDelegate& OnProgressUpdated() override;	
	virtual bool StartDownloading() override;
	virtual void CancelDownloading() override;
	
protected:
	void HandleProgressReceived(FHttpRequestPtr HttpRequest, int32 BytesSent, int32 BytesReceived) const;
	void HandleRequestComplete(FHttpRequestPtr HttpRequest, FHttpResponsePtr HttpResponse, bool bConnectedSuccessfully);
	
	FDownloadCompleteDelegate OnDownloadCompleteDelegate;
	FDownloadProgressDelegate OnProgressUpdatedDelegate;
	FString FileUrl;
	FString TargetFile;
	TSharedPtr<IHttpRequest> Request;

private:
	bool SaveBinaryDataToFile(const TArray<uint8>& InBinaryData) const;
	bool DeleteFileIfExists() const;
};
