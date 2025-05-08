#include "MountWorker.h"

#include "LogDeclarations.h"
#include "ManifestDto.h"

FMountWorker::FMountWorker(const FString& InCachedPakDirectory)
{
	CachedPakDirectory = InCachedPakDirectory;
}

bool FMountWorker::MountPaks(TArray<FDDPakFileEntry>& PakEntries) const
{
	IFileManager& FileManager = IFileManager::Get();
	if (CachedPakDirectory.IsEmpty() || FileManager.DirectoryExists(*CachedPakDirectory) == false)
	{
		UE_LOG(LogDlcDownloaderPlugin, Error, TEXT("[Mount] Failed to mount pak files: the cached pak "
			       "directory '%s' doesn't exists"), *CachedPakDirectory);
		return false;
	}

	const uint32 PakReadOrder = PakEntries.Num();
	bool bAllPaksMounted = true;
	for (int i = 0; i < PakEntries.Num(); ++i)
	{
		if (PakEntries[i].bMounted)
		{
			continue;
		}
		const FString PakFilePath = CachedPakDirectory / PakEntries[i].PakFile.LocalPath;
		if (FileManager.FileExists(*PakFilePath) == false)
		{
			UE_LOG(LogDlcDownloaderPlugin, Error, TEXT("[Mount] Failed to mount pak file: the cached pak"
											  " file '%s' doesn't exists"), *PakFilePath)
		}
		const IPakFile* MountedPak = FCoreDelegates::MountPak.Execute(PakFilePath, PakReadOrder);
		if (MountedPak == nullptr)
		{
			bAllPaksMounted = false;
		}
		PakEntries[i].bMounted = MountedPak != nullptr;
	}
	return bAllPaksMounted;
}

bool FMountWorker::UnmountPaks(TArray<FDDPakFileEntry>& PakEntries) const
{
	IFileManager& FileManager = IFileManager::Get();
	if (CachedPakDirectory.IsEmpty() || FileManager.DirectoryExists(*CachedPakDirectory) == false)
	{
		UE_LOG(LogDlcDownloaderPlugin, Error, TEXT("[Mount] Failed to unmount pak files: the cached pak "
				   "directory '%s' doesn't exists"), *CachedPakDirectory);
		return false;
	}
	
	bool bAllPaksUnmounted = true;
	for (int i = 0; i < PakEntries.Num(); ++i)
	{
		if (PakEntries[i].bMounted == false)
		{
			continue;
		}
		const FString PakFilePath = CachedPakDirectory / PakEntries[i].PakFile.LocalPath;
		if (FileManager.FileExists(*PakFilePath) == false)
		{
			UE_LOG(LogDlcDownloaderPlugin, Error, TEXT("[Mount] Failed to unmount pak file: the cached pak"
											  " file '%s' doesn't exists"), *PakFilePath)
		}
		const bool bUnmounted = FCoreDelegates::OnUnmountPak.Execute(PakFilePath);
		if (bUnmounted == false)
		{
			bAllPaksUnmounted = false;
		}
		PakEntries[i].bMounted = bUnmounted == false;
	}
	return bAllPaksUnmounted;
}
