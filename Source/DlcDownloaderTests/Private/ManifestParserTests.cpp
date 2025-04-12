#include "JsonObjectConverter.h"
#include "ManifestDto.h"
#include "Interfaces/IPluginManager.h"
#include "Misc/AutomationTest.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(ManifestParserTests, "Private.ManifestParserTests",
                                 EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

namespace
{
	const TArray<FPakFileDto> ExpectedPaks = {
		{
			TEXT("pakchunk0-Windows.pak"), TEXT("/Paks/pakchunk0-Windows.pak"), 0, 994924779,
			TEXT("SHA1:8bbcd90a3e63296492c74ece462ba7433d48b72f")
		},
		{
			TEXT("pakchunk0optional-Windows.pak"), TEXT("/Paks/pakchunk0optional-Windows.pak"), 0, 354929981,
			TEXT("SHA1:4aef58e0c89d5fe0fd117caec5f1644a76195b01")
		},
		{
			TEXT("pakchunk160-Windows.pak"), TEXT("/Paks/pakchunk160-Windows.pak"), 160, 1779,
			TEXT("SHA1:8d313473900320b833265d3843dcab0610eff1bd")
		},
		{
			TEXT("pakchunk161-Windows.pak"), TEXT("/Paks/pakchunk161-Windows.pak"), 161, 1575030,
			TEXT("SHA1:ea118a0e4294907d43ad95157120d549f4c8adc4")
		},
		{
			TEXT("pakchunk161optional-Windows.pak"), TEXT("/Paks/pakchunk161optional-Windows.pak"), 161, 3279736,
			TEXT("SHA1:cef569444b873bb8859773e3b7486e5aa16b9582")
		},
		{
			TEXT("pakchunk162-Windows.pak"), TEXT("/Paks/pakchunk162-Windows.pak"), 162, 1575030,
			TEXT("SHA1:fbee853414177c255f22f4e8ca1a1aeb4428fc92")
		},
		{
			TEXT("pakchunk162optional-Windows.pak"), TEXT("/Paks/pakchunk162optional-Windows.pak"), 162, 3279736,
			TEXT("SHA1:e5fdc5f778e7d491ad35ad7897ae8be2713647f9")
		},
		{
			TEXT("pakchunk163-Windows.pak"), TEXT("/Paks/pakchunk163-Windows.pak"), 163, 85711235,
			TEXT("SHA1:be422791768b2ddd9a875e52a97f2be1bdd33155")
		},
		{
			TEXT("pakchunk23-Windows.pak"), TEXT("/Paks/pakchunk23-Windows.pak"), 23, 2671,
			TEXT("SHA1:1b25968d3d2b426bb85bff43d66ed9d220cff9af")
		},
		{
			TEXT("pakchunk24-Windows.pak"), TEXT("/Paks/pakchunk24-Windows.pak"), 24, 82078499,
			TEXT("SHA1:122d8d988b694422a68740f111fa8fce9dbbc40f")
		},
		{
			TEXT("pakchunk25-Windows.pak"), TEXT("/Paks/pakchunk25-Windows.pak"), 25, 5045565,
			TEXT("SHA1:42a54044717d59c6261b1d49bfe06b1de77fd3c0")
		}
	};

	TArray<FSigFileDto> ExpectedSigs = {
		{
			TEXT("pakchunk0-Windows.sig"), TEXT("/Paks/pakchunk0-Windows.sig"), 61256,
			TEXT("SHA1:0f4618d4249ccde1df5b4a79f54cb09fd341b3c0")
		},
		{
			TEXT("pakchunk0optional-Windows.sig"), TEXT("/Paks/pakchunk0optional-Windows.sig"), 22192,
			TEXT("SHA1:7bbc064ed6fb3cef6af6aaf85e4acfe96eaeeaa9")
		},
		{
			TEXT("pakchunk160-Windows.sig"), TEXT("/Paks/pakchunk160-Windows.sig"), 532,
			TEXT("SHA1:5bade99c3b47134be9d30660635431d38d983dd0")
		},
		{
			TEXT("pakchunk161-Windows.sig"), TEXT("/Paks/pakchunk161-Windows.sig"), 628,
			TEXT("SHA1:ad37a94f2de114246d7df4b2209eb032709b0538")
		},
		{
			TEXT("pakchunk161optional-Windows.sig"), TEXT("/Paks/pakchunk161optional-Windows.sig"), 732,
			TEXT("SHA1:d0e375b7a010efa5c406267f0ec8f54e66c1c42a")
		},
		{
			TEXT("pakchunk162-Windows.sig"), TEXT("/Paks/pakchunk162-Windows.sig"), 628,
			TEXT("SHA1:a6d146bc3feec25f461dd4677b571b77fd6639fc")
		},
		{
			TEXT("pakchunk162optional-Windows.sig"), TEXT("/Paks/pakchunk162optional-Windows.sig"), 732,
			TEXT("SHA1:46d25659c7dee909618336c379cd20a8280062f0")
		},
		{
			TEXT("pakchunk163-Windows.sig"), TEXT("/Paks/pakchunk163-Windows.sig"), 5760,
			TEXT("SHA1:183ce972572b2c60830a5f39523a58ea3808b561")
		},
		{
			TEXT("pakchunk23-Windows.sig"), TEXT("/Paks/pakchunk23-Windows.sig"), 532,
			TEXT("SHA1:7de08edbd8c14ae069382f95a6ada835a6d4d221")
		},
		{
			TEXT("pakchunk24-Windows.sig"), TEXT("/Paks/pakchunk24-Windows.sig"), 5540,
			TEXT("SHA1:1357ce2e8aa17cdc12c1d39b76a110122ccfacfc")
		},
		{
			TEXT("pakchunk25-Windows.sig"), TEXT("/Paks/pakchunk25-Windows.sig"), 836,
			TEXT("SHA1:4b51095df49f32b808a56072c9620f055ca8f538")
		}
	};
}

bool ManifestParserTests::RunTest(const FString& Parameters)
{
	FString ManifestDataString;
	const FString PluginDir = IPluginManager::Get().FindPlugin(TEXT("DlcDownloader"))->GetBaseDir();
	const FString ResourcesDir = FPaths::Combine(PluginDir, TEXT("Resources"));
	const FString ManifestFile = FPaths::Combine(ResourcesDir, TEXT("TestData"), TEXT("Test_Manifest.json"));
	AddInfo("Loading Manifest...");
	if (!FPaths::FileExists(ManifestFile) || !FFileHelper::LoadFileToString(ManifestDataString, *ManifestFile))
	{
		AddError("Failed to load Manifest file");
		return false;
	}

	FManifestDto Manifest;
	const bool bParseSuccessful = FJsonObjectConverter::JsonObjectStringToUStruct(
		ManifestDataString, &Manifest, 0, 0, true);

	AddInfo("Test general structure...");
	TestTrue("Manifest data should be successfully parsed", bParseSuccessful);
	TestEqual("File version must be 1", Manifest.FileVersion, 1);
	TestEqual("Manifest version must be 1", Manifest.ManifestVersion, 1);
	TestEqual("Number of pak entries must be equal to 11", Manifest.PakFiles.Num(), 11);
	TestEqual("Number of signing entries must be equal to 11", Manifest.SigFiles.Num(), 11);

	AddInfo("Test paks structure...");
	for (int i = 0; i < Manifest.PakFiles.Num(); ++i)
	{
		const FPakFileDto ExpectedPak = ExpectedPaks[i];
		const FPakFileDto ActualPak = Manifest.PakFiles[i];
		TestEqual("Local path must be same", ActualPak.LocalPath, ExpectedPak.LocalPath);
		TestEqual("Remote path must be same", ActualPak.RemotePath, ExpectedPak.RemotePath);
		TestEqual("Chunk ID must be same", ActualPak.ChunkId, ExpectedPak.ChunkId);
		TestEqual("File size must be same", ActualPak.FileSize, ExpectedPak.FileSize);
		TestEqual("File hash must be same", ActualPak.FileHash, ExpectedPak.FileHash);
	}

	AddInfo("Test signings structure...");
	for (int i = 0; i < Manifest.SigFiles.Num(); ++i)
	{
		const FSigFileDto ExpectedSig = ExpectedSigs[i];
		const FSigFileDto ActualSig = Manifest.SigFiles[i];
		TestEqual("Local path must be same", ActualSig.LocalPath, ExpectedSig.LocalPath);
		TestEqual("Remote path must be same", ActualSig.RemotePath, ExpectedSig.RemotePath);
		TestEqual("File size must be same", ActualSig.FileSize, ExpectedSig.FileSize);
		TestEqual("File hash must be same", ActualSig.FileHash, ExpectedSig.FileHash);
	}
	
	return true;
}
