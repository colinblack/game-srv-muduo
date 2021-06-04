/*
 * ActivitySingleton.h
 *
 *  Created on: 2016-10-20
 *      Author: Ralf
 */

#ifndef ACTIVITYSINGLETON_H_
#define ACTIVITYSINGLETON_H_


#include "Common.h"
#include "ConfigManager.h"

class ActivitySingletonBase
{
public:
	ActivitySingletonBase(){}
	virtual ~ActivitySingletonBase(){}
	virtual bool IsOn() = 0;
	virtual int OnInit() = 0;
	virtual int Init(){return 0;}
	virtual void CheckDay() = 0;
	virtual void OnReload() = 0;
	virtual void OnExit() = 0;
	virtual void OnMin() = 0;
	virtual void OnHour() = 0;
	virtual void CallDestroy() = 0;

};
/*
 *  活动管理的基类，继承该类后会在LogicManager中调用各个函数和虚函数
 *  参数T是AppDefine.h里的枚举ActivityID，对应ActivityTime.json配置中的id
 *  模版有2个功能：
 *  1.提供统一的开启/关闭接口，启动时检测开启活动（要检测处理版本号是否一致），跨日时检测开启/关闭活动，重载配置时开启/关闭活动
 *  2.提供统一的分钟、小时、日定时器接口
 *  只有IsOn为true时才会调用OnExit OnMin OnHour
 */
template <unsigned T>
class ActivitySingleton : public ActivitySingletonBase
{
public:
	virtual ~ActivitySingleton(){}
	ActivitySingleton():m_ver(0),m_bts(0),m_ets(0),m_on(false){}
	bool IsOn()
	{
		return m_on;
	}
	int OnInit()
	{
		User::ActivityItem act;
		if(ConfigManager::Instance()->GetActivity(T, act))
		{
			m_bts = act.bts();
			m_ets = act.ets();
			m_ver = act.ver();
			if(CheckTS())
			{
				m_on = true;
				OnStart();
			}
			else
			{
				m_on = false;
				OnEnd();
			}
		}
	}
	void CheckDay()
	{
		if(CheckTS())
		{
			if(!m_on)
			{
				m_on = true;
				OnStart();
			}
			else
				OnDay();

		}
		else if(m_on)
		{
			m_on = false;
			OnEnd();
		}
	}
	void OnReload()
	{
		User::ActivityItem act;
		if(ConfigManager::Instance()->GetActivity(T, act))
		{
			m_bts = act.bts();
			m_ets = act.ets();
			m_ver = act.ver();
			if(CheckTS() && !m_on)
			{
				m_on = true;
				OnStart();
			}
			else if(!CheckTS() && m_on)
			{
				m_on = false;
				OnEnd();
			}
		}
		else if(m_on)
		{
			m_bts = 0;
			m_ets = 0;
			m_ver = 0;
			m_on = false;
			OnEnd();
		}
	}

	virtual void OnExit(){}
	virtual void OnMin(){}
	virtual void OnHour(){}
	virtual void CallDestroy() = 0;

	unsigned GetVersion() const
	{
		return m_ver;
	}

	unsigned GetBeginTs() const
	{
		return m_bts;
	}

	unsigned GetEndTs() const
	{
		return m_ets;
	}

private:
	bool CheckTS()
	{
		unsigned ts = Time::GetGlobalTime();
		if(ts >= m_bts && ts < m_ets)
			return true;
		return false;
	}
	virtual void OnStart(){}
	virtual void OnEnd(){}
	virtual void OnDay(){}

	unsigned m_ver,m_bts,m_ets;
	bool m_on;
};




#endif /* ACTIVITYSINGLETON_H_ */
