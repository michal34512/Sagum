#pragma once
#define CURL_STATICLIB
#include "D_video_MP2T.h"

size_t D_video_MP2T::WriteData(char* ptr, size_t size, size_t nmemb, void* outfile)
{
	std::fstream* out = static_cast<std::fstream*>(outfile);
	size_t nbytes = size * nmemb;
	out->write(ptr, nbytes);
	return nbytes;
}

int D_video_MP2T::ProgressFunction(void* ptr, double TotalToDownload, double NowDownloaded, double TotalToUpload, double NowUploaded)
{
	D_video_MP2T::SinglePart* out = static_cast<D_video_MP2T::SinglePart*>(ptr);
	out->AlreadyDownloaded = (unsigned long long)NowDownloaded;
	return 0;
}


std::string D_video_MP2T::GetPatchByIndexInQueue(int _IndexInQueue)
{
	for (size_t i = OUTPUT.size() - 1; i + 1 > 0; i--)
		if (OUTPUT[i] == '.')
			return PREFIX + OUTPUT.substr(0, i) + std::to_string(_IndexInQueue) + OUTPUT.substr(i, i - OUTPUT.size());
	return PREFIX + OUTPUT + std::to_string(_IndexInQueue);
}
byte D_video_MP2T::NewTransfer(std::string _URL, std::string _InHeader, int _IndexInArray, int _IndexInQueue)
{
	//Checking for end
	if (segCount != 0 && segCount <= _IndexInQueue) return 2; // END

	CURL* handle;
	handle = curl_easy_init();
	PartsArray[_IndexInArray].Handle = handle;
	PartsArray[_IndexInArray].IndexInQueue = _IndexInQueue;
	if (!handle)
	{
		ReportError::Error("Problem with curl");
		return -1;
	}
	//curl_easy_setopt(handle, CURLOPT_NOPROGRESS, TRUE);
	curl_easy_setopt(handle, CURLOPT_NOPROGRESS, FALSE); //Disable inter progress meter
	curl_easy_setopt(handle, CURLOPT_PROGRESSFUNCTION, ProgressFunction); //Enable custom progress meter
	curl_easy_setopt(handle, CURLOPT_PROGRESSDATA, &PartsArray[_IndexInArray]);

	curl_easy_setopt(handle, CURLOPT_WRITEFUNCTION, WriteData);//send all data to this function
	curl_easy_setopt(handle, CURLOPT_SSL_VERIFYPEER, false);//certificate fix
	curl_easy_setopt(handle, CURLOPT_USERAGENT, "curl/7.79.1");
	curl_easy_setopt(handle, CURLOPT_MAXREDIRS, 50L);
	curl_easy_setopt(handle, CURLOPT_CUSTOMREQUEST, "GET");
	curl_easy_setopt(handle, CURLOPT_FTP_SKIP_PASV_IP, 1L);
	curl_easy_setopt(handle, CURLOPT_TCP_KEEPALIVE, 1L);
	if (COOKIEFILE.size() > 0)
		curl_easy_setopt(handle, CURLOPT_COOKIEFILE, COOKIEFILE.c_str());
	GetHeadersFromFile(handle, _InHeader);
	//Preparing URL
	if (_URL.rfind("{}") == std::string::npos) {
		ReportError::Error("Invalid URL");
		return -1;
	}
	_URL.replace(_URL.rfind("{}"), 2, std::to_string(_IndexInQueue));
	curl_easy_setopt(handle, CURLOPT_URL, _URL.c_str());
	PartsArray[_IndexInArray].OutFile.open((GetPatchByIndexInQueue(_IndexInQueue)).c_str(), std::ios::out | std::ios::binary);//open the output file
	if (PartsArray[_IndexInArray].OutFile.good())
	{
		curl_easy_setopt(handle, CURLOPT_WRITEDATA, &PartsArray[_IndexInArray].OutFile);
		curl_multi_add_handle(MultiManager, handle);
		return 1;
	}
	else {
		ReportError::Error("Problem with output file");
	}
	ReportError::Error("Creating new tranfer failed");
	return -1; //ERROR
}
void D_video_MP2T::GetHeadersFromFile(CURL* _cHandle, std::string _InHeader)
{
	std::string line;
	std::fstream InHeaderFile;
	InHeaderFile.open(_InHeader.c_str(), std::ios::in | std::ios::binary);

	struct curl_slist* chunk = NULL;
	while (!InHeaderFile.eof())
	{
		getline(InHeaderFile, line);
		chunk = curl_slist_append(chunk, line.c_str());
	}
	curl_easy_setopt(_cHandle, CURLOPT_HTTPHEADER, chunk);
}
size_t D_video_MP2T::GetSegCountURLWriteFunc(char* ptr, size_t size, size_t nmemb, unsigned int* out)
{
	std::string data = ptr;
	if (data.rfind("#EXT-X-ENDLIST") != std::string::npos) {
		if (data.rfind("\x0a") != std::string::npos)
		{
			size_t i = data.rfind(",");
			for ( ;i < data.length(); i++)
			{
				if (data[i-1] == '-') break;
			}
			if (i == data.length() - 1) return size* nmemb;
			size_t j = 0;
			for (; j + i  < data.length(); j++)
			{
				if (data[j+i] == '-') break;
			}
			*out = atoi(data.substr(i, 3).c_str());
		}
	}
	return size * nmemb;
}
void D_video_MP2T::GetSegCount(unsigned int *segCount)
{
	//Preparing URL
	std::string indexURL = URL;
	if (indexURL.rfind("seg-{}") == std::string::npos || indexURL.rfind(".ts") == std::string::npos) return;
	indexURL.replace(indexURL.rfind("seg-{}"), 6, "index");
	indexURL = indexURL.replace(indexURL.rfind(".ts"), 3, ".m3u8");
	//indexURL = "https://s71.upstreamcdn.co/hls2/01/04006/tuwnty3gowt6_n/master.m3u8?t=3c6FpjTcm3pM6FAW0MsP5QSqxZx-ngkXE5IEZ1yIMq4&s=1668868645&e=10800&f=20033502&i=0.0&sp=0";

	CURL* curl = curl_easy_init();
	std::fstream HeadersFile;
	HeadersFile.open(InHeaderPath.c_str(), std::ios::in | std::ios::binary);

	if (curl && HeadersFile.good()) {
		curl_easy_setopt(curl, CURLOPT_URL, indexURL.c_str());
		GetHeadersFromFile(curl, InHeaderPath);
		if (COOKIEFILE.size() > 0)
			curl_easy_setopt(curl, CURLOPT_COOKIEFILE, COOKIEFILE.c_str());
		HeadersFile.close();
		curl_easy_setopt(curl, CURLOPT_BUFFERSIZE, 102400L);
		curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 1L);
		curl_easy_setopt(curl, CURLOPT_USERAGENT, "curl/7.79.1");
		curl_easy_setopt(curl, CURLOPT_MAXREDIRS, 50L);
		curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "GET");
		curl_easy_setopt(curl, CURLOPT_FTP_SKIP_PASV_IP, 1L);
		curl_easy_setopt(curl, CURLOPT_TCP_KEEPALIVE, 1L);
		curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, false);//certificate fix
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, GetSegCountURLWriteFunc);
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, segCount);
		curl_easy_perform(curl);
		int RetrySec = 3;
		for (int i = 0; i < RetrySec; i++)
		{
			std::this_thread::sleep_for(std::chrono::seconds(1));
			if (*segCount != 0)
			{
				curl_easy_cleanup(curl);
				return;
			}
		}
	}
}

void D_video_MP2T::Start(std::string _URL, std::string _InHeaderPath, std::string _CookieFile, std::string _OUPUT, std::string _PREFIX, bool _Verbose)
{
	URL = _URL;
	InHeaderPath = _InHeaderPath;
	OUTPUT = _OUPUT;
	LastIndexInQueue = 1;
	COOKIEFILE = _CookieFile;
	PREFIX = _PREFIX;

	//Prepare URL   eg:  /seg-1-v1-a1.ts  ->  /seg-{}-v1-a1.ts
	size_t f = URL.rfind("seg-")+4; //finds last occurance
	if (f != std::string::npos)
	{
		size_t j = 0;
		for (;j < URL.size(); j++)
			if (URL[j+f] == '-') break;
		if (j + 1 != URL.size())
		{
			URL.replace(f, j, "{}");
		}
	}
	//Checking if URL is valid
	if (URL.rfind("{}") == std::string::npos) //finds last occurance
	{
		ReportError::Error("URL is not valid!");
		return;
	}
	//Getting segment count
	GetSegCount(&segCount);
	if (segCount != 0)
	{
		std::cout<< "\x1B[96mFile count: \033[0m" << segCount << std::endl;
	}
	
	curl_global_init(CURL_GLOBAL_ALL); //init the curl session
	MultiManager = curl_multi_init(); //init curl multi
	curl_multi_setopt(MultiManager, CURLMOPT_MAXCONNECTS, (long)Rate); //setting max rate

	PartsArray = new SinglePart[Rate];
	for (int i = 0; i < Rate; i++)
	{
		NewTransfer(URL, InHeaderPath, i, LastIndexInQueue);
		LastIndexInQueue++;
	}
	if (segCount != 0)
	{
		ProgressBar::Start(this);
	}else ReportError::Warning("Progress bar disabled due to lack of file count! Make sure that \"accept - encoding: ...\" is removed from your header file.");
	Merger::Start(OUTPUT, 1);
}
bool D_video_MP2T::Update()
{
	curl_multi_perform(MultiManager, &MultiStillAlive);
	while ((MultiMessages = curl_multi_info_read(MultiManager, &MuliMessagesLeft)))
	{
		if (MultiMessages->msg == CURLMSG_DONE)
		{
			CURL* e = MultiMessages->easy_handle;

			//Getting index
			bool FoundIndex = false;
			int index = 0;
			for (; index < Rate; index++)
				if (PartsArray[index].Handle == e)
				{
					FoundIndex = true;
					break;
				}
			double size = 0;
			curl_easy_getinfo(e, CURLINFO_CONTENT_LENGTH_DOWNLOAD, &size);
			//Adding to merger
			if (segCount != 0 || size > (double)1000)
				Merger::AddFileToMerge(PartsArray[index].IndexInQueue);

			curl_multi_remove_handle(MultiManager, e);
			curl_easy_cleanup(e);

			if (FoundIndex)
			{
				PartsArray[index].OutFile.close();
				if (segCount != 0 || size > (double)1000)
				{
					//Starting next transfer
					PartsArray[index] = SinglePart();
					NewTransfer(URL, InHeaderPath, index, LastIndexInQueue);
					LastIndexInQueue++;
				}
				else {
					std::remove(GetPatchByIndexInQueue(PartsArray[index].IndexInQueue).c_str());
					//Starting next transfer
					PartsArray[index] = SinglePart();
				}
				//Checking if download is completed
				for (int i = 0; i < Rate; i++)
				{
					if (PartsArray[i].IndexInQueue != -1)
						return false;
				}
			}
		}
	}
	if (MultiStillAlive != 0)
		curl_multi_wait(MultiManager, NULL, 0, 1000, NULL);
	else if (!Merger::IsMergning()) { //ending merger
		Merger::End();
		ProgressBar::End();
		FFMpeg::Start(OUTPUT);
		return true;
	}
	return false;
}
float D_video_MP2T::UpdateProgress()
{
	return (float)LastIndexInQueue / segCount;
}