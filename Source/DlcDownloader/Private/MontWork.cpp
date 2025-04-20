#include "MontWork.h"

#include "ManifestDto.h"

FMontWork::FMontWork(const FString& InCachedPakDirectory)
{
	CachedPakDirectory = InCachedPakDirectory;
}

bool FMontWork::MountPaks(TArray<FDDPakFileEntry>& PakEntries) const
{
	const uint32 PakReadOrder = PakEntries.Num();
	bool bAllPaksMounted = true;
	for (int i = 0; i < PakEntries.Num(); ++i)
	{
		if (PakEntries[i].bMounted)
		{
			continue;
		}
		const FString MountDir = CachedPakDirectory / PakEntries[i].PakFile.LocalPath;
		const IPakFile* MountedPak = FCoreDelegates::MountPak.Execute(MountDir, PakReadOrder);
		if (MountedPak == nullptr)
		{
			bAllPaksMounted = false;
		}
		PakEntries[i].bMounted = MountedPak != nullptr;
	}
	return bAllPaksMounted;
}
