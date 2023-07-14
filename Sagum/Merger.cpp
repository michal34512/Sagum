#include "Merger.h"
std::string Merger::GetPathWithIndex(int _index)
{
	for (size_t i = OUTPUT.size() - 1; i+1 > 0; i--)
		if (OUTPUT[i] == '.')
			return  PREFIX + OUTPUT.substr(0, i) + std::to_string(_index) + OUTPUT.substr(i, i- OUTPUT.size());
	return PREFIX + OUTPUT + std::to_string(_index);
}
void Merger::Start(std::string _OUTPUT, int _StartAt)
{
	OUTPUT = _OUTPUT;
	FilesToMerge.clear();
	NowMerging = _StartAt;
	FileOUTPUT.open(OUTPUT.c_str(), std::ios::out | std::ios::binary);
	FileOUTPUT.rdbuf()->pubsetbuf(outBuffer, sizeof(outBuffer));
	th = std::thread(Update);
}
void Merger::End() 
{
	endTh = true;
	th.join();
	FileOUTPUT.close();
}
void Merger::AddFileToMerge(int _IndexInQueue)
{
	FilesToMerge.push_back(_IndexInQueue);
}
void Merger::Update()
{
	while (!endTh)
	{
		std::this_thread::sleep_for(std::chrono::milliseconds(100));
		for (size_t i = 0; i < FilesToMerge.size(); i++)
		{
			if (NowMerging == FilesToMerge[i])
			{
				if (MergeFile(NowMerging)) {
					FilesToMerge.erase(FilesToMerge.begin() + i);
					NowMerging++;
				}
			}
		}
	}
}
bool Merger::MergeFile(int _index)
{
	//Merging
	std::fstream fileToMerge;
	fileToMerge.rdbuf()->pubsetbuf(inBuffer, sizeof(inBuffer));
	FileOUTPUT.rdbuf()->pubsetbuf(outBuffer, sizeof(outBuffer));
	std::string path = GetPathWithIndex(_index);
	fileToMerge.open(path.c_str(), std::ios::in | std::ios::binary);

	if (fileToMerge.good())
	{
		/*FileOUTPUT << fileToMerge.rdbuf();
		fileToMerge.close();
		std::remove(path.c_str());
		return true;*/
		fileToMerge.seekg(0, std::ios::end);
		std::streampos input_length = fileToMerge.tellg();
		fileToMerge.seekg(0, std::ios::beg);

		// Create a buffer of appropriate size
		std::vector<char> buffer(input_length);

		// Read the contents of the source file into the buffer
		fileToMerge.read(buffer.data(), input_length);

		// Save the contents of the buffer to the destination file
		FileOUTPUT.write(buffer.data(), input_length);

		fileToMerge.close();
		std::remove(path.c_str());
		return true;
	}
	else ReportError::Error("Error while opening temporary file");


	
	return false;
}
bool Merger::IsMergning()
{
	return FilesToMerge.size() != 0;
}
Merger::~Merger()
{
	delete inBuffer;
	delete outBuffer;
}
std::vector<int> Merger::FilesToMerge;
std::thread Merger::th;
bool Merger::endTh = false;
std::string Merger::OUTPUT;
std::fstream Merger::FileOUTPUT;
char Merger::inBuffer[1000000];
char Merger::outBuffer[1000000];
int Merger::NowMerging = 0;
std::string Merger::PREFIX = "_";
