#pragma once
#include <iostream>
#include <curl/curl.h>
#include <fstream>
#include <string>
#include <chrono>
#include <thread>

static class MeasureSpeeds
{
	//Downloading
	static void GetHeadersFromFile(CURL* _cHandle, std::fstream* _InHeader);
	static int DownloadedData;
	static bool FirstDataReceived;
	static std::chrono::system_clock::time_point StartTime;

	//Writing
public:
	//Downloading
	static unsigned int DownloadSpeed(std::string _URL, std::string _InHeaderPath);
	static int ProgressFunction(int* _ptr, double _TotalToDownload, double _NowDownloaded, double _TotalToUpload);
	static size_t WriteDataFunction(void* _buffer, size_t _size, size_t _nmemb, void* _userp);

	//Writing
	static unsigned int WritingSpeed(std::streamsize _testBufferSize); //Tests how long it takes to create 2 files and merge them
};

