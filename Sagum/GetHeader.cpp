#pragma once
#define CURL_STATICLIB
#include "GetHeader.h"
#include "ReportError.h"

//Not used 
std::string GetHeader::GetStringAfterFirstSpace(std::string str)
{
	size_t i = 0;
	for (;i < str.size();i++) 
	{
		if (str[i] == ' ') break;
		else if (i + 1 >= str.size())//cannot find space
		{
			return "";
		}
	}
	//test napis
	//^^^^^    
	str.erase(0,i+1);
	return str;
}

void GetHeader::GetHeadersFromFile(CURL* _cHandle, std::fstream* _InHeader)
{
	std::string line;
	struct curl_slist* chunk = NULL;
	while (!(*_InHeader).eof())
	{
		getline(*_InHeader, line);
		chunk = curl_slist_append(chunk, line.c_str());
	}
	curl_easy_setopt(_cHandle, CURLOPT_HTTPHEADER, chunk);
}
size_t GetHeader::ResponseHeader(char* _buffer, size_t _size, size_t _nitems, void* _userdata)
{
	OutHeaders.push_back(_buffer);
	return _nitems * _size;
}
std::vector<std::string> GetHeader::GetOutHeaders(std::string _URL, std::string _InHeaderPath)
{
	CURL* curl = curl_easy_init();
	std::fstream HeadersFile;
	HeadersFile.open(_InHeaderPath.c_str(), std::ios::in | std::ios::binary);

	if (curl&&HeadersFile.good()) {
		curl_easy_setopt(curl, CURLOPT_URL, _URL.c_str());
		GetHeadersFromFile(curl, &HeadersFile);
		HeadersFile.close();
		curl_easy_setopt(curl, CURLOPT_HEADERFUNCTION, GetHeader::ResponseHeader);
		curl_easy_setopt(curl, CURLOPT_BUFFERSIZE, 102400L);
		curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 1L);
		//curl_easy_setopt(curl, CURLOPT_USERAGENT, "curl/7.79.1");
		curl_easy_setopt(curl, CURLOPT_MAXREDIRS, 50L);
		//curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "GET");
		curl_easy_setopt(curl, CURLOPT_FTP_SKIP_PASV_IP, 1L);
		curl_easy_setopt(curl, CURLOPT_TCP_KEEPALIVE, 1L);
		//curl_easy_setopt(curl, CURLOPT_FILETIME, 1L);
		//curl_easy_setopt(curl, CURLOPT_MAXFILESIZE, 1);//Use this instead of CURLOPT_NOBODY
		curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, false);//certificate fix
		curl_easy_setopt(curl, CURLOPT_NOBODY, 1);
		curl_easy_perform(curl);
		int RetrySec = 10;
		for (int i = 0;i < RetrySec;i++)
		{
			std::this_thread::sleep_for(std::chrono::seconds(1));
			if (!OutHeaders.empty())
			{
				return OutHeaders;
			}
		}
	}
	else if (!curl) {
		ReportError::Error("Problem with curl");
	}
	else
	{
		ReportError::Error("Cannot open header file");
	}
	
	ReportError::Warning("Did not receive respose headers");
	return OutHeaders;
}
unsigned long long GetHeader::GetContentLength()
{
	if (GetHeader::OutHeaders.empty()) ReportError::Warning("Atempting to get content length without any response headers");
	if (GetHeader::FileSize != 0) return GetHeader::FileSize;

	for (size_t i = 0; i < GetHeader::OutHeaders.size();i++)
	{
		std::string phrase = GetHeader::OutHeaders[i];
		//lower case
		for (size_t j = 0; j < phrase.size(); j++)
		{
			if ((int)phrase[j] >= 65 && (int)phrase[j] <= 90)
				phrase[j] = (char)(phrase[j] + 32);
		}
		if (phrase.find("content-length") != std::string::npos) {
			
			size_t j = 0;
			for (; j < phrase.length(); j++) { if (isdigit(phrase[j])) break; }
			phrase = phrase.substr(j, phrase.length() - j);
			unsigned long long result = std::stoull(phrase, nullptr, 0);
			GetHeader::FileSize = result;
			return result;
		}
	}
	return 0;
}
GetHeader::FileType GetHeader::RecognizeType()
{
	if (GetHeader::OutType != GetHeader::FileType::UNDEFINED) return GetHeader::OutType;

	GetHeader::OutType = GetHeader::FileType::UNKNOWN;
	std::string result = "";
	for (size_t i = 0; i < GetHeader::OutHeaders.size();i++)
	{
		std::string phrase = GetHeader::OutHeaders[i];
		//lower case
		for (size_t j = 0; j < phrase.size(); j++)
		{
			if ((int)phrase[j] >= 65 && (int)phrase[j] <= 90)
				phrase[j] = (char)(phrase[j] + 32);
		}
		if (phrase.find("content-type") != std::string::npos) {
			result = phrase;
			break;
		}
	}
	if(result.find("video/mp4") != std::string::npos) GetHeader::OutType = GetHeader::FileType::video_MP4;
	else if (result.find("video/mp2t") != std::string::npos) GetHeader::OutType = GetHeader::FileType::video_MP2T;
	if(GetHeader::OutType== GetHeader::FileType::UNKNOWN) ReportError::Warning("Unknown filetype: " + result);
	return GetHeader::OutType;
}
void GetHeader::Reset()
{
	GetHeader::OutHeaders.clear();
	GetHeader::OutType = GetHeader::FileType::UNDEFINED;
	GetHeader::FileSize = 0;
}

std::vector<std::string> GetHeader::OutHeaders;
GetHeader::FileType GetHeader::OutType = GetHeader::FileType::UNDEFINED;
unsigned long long GetHeader::FileSize = 0;