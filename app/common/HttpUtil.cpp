/*
 * HttpUtil.cpp
 *
 *  Created on: 2011-5-25
 *      Author: dada
 */


#include "Common.h"
#include <curl/curl.h>

enum HttpRequestType
{
	HTTP_GET = 0,
	HTTP_POST = 1
};

static size_t HttpWriteCallback(void *ptr, size_t size, size_t nmemb, void *userdata)
{
    string * ptrStrRes = (string *)userdata;

    unsigned long sizes = size * nmemb;
    if (!ptr)
    {
        return 0;
    }
    ptrStrRes->append((char *)ptr, sizes);

    return sizes;
}

static bool HttpRequest(string &response, HttpRequestType type, const string &url, const char *data = NULL, const map<string, string> *headers = NULL)
{
	static bool init = false;
    CURLcode res;
	if(!init)
	{
		res = curl_global_init(CURL_GLOBAL_SSL);
	    if (res != CURLE_OK)
	    {
	    	String::Format(response, "curl_global_init fail. ErrorCode=%d, ErrorMessage=%s", res, curl_easy_strerror(res));
	    	return false;
	    }
		init = true;
	}

    CURL *curl;
    curl = curl_easy_init();
    if (curl == NULL)
    {
    	response = "curl_easy_init fail";
        return false;
    }
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    if(type == HTTP_POST)
    {
    	curl_easy_setopt(curl, CURLOPT_HTTPPOST, 1);
    	if(data != NULL)
    	{
        	curl_easy_setopt(curl, CURLOPT_POSTFIELDS, data);
    	}
    }
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 10);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, HttpWriteCallback);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0);
	struct curl_slist *curlHeaders = NULL;
    if(headers != NULL)
    {
    	if(!headers->empty())
    	{
			for(map<string, string>::const_iterator itr = headers->begin(); itr != headers->end(); itr++)
			{
				curlHeaders = curl_slist_append(curlHeaders, (itr->first + ": " + itr->second).c_str());
			}
			curl_easy_setopt(curl, CURLOPT_HTTPHEADER, curlHeaders);
    	}
    }

    response.clear();
    res = curl_easy_perform(curl);

    if(curlHeaders != NULL)
    {
        curl_slist_free_all(curlHeaders);
    }
    curl_easy_cleanup(curl);

    if(res != CURLE_OK)
    {
    	String::Format(response, "curl_easy_perform fail. ErrorCode=%d, ErrorMessage=%s", res, curl_easy_strerror(res));
    	return false;
    }

	return true;
}

bool Network::HttpGetRequest(string &response, const string &url)
{
	return ::HttpRequest(response, HTTP_GET, url);
}

bool Network::HttpGetRequest(string &response, const string &url, const map<string, string> &headers)
{
	return ::HttpRequest(response, HTTP_GET, url, NULL, &headers);
}

bool Network::HttpPostRequest(string &response, const string &url, const string &data)
{
	return ::HttpRequest(response, HTTP_POST, url, data.c_str());
}

bool Network::HttpPostRequest(string &response, const string &url, const string &data, const map<string, string> &headers)
{
	return ::HttpRequest(response, HTTP_POST, url, data.c_str(), &headers);
}
