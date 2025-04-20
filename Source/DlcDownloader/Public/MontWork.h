#pragma once
#include "ManifestDto.h"

class FMontWork
{
public:
	FMontWork(const FString& InCachedPakDirectory);
	
	bool MountPaks(TArray<FDDPakFileEntry>& PakEntries) const;

protected:
	FString CachedPakDirectory;
};
