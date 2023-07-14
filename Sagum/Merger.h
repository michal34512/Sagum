#pragma once
#include <thread>
#include <atomic>
#include <chrono>
#include <vector>
#include <iostream>
#include <string>
#include <fstream>
#include <cstdio>
#include "ReportError.h"

static class Merger {
	static std::vector<int> FilesToMerge;
	static std::thread th;
	static bool endTh;
	static std::string OUTPUT;
	static std::fstream FileOUTPUT;
	static char inBuffer[1000000]; //1mb
	static char outBuffer[1000000]; //1mb
	static int NowMerging;
	static std::string GetPathWithIndex(int _index);
	static void Update();
	static bool MergeFile(int _index);
	~Merger();
public:
	static std::string PREFIX;
	static void Start(std::string _OUTPUT, int _StartAt = 0);
	static void End();
	static void AddFileToMerge(int _IndexInQueue);
	static bool IsMergning();
};