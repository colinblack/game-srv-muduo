#ifndef    MINICURL_H
#define    MINICURL_H

#include <string>
#include <iostream>

 #include <curl/curl.h>
 #include <curl/types.h>
 #include <curl/easy.h>

class CMiniCUrl
{
public:
	int get(std::string& sData, const std::string& sUrl, const std::string& sCookie="")
	{
		curl_global_init(CURL_GLOBAL_ALL);
		CURL* ptrCUrl = curl_easy_init();

		if (NULL == ptrCUrl)
		{
			return -1;
		}

		curl_easy_setopt(ptrCUrl, CURLOPT_HTTPGET, 1);
		curl_easy_setopt(ptrCUrl, CURLOPT_URL, sUrl.c_str());
		curl_easy_setopt(ptrCUrl, CURLOPT_COOKIE, sCookie.c_str());
		curl_easy_setopt(ptrCUrl, CURLOPT_WRITEFUNCTION, callBack);
		curl_easy_setopt(ptrCUrl, CURLOPT_WRITEDATA, &sData);
		CURLcode stRes = curl_easy_perform(ptrCUrl);
		curl_easy_cleanup(ptrCUrl);

		return (CURLE_OK==stRes) ? 0 : -2;
	}
protected:
	static size_t callBack(char* ptrBuff, size_t nSize, size_t nItems, void* ptrStream)
	{
		std::string& sData = *(std::string*)ptrStream;
		sData.append(ptrBuff, nSize*nItems);
		return nSize*nItems;
	}
};

#endif

