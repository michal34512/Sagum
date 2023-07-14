# Sagum
The aim of this project was to create a quick video downloader that bypass some host's download speed limitations.

# How does it work?
Many video hosting sites are imposing download speed limitations per each download transfer. It means that if we divide the video file into several smaller segments and download them asynchronously, we will achieve a significantly higher download speed. This is the vurnability that Sagum is designed to exploit.

# When does it not work?
Some video hostings have limitation on transfer count per IP address. When you surpass this number download speed drops significantly. This number can be different depending on the hosting site but in most cases it's 3.
While using sagum it is still possible to restrain this download rate to any number.

# Header & Cookie files
Before downloading any video file you need to create two files:
* <b>header file</b>  - This file contains additional information sent along with an HTTP request or response. HTTP headers provide important metadata about the request or response, such as the content type, authentication credentials, caching directives, and more
  </br>This is the example of how <b>header.txt</b> could look like:
```
# HTTP Header File

Accept: */*
Accept-Encoding: identity;q=1, *;q=0
Accept-Language: pl-PL,pl;q=0.8
Connection: keep-alive
Host: sl108.example.com
If-Range: "625691e3-536bf62"
Range: bytes=0-
Referer: https://www.example.com/
Sec-Ch-Ua: "Not.A/Brand";v="8", "Chromium";v="114", "Brave";v="114"
Sec-Ch-Ua-Mobile: ?0
Sec-Ch-Ua-Platform: "Windows"
Sec-Fetch-Dest: video
Sec-Fetch-Mode: no-cors
Sec-Fetch-Site: same-site
Sec-Gpc: 1
User-Agent: Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/114.0.0.0 Safari/537.36
```
* <b>Cookie file</b> - This file contains all the coockies needed to make a request for the file. It is rarely needed and in most cases, just the <b>header.txt</b> file is sufficient.
  </br>This is the example of how <b>cookie.txt</b> file could look like:
```
# HTTP Cookie File

example.com    FALSE   /   FALSE   0   session_cookie   abcdef123456789
example.com    FALSE   /   FALSE   0   user_cookie      987654321abcdef
```

<p align="center">
  You can obtain both headers.txt and cookies.txt for example by using Google Chrome inspector tool.
</p>

# Using Sagum
Manual:
```
> Sagum.exe --help

Sagum.exe <URL> <OUTPUT> <RATE> <OPTIONS...>

URL is required
OUTPUT & RATE are optional

Options:
/?     --help               software manual
/s     --singlesize         setting the size of single chunks which are being downloaded
/h     --header             setting path to header file (default value is "header.txt")
/c     --cookie             setting path to cookies file (by default soft. doesn't use cookies)
/v     --verbose            prints connection details
```

Lets say you want to download <b>.mp4</b> file from https://www.example.com/file.mp4 </br>
The command could look like this:
```diff
+ OUTPUT=OUTPUT.mp4   HEADER=header.txt   RATE=30   SINGLEPARTSIZE=10000000
Sagum.exe https://www.example.com/file.mp4 Movie.mp4

+ OUTPUT=Movie.mp4   HEADER=header.txt   RATE=30   SINGLEPARTSIZE=10000000
Sagum.exe https://www.example.com/file.mp4 Movie.mp4

+ OUTPUT=Movie.mp4   HEADER=header.txt   RATE=3   SINGLEPARTSIZE=10000000
Sagum.exe https://www.example.com/file.mp4 Movie.mp4 3

+ OUTPUT=Movie.mp4   HEADER=header.txt   RATE=3   SINGLEPARTSIZE=10000000
Sagum.exe https://www.example.com/file.mp4 Movie.mp4 3 --header "C:\User\Admin\Desktop\header.txt"
```
