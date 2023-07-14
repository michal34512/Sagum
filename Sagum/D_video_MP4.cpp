#pragma once
#define CURL_STATICLIB
#include "D_video_MP4.h"

size_t D_video_MP4::WriteData(char* ptr, size_t size, size_t nmemb, void* outfile)
{
	std::fstream* out = static_cast<std::fstream*>(outfile);
	size_t nbytes = size * nmemb;
	out->write(ptr, nbytes);
	return nbytes;
}

int D_video_MP4::ProgressFunction(void* ptr, double TotalToDownload, double NowDownloaded, double TotalToUpload, double NowUploaded)
{
	D_video_MP4::SinglePart* out = static_cast<D_video_MP4::SinglePart*>(ptr);
	out->AlreadyDownloaded = (unsigned long long)NowDownloaded;
	return 0;
}


std::string D_video_MP4::GetPatchByIndexInQueue(int _IndexInQueue)
{
	for (size_t i = OUTPUT.size() - 1; i + 1 > 0; i--)
		if (OUTPUT[i] == '.')
			return PREFIX + OUTPUT.substr(0, i) + std::to_string(_IndexInQueue) + OUTPUT.substr(i, i - OUTPUT.size());
	return PREFIX + OUTPUT + std::to_string(_IndexInQueue);
}

byte D_video_MP4::NewTransfer(int _IndexInArray, int _IndexInQueue)
{
	unsigned long long DownBound = _IndexInQueue * SinglePartSize;
	unsigned long long UpBound = ((_IndexInQueue + 1) * SinglePartSize) - 1;
	if (DownBound > GetHeader::FileSize) return 0;
	if (UpBound > GetHeader::FileSize) UpBound = GetHeader::FileSize;
	
	CURL* handle;
	handle = curl_easy_init();
	PartsArray[_IndexInArray].Handle = handle;
	PartsArray[_IndexInArray].IndexInQueue = _IndexInQueue;
	if (!handle) 
	{
		ReportError::Error("Problem with curl");
		return -1;
	}
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
	if(COOKIEFILE.size()>0)
		curl_easy_setopt(handle, CURLOPT_COOKIEFILE, COOKIEFILE.c_str());
	GetHeadersFromFileWithRange(handle, InHeaderPath, DownBound, UpBound);

	//Preparing URL
	curl_easy_setopt(handle, CURLOPT_URL, URL.c_str());
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
void D_video_MP4::GetHeadersFromFileWithRange(CURL* _cHandle, std::string _InHeader, unsigned long long _DownRange, unsigned long long _UpRange)
{
	std::string line;
	std::fstream InHeaderFile;
	InHeaderFile.open(_InHeader.c_str(), std::ios::in | std::ios::binary);

	struct curl_slist* chunk = NULL;
	bool found = false;
	while (!InHeaderFile.eof())
	{
		getline(InHeaderFile, line);
		//lower case
		std::string phrase = line;
		for (size_t j = 0; j < phrase.size(); j++)
		{
			if ((int)phrase[j] >= 65 && (int)phrase[j] <= 90)
				phrase[j] = (char)(phrase[j] + 32);
		}
		//searching for Content-Range
		if (phrase.find("range") != std::string::npos) {
			int i = 0;
			for (; i < line.length(); i++)
				if (line[i] == ':') break;
			if (i + 1 == line.length()) ReportError::Error("Problem with content range line in headers file");
			else {
				line.erase(i+1, line.length() - i);
				//Content-Range: bytes 0-200000
				//              ^^^^^^^^^^^^^^^
				//             14             29
				line += " bytes="+std::to_string(_DownRange)+"-"+ std::to_string(_UpRange);
				found = true;
			}
		}
		chunk = curl_slist_append(chunk, line.c_str());
	}
	if (!found) ReportError::Error("Content range not found.");
	curl_easy_setopt(_cHandle, CURLOPT_HTTPHEADER, chunk);
}

void D_video_MP4::Start(std::string _URL, std::string _InHeaderPath, std::string _CookieFile, std::string _OUPUT, std::string _PREFIX, bool _Verbose)
{
	URL = _URL;
	InHeaderPath = _InHeaderPath;
	OUTPUT = _OUPUT;
	COOKIEFILE = _CookieFile;
	LastIndexInQueue = 0;
	PREFIX = _PREFIX;
	
	curl_global_init(CURL_GLOBAL_ALL); //init the curl session
	MultiManager = curl_multi_init(); //init curl multi
	curl_multi_setopt(MultiManager, CURLMOPT_MAXCONNECTS, (long)Rate); //setting max rate
	
	PartsArray = new SinglePart[Rate];
	for (int i = 0; i < Rate; i++)
	{
		NewTransfer( i, i);
		LastIndexInQueue++;
	}
	ProgressBar::Start(this);
	Merger::PREFIX = PREFIX;
	Merger::Start(OUTPUT);
}
bool D_video_MP4::Update()
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
			//Adding to merger
			Merger::AddFileToMerge(PartsArray[index].IndexInQueue);
			curl_multi_remove_handle(MultiManager, e);
			curl_easy_cleanup(e);
			if (FoundIndex)
			{
				PartsArray[index].OutFile.close();
				//Starting next transfer
				PartsArray[index] = SinglePart();
				NewTransfer(index, LastIndexInQueue);
				LastIndexInQueue++;
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
	{
		curl_multi_wait(MultiManager, NULL, 0, 1000, NULL);
	}
		
	else if(!Merger::IsMergning()) { //ending merger
		Merger::End();
		ProgressBar::End();
		return true;
	}

	return false;
	
}
float D_video_MP4::UpdateProgress()
{
	unsigned long long totalDownloaded = 0;
	if (LastIndexInQueue >= Rate) totalDownloaded = (LastIndexInQueue - Rate) * SinglePartSize;
	for (int i = 0; i < Rate; i++)
	{
		totalDownloaded += PartsArray[i].AlreadyDownloaded;
	}
	return (float)totalDownloaded / GetHeader::FileSize;
}