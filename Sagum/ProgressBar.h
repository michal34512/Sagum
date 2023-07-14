#pragma once
#include <thread>
#include <atomic>
#include <chrono>
#include <iostream>
#include <windows.h>
#include "DownloadFile.h"

static class ProgressBar {
	static std::thread th;
	static DownloadFile *segment;
public:
	static bool endTh;

	static void Start(DownloadFile * _segment);
	static void End();
	static void Update();
};
