/*
 * MainConfig.h
 *
 *  Created on: 2016-8-22
 *      Author: Ralf
 */

#ifndef MAINCONFIG_H_
#define MAINCONFIG_H_


#include <string>
using std::string;
#include <set>
using std::set;

namespace MainConfig
{
	bool Init(const string &path);
	bool GetValue(string &value, const string &name);
	bool GetDB(string &value, const string &name);
	bool GetIntDB(int &db, const string &name);
	bool GetIntValue(int &value, const string &name);
	int GetMergedDomain(int serverid);
	void GetDomains(set<int> &domains);
	void GetIncludeDomains(int serverid,set<int> &domains);
	string GetHost(int serverid);

	string GetAllServerPath(string path);
}


#endif /* MAINCONFIG_H_ */
