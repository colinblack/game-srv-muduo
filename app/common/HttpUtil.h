/*
 * HttpUtil.h
 *
 *  Created on: 2011-5-25
 *      Author: dada
 */

#ifndef HTTPUTIL_H_
#define HTTPUTIL_H_

#include <string>
using std::string;

namespace Network
{
	//return: true - success; false - error, error message in response;
	bool HttpGetRequest(string &response, const string &url);
	bool HttpGetRequest(string &response, const string &url, const map<string, string> &headers);
	bool HttpPostRequest(string &response, const string &url, const string &data);
	bool HttpPostRequest(string &response, const string &url, const string &data, const map<string, string> &headers);
}

#endif /* HTTPUTIL_H_ */
