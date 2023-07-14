#include "ReportError.h"

void ReportError::Error(std::string log)
{
	std::cout << "\x1B[31mERROR:\033[0m " << log << std::endl;
}
void ReportError::Warning(std::string log)
{
	std::cout << "\x1B[33mWARNING\033[0m " << log << std::endl;
}