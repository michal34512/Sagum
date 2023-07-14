#pragma once
#include <string>
#include <vector>
#include <curl/curl.h>
#include <fstream>
#include <chrono>
#include <thread>

static class GetHeader
{
	static std::string GetStringAfterFirstSpace(std::string str);
public:
	static enum class FileType
	{
		UNDEFINED,
		UNKNOWN,
		video_MP4,
		video_MP2T
	};
	static std::vector<std::string> OutHeaders;
	static FileType OutType;
	static unsigned long long FileSize;

	static std::vector<std::string> GetOutHeaders(std::string _URL, std::string _InHeaderPath);
	static size_t ResponseHeader(char* _buffer, size_t _size, size_t _nitems, void* _userdata);
	static void GetHeadersFromFile(CURL* _cHandle, std::fstream* _InHeader);
	static FileType RecognizeType();
	static unsigned long long GetContentLength();
	static void Reset();
};
