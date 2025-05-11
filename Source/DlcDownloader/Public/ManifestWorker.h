#pragma once
#include "ManifestDto.h"
#include "Interfaces/IHttpRequest.h"

DECLARE_MULTICAST_DELEGATE_ThreeParams(FCheckForUpdatesDelegate, bool /*bSuccessful*/, FString /*ErrorMessage*/,
                                       bool /*bUpdateRequired*/);

DECLARE_DELEGATE_TwoParams(FReadRemoteManifestDelegate, bool /*bSuccessful*/, const FManifestDto& /*RemoteManifest*/);

class FManifestWorker : public TSharedFromThis<FManifestWorker>
{
public:
	FManifestWorker(const FString& InDeploymentId, const FString& InManifestUrl);

	FString GetDeploymentId() const;
	void CheckForUpdates();

private:
	void ReadRemoteManifest(const TSharedRef<FReadRemoteManifestDelegate>& Callback);

public:
	FCheckForUpdatesDelegate OnCheckForUpdatesCompleted;

private:
	FString DeploymentId;
	FString RemoteManifestUrl;
	FString CachedManifestPath;
	bool bHasCachedManifest = false;
	bool bUpdateRequired = false;
	FManifestDto RemoteManifest = FManifestDto();
	FHttpRequestPtr CheckForUpdatesRequest;
};
