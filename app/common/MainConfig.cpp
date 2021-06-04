/*
 * MainConfig.cpp
 *
 *  Created on: 2016-8-22
 *      Author: Ralf
 */


#include "MainConfig.h"
#include "Common.h"
#include <sys/stat.h>
#include <unistd.h>
#include "../kernel/AppDefine.h"

static map<string, pair<string, string> > g_main_config;
static map<int, int> g_merge_domain;
static map<int, string> g_domain_host;

bool MainConfig::Init(const string &path) {
	CMarkupSTL xmlConf;
	if (!xmlConf.Load(path.c_str())) {
		return false;
	}
	if (!xmlConf.FindElem("configs")) {
		return false;
	}
	xmlConf.IntoElem();

	while (xmlConf.FindElem("config")) {
		string name = xmlConf.GetAttrib("name");
		string value = xmlConf.GetAttrib("value");
		string db = xmlConf.GetAttrib("db");
		if (db.empty())
			db = "0";
		g_main_config[name] = pair<string, string>(value, db);
		int serverid = 0;
		int domainid = 0;
		Convert::StringToInt(domainid, value);
		Convert::StringToInt(serverid, db);
		if (serverid && domainid)
		{
			g_merge_domain[serverid] = domainid;
			g_domain_host[serverid] = name;
		}
	}

	return true;
}

bool MainConfig::GetValue(string &value, const string &name) {
	map<string, pair<string, string> >::iterator itr = g_main_config.find(name);
	if (itr == g_main_config.end()) {
		return false;
	}
	value = itr->second.first;
	return true;
}
bool MainConfig::GetDB(string &value, const string &name) {
	map<string, pair<string, string> >::iterator itr = g_main_config.find(name);
	if (itr == g_main_config.end()) {
		return false;
	}
	value = itr->second.second;
	return true;
}

bool MainConfig::GetIntValue(int &value, const string &name) {
	string sValue;
	if (!MainConfig::GetValue(sValue, name)) {
		return false;
	}
	return Convert::StringToInt(value, sValue);
}
bool MainConfig::GetIntDB(int &value, const string &name) {
	string sValue;
	if (!MainConfig::GetDB(sValue, name)) {
		return false;
	}
	return Convert::StringToInt(value, sValue);
}

int MainConfig::GetMergedDomain(int serverid) {
	if (g_merge_domain.count(serverid))
		return g_merge_domain[serverid];
	return 0;
}
void MainConfig::GetDomains(set<int> &domains) {
	for (map<int, int>::iterator it = g_merge_domain.begin();
			it != g_merge_domain.end(); ++it)
		domains.insert(it->second);
}
void MainConfig::GetIncludeDomains(int serverid, set<int> &domains) {
	for (map<int, int>::iterator it = g_merge_domain.begin();
			it != g_merge_domain.end(); ++it)
		if (it->second == serverid)
			domains.insert(it->first);
}
string MainConfig::GetHost(int serverid) {
	if (g_domain_host.count(serverid))
		return g_domain_host[serverid];
	return "";
}

string MainConfig::GetAllServerPath(string path)
{
	string serverPath;
	if(!MainConfig::GetValue(serverPath, "server_path"))
		serverPath = DEFAULT_APP_PATH;
	if (serverPath[serverPath.length() - 1] != '/')
		serverPath.append("/");
	return serverPath + path;
};
