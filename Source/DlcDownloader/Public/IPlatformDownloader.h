#pragma once
#include "DownloadStatus.h"

DECLARE_DELEGATE_OneParam(FDownloadCompleteDelegate, EDownloadStatus);
DECLARE_DELEGATE_OneParam(FDownloadProgressDelegate, int32 BytesReceived);

class IPlatformDownloader
{
public:
	virtual ~IPlatformDownloader() = default;
	virtual FDownloadCompleteDelegate& OnDownloadComplete() = 0;
	virtual FDownloadProgressDelegate& OnProgressUpdated() = 0;
	virtual bool StartDownloading() = 0;
	virtual void CancelDownloading() = 0;
};
