#include "ManifestWorker.h"

#include "HttpModule.h"
#include "JsonObjectConverter.h"
#include "LogDeclarations.h"
#include "ManifestDto.h"
#include "Interfaces/IHttpResponse.h"

FManifestWorker::FManifestWorker(const FString& InDeploymentId, const FString& InManifestUrl)
{
	this->DeploymentId = InDeploymentId;
	this->RemoteManifestUrl = InManifestUrl;
	this->CachedManifestPath = FPaths::ProjectPersistentDownloadDir() / TEXT("PakCache/");
}

FString FManifestWorker::GetDeploymentId() const
{
	return DeploymentId;
}

void FManifestWorker::CheckForUpdates()
{
	const TSharedPtr<FReadRemoteManifestDelegate> Callback = MakeShared<FReadRemoteManifestDelegate>();
	Callback->BindSPLambda(this, [this](const bool bSuccessful, const FManifestDto& InRemoteManifest)
	{
		if (bSuccessful == false)
		{
			OnCheckForUpdatesCompleted.Broadcast(false, TEXT("Unable to read remote manifest data"), false);
			return;
		}

		RemoteManifest = InRemoteManifest;
		// Read local manifest data
		const FString FullManifestPath = FPaths::Combine(CachedManifestPath, DeploymentId + TEXT(".json"));
		bHasCachedManifest = FPaths::FileExists(FullManifestPath);
		FString JsonString = FString();
		if (bHasCachedManifest == false)
		{
			bUpdateRequired = true;
			OnCheckForUpdatesCompleted.Broadcast(true, FString(), bUpdateRequired);
			return;
		}
		if (FFileHelper::LoadFileToString(JsonString, *FullManifestPath) == false)
		{
			bUpdateRequired = true;
			OnCheckForUpdatesCompleted.Broadcast(false, TEXT("Unable to read cached manifest data"), bUpdateRequired);
			return;
		}

		FManifestDto LocalManifest;
		if (FJsonObjectConverter::JsonObjectStringToUStruct(JsonString, &LocalManifest, 0, 0, true) == false)
		{
			bUpdateRequired = true;
			OnCheckForUpdatesCompleted.Broadcast(false, TEXT("Unable to deserialize cached manifest data"),
			                                     bUpdateRequired);
			return;
		}
		bUpdateRequired = LocalManifest.FileVersion != InRemoteManifest.FileVersion;
		OnCheckForUpdatesCompleted.Broadcast(true, FString(), bUpdateRequired);
	});
	ReadRemoteManifest(Callback.ToSharedRef());
}

void FManifestWorker::ReadRemoteManifest(const TSharedRef<FReadRemoteManifestDelegate>& Callback)
{
	// If it has another request pending then we should cancel it. Unbind OnProcessRequestComplete event
	// because we wouldn't receive this event after cancel this request
	if (CheckForUpdatesRequest && CheckForUpdatesRequest->GetStatus() == EHttpRequestStatus::Processing)
	{
		CheckForUpdatesRequest->OnProcessRequestComplete().Unbind();
		CheckForUpdatesRequest->CancelRequest();
	}

	FHttpModule& HttpModule = FHttpModule::Get();
	CheckForUpdatesRequest = HttpModule.CreateRequest();
	CheckForUpdatesRequest->SetURL(RemoteManifestUrl);
	CheckForUpdatesRequest->SetVerb("GET");
	CheckForUpdatesRequest->OnProcessRequestComplete().BindLambda(
		[Callback](FHttpRequestPtr Request, FHttpResponsePtr Response, bool bConnectedSuccessfully)
		{
			FManifestDto ManifestDto;
			if (bConnectedSuccessfully == false || Response == nullptr)
			{
				UE_LOG(LogDlcDownloaderPlugin, Error, TEXT("[Manifest] Failed to read remote manifest data:"
					       " failed to process request"));
				Callback->ExecuteIfBound(false, ManifestDto);
				return;
			}
			const int32 ResponseCode = Response->GetResponseCode();
			if (EHttpResponseCodes::IsOk(ResponseCode) == false)
			{
				UE_LOG(LogDlcDownloaderPlugin, Error, TEXT("[Manifest] Failed to read remote manifest data:"
					       " response returned an error"));
				Callback->ExecuteIfBound(false, ManifestDto);
				return;
			}
			FString ResponseString = Response->GetContentAsString();

			const bool bConverted = FJsonObjectConverter::JsonObjectStringToUStruct(
				ResponseString, &ManifestDto, 0, 0, true);
			if (bConverted == false)
			{
				UE_LOG(LogDlcDownloaderPlugin, Error, TEXT("[Manifest] Failed to read remote manifest data:"
					       " failed to convert JSON data to USTRUCT"));
				Callback->ExecuteIfBound(false, ManifestDto);
				return;
			}
			Callback->ExecuteIfBound(true, ManifestDto);
		});
	CheckForUpdatesRequest->ProcessRequest();
}
