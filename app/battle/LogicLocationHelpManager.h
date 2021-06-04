#ifndef LOGIC_LOCATION_HELP_MANAGER_H_
#define LOGIC_LOCATION_HELP_MANAGER_H_

#include "Common.h"
#include "Kernel.h"
#include "DataInc.h"
#include "ProtoDynamicInfo.pb.h"


class  LogicLocationHelpManager : public BattleSingleton, public CSingleton<LogicLocationHelpManager>
{
private:
	friend class CSingleton<LogicLocationHelpManager>;
	LogicLocationHelpManager() {}

public:
	virtual void CallDestroy() { Destroy();}

	//定向请求好友帮助
	int Process(unsigned uid, ProtoDynamicInfo::LocationHelpReq *reqmsg);

	//处理好友帮助消息 后台收到请求后将其降级为普通消息
	int Process(unsigned uid, ProtoDynamicInfo::HandleLocationHelpReq *reqmsg, ProtoDynamicInfo::HandleLocationHelpResq *respmsg);

	//同服定向请求帮助添加动态消息
	bool AddLocationHelpDyInfo(unsigned uid,unsigned other_uid,unsigned type_id);

	//跨服定向请求帮助添加动态消息
	bool AddLocationHelpDyInfoOverServer(unsigned uid,unsigned other_uid,unsigned type_id);

};


#endif
