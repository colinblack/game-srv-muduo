/*
 * Config.h
 *
 *  Created on: 2011-5-25
 *      Author: dada
 */

#ifndef CONFIG_H_
#define CONFIG_H_

#include <string>
using namespace std;

namespace Config
{
	bool Init(const string &path);
	bool GetValue(string &value, const string &name);
	string GetValue(const string &name);
	bool GetIntValue(int &value, const string &name);
	int GetIntValue(const string &name);
	bool GetUIntValue(unsigned &value, const string &name);

	string GetPath(string path, int server = 0);

	unsigned GetUIDStart(unsigned serverid = 0);
	unsigned GetAIDStart(unsigned serverid = 0);
	unsigned GetZoneByUID(unsigned uid);
	unsigned GetZoneByAID(unsigned aid);
};

#endif /* CONFIG_H_ */
