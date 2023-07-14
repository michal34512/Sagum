#define CURL_STATICLIB

#include <iostream>
#include <curl/curl.h>
#include <fstream>
#include <windows.h>
#include <string>
#include <math.h>
#include <thread>
#include <chrono>         // std::chrono::seconds

#include "GetHeader.h"
#include "DownloadFile.h"
#include "D_video_MP4.h"
#include "D_video_MP2T.h"
#include "ReportError.h"
#include "OpenLoadURL.h"

int main(int argc, char* argv[])
{
	std::string URL = "";
	std::string HEADERPATH = "header.txt";
	std::string COOKIEPATH = "";
	std::string OUTPUT = "OUTPUT.mp4";
	std::string SOFTNAME = "_NULL_";
	std::string PREFIX = "_";

	unsigned long long SINGLEPARTSIZE = 10000000; //10mb
	int RATE = 30;
	bool VerboseMode = false;


	/*USER INTERFACE*/
	#pragma region Cleaning_And_Other_Stuff
	system("CHCP 437"); //Colorful cmd
	printf("\033[A");
	printf("\33[2K\r");
	system(("del " + PREFIX + "* > NUL").c_str());
	printf("\033[A");
	printf("\33[2K\r");
	#pragma endregion

	#pragma region Getting_Soft_Name
	if (argc >= 1)
	{
		SOFTNAME = argv[0];
		SOFTNAME.erase(0, SOFTNAME.rfind("\\")+1);
	}
	#pragma endregion

	#pragma region Getting_Arguments
	std::vector<std::string>  arguments;
	if (argc > 1)
	{
		for (int i = 1; i < argc; i++)
		{
			arguments.push_back(argv[i]);
		}
	}
	else {
		std::string Command;
		std::cout << ">>> ";
		getline(std::cin, Command);
		std::string arg = "";
		size_t pos = 0;
		for (; pos<Command.length(); pos++)
		{
			if (Command[pos] != ' ')
			{
				break;
			}
		}
		for (; pos < Command.length(); pos++)
		{
			if (Command[pos] != ' ')
			{
				arg += Command[pos];
			}
			else {
				if (arg != "")
				{
					arguments.push_back(arg);
					arg = "";
				}
			}
		}
		if (arg != "") arguments.push_back(arg);
	}
	for (int i = 0; i < arguments.size(); i++)
	{
		if (arguments[i].find("--help") == 0 || arguments[i].find("/?") == 0 || arguments[i].find("-h") == 0) //help argument
		{
			std::cout << std::endl << SOFTNAME << " <URL> <OUTPUT> <RATE> <OPTIONS...>" << std::endl << std::endl;
			std::cout << "URL is required" << std::endl;
			std::cout << "OUTPUT & RATE are optional" << std::endl << std::endl;
			std::cout << "Options:" << std::endl;
			std::cout << "/?     --help               software manual" << std::endl;
			std::cout << "/s     --singlesize         setting the size of single chunks which are being downloaded" << std::endl;
			std::cout << "/h     --header             setting path to header file (default value is \"header.txt\")" << std::endl;
			std::cout << "/c     --cookie             setting path to cookies file (by default soft. doesn't use cookies)" << std::endl;
			std::cout << "/v     --verbose            prints connection details" << std::endl;
			return 0;
		}
	}
	int argIndex = 0;
	for (int i = 0; i < arguments.size(); i++) //next arguments
	{
		if (arguments[i].find("--") == 0 || arguments[i].find("/") == 0)
		{
			if (i + 1 <= arguments.size() - 1)
			{
				if (arguments[i] == "/s" || arguments[i] == "--singlesize")
				{
					try {
						SINGLEPARTSIZE = std::stoull(arguments[i + 1], nullptr, 0);
					}
					catch (std::invalid_argument& e) {
						ReportError::Error("Wrong arguments, check --help.");
						return 1;
					}
				}
				else if (arguments[i] == "/h" || arguments[i] == "--header")
				{
					HEADERPATH = arguments[i + 1];
				}
				else if (arguments[i] == "/c" || arguments[i] == "--cookie")
				{
					COOKIEPATH = arguments[i + 1];
				}
			}
			if (arguments[i] == "/v" || arguments[i] == "--verbose")
			{
				VerboseMode = true;
			}
			i++;
		}
		else {
			switch (argIndex)
			{
			case 0:
				URL = arguments[i];
				break;
			case 1:
				OUTPUT = arguments[i];
				break;
			case 2:
				try {
					RATE = stoi(arguments[i]);
				}
				catch (std::invalid_argument& e) {
					ReportError::Error("Wrong arguments, check --help.");
					return 1;
				}
				break;
			default:
				ReportError::Error("Too many arguments, check --help.");
				return 1;
				break;
			}
			argIndex++;
		}
	}
	if (URL.find("userload.co") != std::string::npos)
	{
		URL = OpenLoadURL::GetURL(URL);
	}
	if (URL == "")
	{
		ReportError::Error("Error while getting URL");
		return 1;
	}
	URL.erase(std::remove(URL.begin(), URL.end(), '\''), URL.end());
	URL.erase(std::remove(URL.begin(), URL.end(), '\"'), URL.end());
	#pragma endregion

	#pragma region Get_File_Size
	GetHeader::GetOutHeaders(URL, HEADERPATH);
	std::cout << "\x1B[96mFile size: \033[0m";
	if (GetHeader::GetContentLength() == 0) std::cout << "Unknown file size" << std::endl;
	else std::cout << GetHeader::GetContentLength() << std::endl;
	#pragma endregion 

	#pragma region Print_Response_Headers
	if (VerboseMode)
	{
		std::cout << "Printing response headers from the server..." << std::endl << std::endl;
		for (size_t i = 0; i < GetHeader::OutHeaders.size(); i++)
			std::cout << GetHeader::OutHeaders[i] << std::endl;
	}
	#pragma endregion

	#pragma region Downloading_File
	std::cout << "\x1B[95mURL: \033[0m" << URL << std::endl;
	DownloadFile * client = nullptr;
	std::cout << "\x1B[96mFile type: \033[0m";
	switch (GetHeader::RecognizeType())
	{

		case GetHeader::FileType::video_MP4:
		{
			std::cout << "mp4" << std::endl;
			D_video_MP4 a;
			a.Rate = RATE;
			a.SinglePartSize = SINGLEPARTSIZE;
			client = &a;
		}
		break;
		case GetHeader::FileType::video_MP2T:
		{
			std::cout << "mp2t" << std::endl;
			D_video_MP2T a;
			a.Rate = RATE;
			client = &a;
		}
		break;
		default: //UNKNOWN UNDEFINED
		{
			std::cout << "Unsupported file type" << std::endl;
			std::cout << "Do you still want to try download it? (Y/N)" << std::endl;
			char ans;
			std::cin >> ans; 
			if (ans == 'y' || ans == 'Y')
			{
				D_video_MP4 a;
				a.Rate = RATE;
				a.SinglePartSize = SINGLEPARTSIZE;
				client = &a;
			}
			else return 0;
		}
		break;
	}
	if (client == nullptr)
	{
		ReportError::Error("Could not start download client");
		return -1;
	}
	client->Start(URL, HEADERPATH, COOKIEPATH, OUTPUT, PREFIX);
	while (true) { std::this_thread::sleep_for(std::chrono::milliseconds(100)); if (client->Update()) break; }
	#pragma endregion
}