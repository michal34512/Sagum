#pragma once
#include <iostream>
#include "GetHeader.h"

class DownloadFile
{
public:
	virtual void Start(std::string _URL, std::string _InHeaderPath, std::string _CookieFile, std::string _OUPUT, std::string _PREFIX, bool _Verbose = false);
	virtual bool Update(); //returns true if done
	virtual float UpdateProgress();
};