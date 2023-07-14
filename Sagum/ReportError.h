#pragma once
#include <iostream>
static class ReportError
{
public:
	static void Error(std::string log);
	static void Warning(std::string log);
};