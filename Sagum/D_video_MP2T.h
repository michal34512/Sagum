#pragma once
#include <curl/curl.h>
#include "DownloadFile.h"
#include "MeasureSpeeds.h"
#include "GetHeader.h"
#include "ReportError.h"
#include "ProgressBar.h"
#include "Merger.h"
#include "FFMpeg.h"
#include <fstream>

//1# URL example https://s98.upstreamcdn.co/hls/w47ryvlixfbnx2nroykk6bnhiz3ryjotsxd6cphdv2iromxcempgsxv2ibxq/seg-1-v1-a1.ts
//2# URL example https://s98.upstreamcdn.co/hls/w47ryvlixfbnx2nroykk6bnhiz3ryjotsxd6cphdv2iromxcempgsxv2ibxq/seg-{}-v1-a1.ts

class  D_video_MP2T : public DownloadFile
{
	std::string URL;
	std::string InHeaderPath;
	std::string OUTPUT;
	std::string COOKIEFILE;

	int LastIndexInQueue = 0;
	unsigned int segCount = 0;
	CURLM* MultiManager;
	CURLMsg* MultiMessages;
	int MultiStillAlive;
	int MuliMessagesLeft;
	byte NewTransfer(std::string _URL, std::string _InHeader, int _IndexInArray, int _IndexInQueue);
	void GetHeadersFromFile(CURL* _cHandle, std::string _InHeader);
	std::string GetPatchByIndexInQueue(int _IndexInQueue);
	static size_t GetSegCountURLWriteFunc(char* ptr, size_t size, size_t nmemb, unsigned int* out);
	void GetSegCount(unsigned int* segCount);
	virtual float UpdateProgress() override;
public:
	int Rate = 20;
	std::string PREFIX = "";

	class SinglePart
	{
	public:
		CURL* Handle;
		int IndexInQueue = -1;
		std::fstream OutFile;
		unsigned long long AlreadyDownloaded = 0;
	};
	SinglePart* PartsArray;

	virtual void Start(std::string _URL, std::string _InHeaderPath, std::string _CookieFile, std::string _OUPUT, std::string _PREFIX, bool _Verbose) override;
	virtual bool Update() override;

	static size_t WriteData(char* ptr, size_t size, size_t nmemb, void* outfile);
	static int ProgressFunction(void* ptr, double TotalToDownload, double NowDownloaded, double TotalToUpload, double NowUploaded);
};