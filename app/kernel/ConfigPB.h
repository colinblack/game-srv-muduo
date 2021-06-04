/*
 * ConfigPB.h
 *
 *  Created on: 2016-8-22
 *      Author: Ralf
 */

#ifndef CONFIGPB_H_
#define CONFIGPB_H_

#include "Common.h"
#include "AppDefine.h"

/*
 *  把json解析成pb的基类
 */
class ConfigPBBase
{
public:
	ConfigPBBase(){}
	virtual ~ConfigPBBase(){}
	static bool _j2p(Message* msg, Json::Value &json);
	static void Fail();
};

/*
 *  类模版参数显示定义pb message的类型，构造函数传入json文件名，构造函数中自动解析
 */
template<class T>
class ConfigPB : public ConfigPBBase
{
public:
	ConfigPB(string path, bool allserver=true)
	{
		string buf;
		Json::Value json;
		if(allserver)
			path = MainConfig::GetAllServerPath(CONFIG_JSON_PATH + path);
		else
			path = Config::GetPath("conf/" + path);
		int ret = File::Read(path, buf);
		if(ret)
		{
			Fail();
			error_log("read json file %s error!", path.c_str());
			return;
		}

		Json::Reader reader;
		if(!reader.parse(buf, json))
		{
			Fail();
			error_log("parse json %s error! reason=%s", path.c_str(),  reader.getFormatedErrorMessages().c_str());
			return;
		}

		try
		{
			if(!_j2p(&m_config, json))
			{
				Fail();
				error_log("config %s error!", path.c_str());
			}
		}
		catch(const std::exception& e)
		{
			Fail();
			error_log("config %s throw:%s", path.c_str(), e.what());
		}
	}
	~ConfigPB(){}
	T m_config;
};


#endif /* CONFIGPB_H_ */
