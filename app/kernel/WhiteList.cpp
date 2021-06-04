#include "WhiteList.h"
#include <fstream>

bool WhiteList::Check(const string &openid, const string &openkey)
{
	static unsigned s_wfLastUpdateTime = 0;
	static map<string, string> s_mapWhiteList;
	if(Time::GetGlobalTime() > s_wfLastUpdateTime)
	{
		s_mapWhiteList.clear();
		s_wfLastUpdateTime = CTime::GetDayBeginTime(Time::GetGlobalTime()) + 60 * 60;

		string sWhiteListPath;
		if(!Config::GetValue(sWhiteListPath, CONFIG_WHITE_LIST))
		{
			fatal_log("[config_not_exist][name=white_list]");
			return false;
		}
		string data;
		if(!File::Read(sWhiteListPath, data))
		{
			fatal_log("[white_list_not_exist][path=%s]", sWhiteListPath.c_str());
		}
		vector<string> whiteList;
		CBasic::StringSplitTrim(data, "\n", whiteList);
		for(vector<string>::iterator itr = whiteList.begin(); itr != whiteList.end(); itr++)
		{
			vector<string> whiteItems;
			CBasic::StringSplitTrim(*itr, ":", whiteItems);
			if(whiteItems.size() >= 2)
			{
				s_mapWhiteList[whiteItems[0]] = whiteItems[1];
			}
			else
			{
				error_log("[invalid_white_list_item][data=%s]", itr->c_str());
			}
		}
	}

	map<string, string>::iterator itrFind;
	itrFind = s_mapWhiteList.find(openid);
	if(itrFind != s_mapWhiteList.end())
	{
		if(itrFind->second == openkey)
		{
			return true;
		}
	}
	return false;
}



