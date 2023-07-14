#include "FFmpeg.h"
void FFMpeg::Start(std::string OUTPUT)
{
	std::cout << "searching for FFMPEG...\n" << std::endl;;
	std::remove("EXIST.txt");
	system("where /q ffmpeg");
	system("IF ERRORLEVEL 1 (echo EXIST>EXIST.txt)");
	std::fstream Exist;
	Exist.open("EXIST.txt", std::ios::in);
	if (!Exist.good())
	{
		std::cout << "FFMPEG found, reconverting file... \n" << std::endl;
		std::rename(OUTPUT.c_str(), ("_" + OUTPUT).c_str());
		std::fstream MList;
		MList.open("MList.txt", std::ios::out);
		MList << "file _" + OUTPUT;
		MList.close();
		system(("ffmpeg -y -f concat -safe 0 -i MList.txt -bsf:a aac_adtstoasc -c copy " + OUTPUT).c_str());
		std::remove("MList.txt");
		std::remove(("_" + OUTPUT).c_str());
		std::cout << "File reconverted, task successful..." << std::endl;

	}
	else {
		ReportError::Warning("FFMPEG not found, task successful...");
		Exist.close();
		std::remove("EXIST.txt");
	}
}