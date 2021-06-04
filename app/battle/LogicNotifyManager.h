#ifndef LOGIC_NOTIFY_MANAGER_H_
#define LOGIC_NOTIFY_MANAGER_H_

#include "Common.h"
#include "Kernel.h"
#include "DataInc.h"

class  LogicNotifyManager : public BattleSingleton, public CSingleton<LogicNotifyManager>
{
private:
	friend class CSingleton<LogicNotifyManager>;
	LogicNotifyManager() {};

public:
	virtual void CallDestroy() { Destroy();}

	int AddNotify(NotifyItem & notify);

	int Process(unsigned uid, ProtoNotify::GetNotifyReq *reqmsg, ProtoNotify::GetNotifyResp * respmsg);

private:
	//获取通知
	int GetNotify(unsigned uid, unsigned id, ProtoNotify::GetNotifyResp * respmsg);
};


#endif  //LOGIC_NOTIFY_MANAGER_H_
