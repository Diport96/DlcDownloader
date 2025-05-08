#pragma once
#include "ManifestDto.h"

class DLCDOWNLOADER_API FMountWorker
{
public:
	FMountWorker(const FString& InCachedPakDirectory);
	
	bool MountPaks(TArray<FDDPakFileEntry>& PakEntries) const;

	bool UnmountPaks(TArray<FDDPakFileEntry>& PakEntries) const;
	
protected:
	FString CachedPakDirectory;
};
