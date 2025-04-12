// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "ManifestDto.generated.h"


USTRUCT(BlueprintType)
struct FSigFileDto
{
	GENERATED_BODY()

	UPROPERTY()
	FString LocalPath;

	UPROPERTY()
	FString RemotePath;

	UPROPERTY()
	int64 FileSize = 0;

	UPROPERTY()
	FString FileHash;
};

USTRUCT()
struct FPakFileDto
{
	GENERATED_BODY()

	UPROPERTY()
	FString LocalPath;

	UPROPERTY()
	FString RemotePath;

	UPROPERTY()
	int32 ChunkId = INDEX_NONE;

	UPROPERTY()
	int64 FileSize = 0;

	UPROPERTY()
	FString FileHash;
};

USTRUCT()
struct FManifestDto
{
	GENERATED_BODY()

	UPROPERTY()
	int32 FileVersion = 1;

	UPROPERTY()
	int32 ManifestVersion;

	UPROPERTY()
	TArray<FPakFileDto> PakFiles;
	
	UPROPERTY()
	TArray<FSigFileDto> SigFiles;
};
