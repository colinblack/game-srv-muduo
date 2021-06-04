/*
 * LogicResourceManager.h
 *
 *  Created on: 2016-9-10
 *      Author: dawx62fac
 */

#ifndef LOGICRESOURCEMANAGER_H_
#define LOGICRESOURCEMANAGER_H_

#include "ServerInc.h"

class LogicResourceManager : public CSingleton<LogicResourceManager>
{
private:
	friend class CSingleton<LogicResourceManager>;
	LogicResourceManager() {}
	virtual ~LogicResourceManager() {}
public:
	virtual void CallDestroy() {Destroy();}
	//上线逻辑
	void Online(unsigned uid);
	//离线
	void Offline(unsigned uid);

	//同一天的充值积分
	unsigned GetRechargePays(unsigned uid, unsigned time);

	//获取一段时间内的充值积分
	unsigned GetDurationRechargePays(unsigned uid, unsigned startts, unsigned endts);

	//////////////////////////////////////////////////////////////////////////
	OfflineResourceItem& Get(unsigned uid);
	int Sync(const DataBase& userBase);
	void SyncUserLevel(unsigned uid, unsigned level);
	void Print();
	//////////////////////////////////////////////////////////////////////////
};

#endif /* LOGICRESOURCEMANAGER_H_ */
