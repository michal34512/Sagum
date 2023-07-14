#pragma once
#include <curl/curl.h>
#include <fstream>
#include <filesystem>

#include "DownloadFile.h"
#include "GetHeader.h"
#include "ReportError.h"
#include "ProgressBar.h"
#include "Merger.h"


class  D_video_MP4 : public DownloadFile
{
	//Download parameters
	std::string URL;
	std::string InHeaderPath;
	std::string OUTPUT;
	std::string COOKIEFILE;

	//Curl stuff
	CURLM* MultiManager;
	CURLMsg* MultiMessages;
	int MultiStillAlive;
	int MuliMessagesLeft;

	int LastIndexInQueue = 0;
	void GetHeadersFromFileWithRange(CURL* _cHandle, std::string _InHeader, unsigned long long _DownRange, unsigned long long _UpRange);
	std::string GetPatchByIndexInQueue(int _IndexInQueue);
	byte NewTransfer(int _IndexInArray, int _IndexInQueue);
	virtual float UpdateProgress() override;
public:
	//Download parameters
	int Rate = 20;
	unsigned long long SinglePartSize = 10000000; //=10 mb
	std::string PREFIX;

	class SinglePart
	{
	public:
		CURL* Handle;
		int IndexInQueue = -1;
		std::fstream OutFile;
		unsigned long long AlreadyDownloaded = 0;
	};
	SinglePart* PartsArray;
	
	virtual void Start(std::string _URL, std::string _InHeaderPath, std::string _CookieFile, std::string _OUPUT, std::string _PREFIX, bool _Verbose = false) override;
	virtual bool Update() override;

	static size_t WriteData(char* ptr, size_t size, size_t nmemb, void* outfile);
	static int ProgressFunction(void* ptr, double TotalToDownload, double NowDownloaded, double TotalToUpload, double NowUploaded);
};