/*
 * Config.cpp
 *
 *  Created on: 2011-5-25
 *      Author: dada
 */

#include "Config.h"
#include "Common.h"
#include "../kernel/AppDefine.h"

static map<string, string> g_config;

bool Config::Init(const string &path)
{
	g_config.clear();
	CMarkupSTL xmlConf;
	if(!xmlConf.Load(path.c_str()))
	{
		return false;
	}
	if(!xmlConf.FindElem("configs") )
	{
		return false;
	}

	xmlConf.IntoElem();
	while(xmlConf.FindElem("config"))
	{
		string name = xmlConf.GetAttrib("name");
		string value = xmlConf.GetAttrib("value");
		g_config[name] = value;
	}

	return true;
}

bool Config::GetValue(string &value, const string &name)
{
	map<string, string>::iterator itr = g_config.find(name);
	if(itr == g_config.end())
	{
		return false;
	}
	value = itr->second;
	return true;
}

string Config::GetValue(const string &name)
{
	string value;
	Config::GetValue(value, name);
	return value;
}

bool Config::GetIntValue(int &value, const string &name)
{
	string sValue;
	if(!Config::GetValue(sValue, name))
	{
		return false;
	}
	return Convert::StringToInt(value, sValue);
}

int Config::GetIntValue(const string &name)
{
	int value;
	if(!Config::GetIntValue(value, name))
	{
		return 0;
	}
	return value;
}

bool Config::GetUIntValue(unsigned &value, const string &name)
{
	string sValue;
	if(!Config::GetValue(sValue, name))
	{
		return false;
	}
	return Convert::StringToUInt(value, sValue);
}

string Config::GetPath(string path, int server)
{
	if(server == 0)
		server = GetIntValue(CONFIG_SRVID);
	string serverPath;
	if(server == 0)
		serverPath = "../";
	else
	{
		if(!MainConfig::GetValue(serverPath, "server_path"))
			serverPath = DEFAULT_APP_PATH;
		if(serverPath[serverPath.length() - 1] != '/')
			serverPath.append("/");
		string serverid;
		String::Format(serverid, "s%d/", server);
		serverPath += serverid;
	}

	return serverPath + path;
}

unsigned Config::GetUIDStart(unsigned serverid)
{
	if(serverid == 0)
		serverid = GetIntValue(CONFIG_SRVID);
	return UID_MIN + UID_ZONE * (serverid - 1);
}
unsigned Config::GetZoneByUID(unsigned uid)
{
	return (uid - UID_MIN) / UID_ZONE + 1;
}
unsigned Config::GetAIDStart(unsigned serverid)
{
	if(serverid == 0)
		serverid = GetIntValue(CONFIG_SRVID);
	return ALLIANCE_ID_START + AID_ZONE * (serverid - 1);
}
unsigned Config::GetZoneByAID(unsigned aid)
{
	return (aid - ALLIANCE_ID_START) / AID_ZONE + 1;
}

