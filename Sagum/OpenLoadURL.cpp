#include "OpenLoadURL.h"
#define CURL_STATICLIB

size_t OpenLoadURL::WriteData(char* ptr, size_t size, size_t nmemb, std::string* outfile)
{
	std::string * a = dynamic_cast<std::string*>(outfile);
	*a += ptr;
	size_t nbytes = size * nmemb;
	return nbytes;
}
std::string OpenLoadURL::GetURL(std::string oldURL)
{
	std::string htmlcode = "";

	CURL* curl = curl_easy_init();
	/*GETTING MOROCCO*/
	if (curl) {
		curl_easy_setopt(curl, CURLOPT_URL, oldURL.c_str());
		curl_easy_setopt(curl, CURLOPT_BUFFERSIZE, 102400L);
		curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 1L);
		curl_easy_setopt(curl, CURLOPT_USERAGENT, "curl/7.79.1");
		curl_easy_setopt(curl, CURLOPT_MAXREDIRS, 50L);
		curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "GET");
		curl_easy_setopt(curl, CURLOPT_FTP_SKIP_PASV_IP, 1L);
		curl_easy_setopt(curl, CURLOPT_TCP_KEEPALIVE, 1L);
		curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, false);//certificate fix
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteData);
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, &htmlcode);
		curl_easy_perform(curl);
		int RetrySec = 30;
		for (int i = 0; i < RetrySec; i++) // Waiting for 3 seconds 
		{
			std::this_thread::sleep_for(std::chrono::milliseconds(100));
			if (htmlcode.size() != 0)
			{
				size_t firstChar = htmlcode.find("var||") + 5;
				if (firstChar == std::string::npos) return ""; // Could not find morocco
				size_t lastChar = htmlcode.find("\'", firstChar);
				if (lastChar == std::string::npos && lastChar > firstChar) return ""; // Could not find morocco
				htmlcode = htmlcode.substr(firstChar, lastChar - firstChar);

				std::vector<std::string> sliced;
				firstChar = 0;
				lastChar = 0;
				while (lastChar != std::string::npos)
				{
					lastChar = htmlcode.find("|", firstChar);
					sliced.push_back(htmlcode.substr(firstChar, lastChar - firstChar));
					firstChar = lastChar + 1;
				}

				for (int i = 0; i < sliced.size(); i++)
				{
					if (sliced[i].length() == 31) htmlcode = sliced[i];
				}
				break;
			}
		}
		if(htmlcode.size()==0) return "";
	}
	else return ""; // Problem with curl
	curl_easy_cleanup(curl);
	curl = curl_easy_init();
	/*GETTING NEW LINK*/
	std::string newLink = "";
	if (curl) {
		curl_easy_setopt(curl, CURLOPT_URL, "https://userload.co/api/request/");
		curl_easy_setopt(curl, CURLOPT_BUFFERSIZE, 102400L);
		curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 1L);
		curl_easy_setopt(curl, CURLOPT_MAXREDIRS, 50L);
		curl_easy_setopt(curl, CURLOPT_FTP_SKIP_PASV_IP, 1L);
		curl_easy_setopt(curl, CURLOPT_TCP_KEEPALIVE, 1L);
		curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, false);//certificate fix
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteData);
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, &newLink);
		curl_easy_setopt(curl, CURLOPT_POST, 1);
		std::string post = "morocco=" + htmlcode + "&mycountry=motherfucker";
		curl_easy_setopt(curl, CURLOPT_POSTFIELDS, post.c_str());
		curl_easy_perform(curl);
		int RetrySec = 30;
		for (int i = 0; i < RetrySec; i++) // Waiting for 3 seconds 
		{
			std::this_thread::sleep_for(std::chrono::milliseconds(100));
			if (newLink.size() != 0)
			{
				if(newLink.find(".mp4")!=std::string::npos)
				{
					newLink = newLink.substr(0,newLink.rfind(".mp4")+4);
					return newLink;
				}

				break;
			}
		}
		if (newLink.size() == 0) return "";
	}
	else return ""; // Problem with curl
	return "";
}