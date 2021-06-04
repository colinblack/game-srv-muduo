#include "ServerInc.h"
#include "BattleServer.h"

int LogicLocationHelpManager::Process(unsigned uid, ProtoDynamicInfo::LocationHelpReq *reqmsg)
{
	// get type_id
	unsigned type_id = reqmsg->typeid_();
	if(type_id < TYPE_DY_HELP_TREE || type_id > TYPE_DY_HELP_BUY)
	{
		return 1;
	}

	// get uids
	unsigned size = reqmsg->arrayuid_size();
	if(!size)
	{
		return 1;
	}

	vector<unsigned> vec_uids;
	for(unsigned idx = 0;idx < size;++idx)
	{
		vec_uids.push_back(reqmsg->arrayuid(idx));
	}

	for(unsigned jdx = 0;jdx < size;++jdx)
	{
		unsigned othuid = vec_uids[jdx];
		if(CMI->IsNeedConnectByUID(othuid))		//跨服
		{
			AddLocationHelpDyInfoOverServer(uid,othuid,type_id);
		}
		else		//同服
		{
			AddLocationHelpDyInfo(uid,othuid,type_id);
		}
	}

	return 0;
}

int LogicLocationHelpManager::Process(unsigned uid, ProtoDynamicInfo::HandleLocationHelpReq *reqmsg,ProtoDynamicInfo::HandleLocationHelpResq *respmsg)
{
	// get dyidx
	unsigned dyidx = reqmsg->id();
	if(dyidx < 0 || dyidx >= PER_USER_MAX_DYNAMIC_INFO)
	{
		return 1;
	}

	// get type_id
	unsigned type_id = reqmsg->typeid_();
	if(type_id < TYPE_DY_HELP_TREE)
	{
		return 1;
	}

	// degrade
	LogicDynamicInfoManager::Instance()->Degrade2Normal(uid,dyidx);

	respmsg->set_id(dyidx);
	respmsg->set_newtypeid(type_id + 100);

	return 0;
}

bool LogicLocationHelpManager::AddLocationHelpDyInfo(unsigned uid,unsigned other_uid,unsigned type_id)
{
	//uid:请求者,other_uid:被请求者,定向请求好友帮助会让被请求者增加一条置顶动态
	DynamicInfoAttach *pattach = new DynamicInfoAttach;
	pattach->op_uid = uid;
	if(LogicDynamicInfoManager::Instance()->ProduceOneDyInfo(other_uid,type_id,pattach))
	{
		return true;
	}
	return false;
}

bool LogicLocationHelpManager::AddLocationHelpDyInfoOverServer(unsigned uid,unsigned other_uid,unsigned type_id)
{
	ProtoDynamicInfo::RequestOtherUserMakeDy * msg = new ProtoDynamicInfo::RequestOtherUserMakeDy;
	string ret;
	msg->set_myuid(uid);
	msg->set_othuid(other_uid);
	msg->set_typeid_(type_id);
	msg->set_productid(0);
	msg->set_coin(0);
	msg->set_words(ret);

	return BMI->BattleConnectNoReplyByUID(other_uid, msg);
}
