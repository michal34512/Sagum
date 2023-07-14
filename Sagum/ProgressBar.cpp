#include "ProgressBar.h"
void ProgressBar::Start(DownloadFile * _segment)
{
	th = std::thread(Update);
	std::cout << std::endl;
	endTh = false;
	segment = _segment;
}
void ProgressBar::End()
{
	if (!endTh)
	{
		endTh = true;
		th.join();
	}
}
void ProgressBar::Update() 
{
	while (!endTh)
	{
		std::this_thread::sleep_for(std::chrono::seconds(1));
		//Update progress
		float progress = segment->UpdateProgress();
		//Draw progress
		printf("\033[A");
		printf("\33[2K\r");
		if (progress < 1.0) {
			int barWidth = 30;
			std::cout << "\x1B[31m[\033[0m";
			int pos = barWidth * progress;
			for (int i = 0; i < barWidth; ++i) {
				if (i < pos) std::cout<<"\x1B[32m\xDC\033[0m";
				else std::cout << " ";
			}
			std::cout << "\x1B[31m]\033[0m " << int(progress * 100.0) << " %\r";
		}
		std::cout << std::endl;
	}
}
bool ProgressBar::endTh = true;
std::thread ProgressBar::th;
DownloadFile *ProgressBar::segment;