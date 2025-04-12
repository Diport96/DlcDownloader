#include "JsonObjectConverter.h"
#include "ManifestDto.h"
#include "Interfaces/IPluginManager.h"
#include "Misc/AutomationTest.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(ManifestParserTests, "Private.ManifestParserTests",
                                 EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

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

	TestTrue("Manifest data should be successfully parsed", bParseSuccessful);
	TestEqual("File version must be 1", Manifest.FileVersion, 1);
	TestEqual("Manifest version must be 1", Manifest.ManifestVersion, 1);
	TestEqual("Number of pak entries must be equal to 11", Manifest.PakFiles.Num(), 11);
	TestEqual("Number of signing entries must be equal to 11", Manifest.SigFiles.Num(), 11);
	
	return true;
}
