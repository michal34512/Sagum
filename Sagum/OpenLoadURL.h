#pragma once
#include <curl/curl.h>
#include <iostream>
#include <string>
#include <vector>
#include <chrono>
#include <thread>

static class OpenLoadURL
{
public:
	static size_t WriteData(char* ptr, size_t size, size_t nmemb, std::string* outfile);
	static std::string GetURL(std::string oldURL);
};