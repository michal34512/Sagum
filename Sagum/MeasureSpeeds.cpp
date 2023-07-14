#include "MeasureSpeeds.h"
#include "ReportError.h"

void MeasureSpeeds::GetHeadersFromFile(CURL* _cHandle, std::fstream* _InHeader)
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
int  MeasureSpeeds::ProgressFunction(int* _ptr, double _TotalToDownload, double _NowDownloaded, double _TotalToUpload)
{
	*_ptr += (int)_NowDownloaded;
	return 0;
}
size_t MeasureSpeeds::WriteDataFunction(void* _buffer, size_t _size, size_t _nmemb, void* _userp)
{
	if (!MeasureSpeeds::FirstDataReceived)
	{
		MeasureSpeeds::FirstDataReceived = true;
		MeasureSpeeds::StartTime = std::chrono::system_clock::now();
	}
	size_t nbytes = _size * _nmemb;
	MeasureSpeeds::DownloadedData += (int)nbytes;
	return nbytes;
}
unsigned int MeasureSpeeds::DownloadSpeed(std::string _URL, std::string _InHeaderPath)
{
	MeasureSpeeds::FirstDataReceived = false;
	CURL* curl = curl_easy_init();
	std::fstream HeadersFile;
	HeadersFile.open(_InHeaderPath.c_str(), std::ios::in | std::ios::binary);

	if (curl && HeadersFile.good()) {
		curl_easy_setopt(curl, CURLOPT_URL, _URL.c_str());
		GetHeadersFromFile(curl, &HeadersFile);
		curl_easy_setopt(curl, CURLOPT_BUFFERSIZE, 102400L);
		curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 1L);
		curl_easy_setopt(curl, CURLOPT_USERAGENT, "curl/7.79.1");
		curl_easy_setopt(curl, CURLOPT_MAXREDIRS, 50L);
		curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "GET");
		curl_easy_setopt(curl, CURLOPT_FTP_SKIP_PASV_IP, 1L);
		curl_easy_setopt(curl, CURLOPT_TCP_KEEPALIVE, 1L);
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, MeasureSpeeds::WriteDataFunction);
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, nullptr);
		curl_easy_setopt(curl, CURLOPT_TIMEOUT, 10L); //setting test time to 10 sec
		curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, false);//certificate fix
		curl_easy_perform(curl);
		if (!MeasureSpeeds::FirstDataReceived)
		{
			ReportError::Warning("Failed performing the download speed test");
			return 0;
		}
		float time = (float)(std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::system_clock::now() - MeasureSpeeds::StartTime).count()) / 1000000.f;
		if (time != 0)
		{
			unsigned int speed = (float)MeasureSpeeds::DownloadedData / time;
			return speed;
		}
		else 
		{
			ReportError::Warning("Failed performing the download speed test due to problem with timer");
			return 0;
		}
	}
	else if (!curl) {
		ReportError::Error("Problem with curl");
	}
	else
	{
		ReportError::Error("Cannot open header file");
	}
	ReportError::Error("Something went wrong while performing the download speed");
	return 0;
}
int MeasureSpeeds::DownloadedData = 0;
std::chrono::system_clock::time_point MeasureSpeeds::StartTime;
bool MeasureSpeeds::FirstDataReceived = false;

unsigned int MeasureSpeeds::WritingSpeed(std::streamsize _testBufferSize)
{
	char* str = (char*)malloc(sizeof(char) * _testBufferSize);
	if(str!=nullptr)
		memset(str, 'A', _testBufferSize);
	MeasureSpeeds::StartTime = std::chrono::system_clock::now();
	std::fstream A;
	A.open("A.test", std::ios::out | std::ios::binary);
	A.write(str, _testBufferSize);
	A.close();
	std::fstream B;
	B.open("B.test", std::ios::out | std::ios::binary);
	B.write(str, _testBufferSize);
	B.close();
	//Merging
	A.open("A.test", std::ios::in | std::ios::binary);
	B.open("B.test", std::ios::in | std::ios::binary);
	std::fstream C;
	C.open("C.test", std::ios::app | std::ios::binary);
	static size_t const BufferSize = 8192;
	char buffer[BufferSize];
	while (A.read(&buffer[0], BufferSize)) {
		C.write(&buffer[0], BufferSize);
	}C.write(&buffer[0], A.gcount());
	while (B.read(&buffer[0], BufferSize)) {
		C.write(&buffer[0], BufferSize);
	}C.write(&buffer[0], B.gcount());
	A.close();
	B.close();
	C.close();
	std::remove("A.test");
	std::remove("B.test");
	float time = (float)(std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::system_clock::now() - MeasureSpeeds::StartTime).count()) / 1000000.f;
	std::remove("C.test");
	if(time!=0)
		return (float)_testBufferSize * 2.f / time;
	else {
		ReportError::Warning("Failed performing the download speed test due to problem with timer");
		return 0;
	}
}