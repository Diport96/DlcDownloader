#pragma once

enum class EDownloadStatus: uint8
{
	NotStarted,
	Downloading,
	Completed,
	Failed,
	Canceled
};
