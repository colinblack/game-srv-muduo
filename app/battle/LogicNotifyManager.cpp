#include "LogicNotifyManager.h"

int LogicNotifyManager::AddNotify(NotifyItem & notify)
{
	unsigned uid = notify.uid;

	int ret = NotifyManager::Instance()->Add(notify);

	if (ret)
	{
		error_log("add notify error. uid=%u,ret=%d", uid, ret);
		return ret;
	}

	return 0;
}

int LogicNotifyManager::Process(unsigned uid, ProtoNotify::GetNotifyReq *reqmsg, ProtoNotify::GetNotifyResp * respmsg)
{
	unsigned id = reqmsg->id();

	GetNotify(uid, id, respmsg);

	return 0;
}

int LogicNotifyManager::GetNotify(unsigned uid, unsigned id, ProtoNotify::GetNotifyResp * respmsg)
{
	//根据uid和id判断是否存在该通知数据
	bool ishave = NotifyManager::Instance()->Has(uid, id);

	if (ishave)
	{
		NotifyItem & notify = NotifyManager::Instance()->Get(uid, id);
		notify.SetMessage(respmsg->mutable_notify());

		//获取了通知之后，删除
		NotifyManager::Instance()->DelItem(notify.uid, notify.id);
	}

	return 0;
}
