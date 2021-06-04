#include "ServerInc.h"
#include "BattleServer.h"


int LogicDynamicInfoManager::Process(unsigned uid, ProtoDynamicInfo::GetDynamicInfoReq *reqmsg, ProtoDynamicInfo::GetDynamicInfoResp * respmsg)
{
	unsigned dy_show_max = PER_USER_MAX_DYNAMIC_INFO;	//最多动态条数
	unsigned dy_show_cnt = 0;							//实际可以展示的动态条数

	DyInfoItem* dyItem = NULL;
	dyItem = (DyInfoItem*)malloc(sizeof(DyInfoItem)*dy_show_max);
	if(dyItem == NULL)
	{
		error_log("init is failed");
		throw std::runtime_error("init is failed");
	}

	//获取动态条数
	dy_show_cnt = GetDyInfo(uid,dyItem,dy_show_max);

	//设置返回信息
	for(int i = 0; i < dy_show_cnt; i++)
	{
		SetDyRespMsg(uid,dyItem[i],respmsg->add_arraydyinfo());
	}

	delete [] dyItem;
	return 0;
}

int LogicDynamicInfoManager::Process(unsigned uid, ProtoDynamicInfo::DeleteDynamicInfoReq *reqmsg)
{
	// get dyidx
	unsigned dyidx = reqmsg->id();
	// delete dyidx
	DynamicInfoManager::Instance()->DeleteOneDyInfo(uid,dyidx);

	return 0;
}

int LogicDynamicInfoManager::Process(ProtoDynamicInfo::RequestOtherUserMakeDy *msg)
{
	unsigned myuid = msg->othuid();
	unsigned othuid = msg->myuid();
	unsigned type_id = msg->typeid_();
	unsigned product_id = msg->productid();

	DynamicInfoAttach *pattach = new DynamicInfoAttach;
	pattach->op_uid = othuid;
	pattach->coin = msg->coin();
	pattach->words = msg->words();
	pattach->product_id= product_id;

	ProduceOneDyInfo(myuid,type_id,pattach);

	return 0;
}

bool LogicDynamicInfoManager::NotifyNewDy2Client(unsigned uid)
{
	bool ret = false;

	//检查是否有动态
	if(DynamicInfoManager::Instance()->HasNewDyInfo(uid))
	{
		ret = true;
	}

	ProtoDynamicInfo::HasNewDynamicInfoResp * respmsg = new ProtoDynamicInfo::HasNewDynamicInfoResp;
	respmsg->set_hasnewdy(ret);

	//发送动态消息到client
	LogicManager::Instance()->sendMsg(uid,respmsg);

	return ret;
}

int LogicDynamicInfoManager::GetDyInfo(unsigned uid,DyInfoItem dyItem[],unsigned max_count)
{

	map<unsigned,deque<unsigned> > & dyMap = DynamicInfoManager::Instance()->GetDyInfoMap();

	map<unsigned,deque<unsigned> >::iterator it_map = dyMap.find(uid);
	if(it_map == dyMap.end())	//no uid
	{
		return 0;
	}
	if(!it_map->second.size())	//no dynamicInfo
	{
		return 0;
	}
	int show_count = 0;
	int info_size = it_map->second.size();
	if(info_size > max_count)
	{
		info_size = max_count;
	}
	unsigned time = 0;							//用于除去相同的动态
	for(unsigned idx = 0;idx < info_size;++idx)	//从0开始遍历deque
	{
		unsigned dyidx = it_map->second[idx];
		DyInfoItem & item = DynamicInfoManager::Instance()->GetDyInfoItem(uid,dyidx);
		if(0 == idx)
		{
			time = item.ts;
		}
		else
		{
			if(time == item.ts)		//过滤重复动态
			{
				debug_log("find multiple equal dynamic info|uid:%u|type:%u|dyidx:%u",uid,item.type_id,item.dyidx);
				continue;
			}
			else
			{
				time = item.ts;
			}
		}

		if(time > 0)			//过滤空动态
		{
			dyItem[show_count].Set(item);
			++show_count;
		}

	}
	return show_count;

}

bool LogicDynamicInfoManager::SetDyRespMsg(unsigned uid,const DyInfoItem& dyItem,ProtoDynamicInfo::DynamicInfo *msg)
{
	msg->set_id(dyItem.dyidx);
	msg->set_ts(dyItem.ts);
	msg->set_typeid_(dyItem.type_id);
	msg->set_opuid(dyItem.op_uid);

	if(dyItem.type_id == TYPE_DY_BUY || dyItem.type_id == TYPE_DY_HELP_BUY || dyItem.type_id == TYPE_DY_HELP_BUY_DONE)
	{
		msg->set_productid(dyItem.product_id);
		msg->set_coin(dyItem.coin);
	}
	else if(dyItem.type_id == TYPE_DY_ANSWER)
	{
		MsgInfoItem &msgitem = MessageBoardManager::Instance()->GetMsgInfoItem(uid,dyItem.windex);
		msg->set_words(msgitem.words);
	}
	else if(dyItem.type_id == TYPE_DY_INVITE_ALLIANCE || dyItem.type_id == TYPE_DY_INVITE_ALLIANCE_DONE)
	{
		unsigned aid = dyItem.product_id;
		string name;
		if(!CMI->IsNeedConnectByAID(aid))	//商会同服 跨服无法立即获取商会名
		{
			DataAlliance & alliance = DataAllianceManager::Instance()->GetData(aid);
			name = alliance.name;
		}
		msg->set_words(name);
	}

	return true;
}

bool LogicDynamicInfoManager::ProduceOneDyInfo(unsigned uid,unsigned type_id,DynamicInfoAttach *pattach)
{
	bool has = true;

	DyInfoItem dyitem;
	dyitem.type_id = type_id;

	switch(type_id)
	{

		case TYPE_DY_BUY:
		{
			dyitem.op_uid = pattach->op_uid;
			dyitem.product_id = pattach->product_id;
			dyitem.coin = pattach->coin;
			break;
		}
		case TYPE_DY_MSG:
		case TYPE_DY_TREE:
		case TYPE_DY_VISIT:
		case TYPE_DY_SHIP:
		case TYPE_DY_CONCERN:
		case TYPE_DY_WATER:
		case TYPE_DY_HELP_TREE:
		case TYPE_DY_HELP_SHIP:
		case TYPE_DY_HELP_WATER:
		{
			dyitem.op_uid = pattach->op_uid;
			break;
		}
		case TYPE_DY_ANSWER:
		{
			dyitem.op_uid = pattach->op_uid;
			//跨服发留言请求 根据留言内容新建一条留言 返回该条留言的index
			MsgInfoItem msgitem;
			msgitem.senderUid = pattach->op_uid;
			msgitem.receiverUid = uid;
			msgitem.typeId = pattach->product_id;
			strcpy(msgitem.words,pattach->words.c_str());
			unsigned msgidx = LogicMessageBoardManager::Instance()->ProduceOneMsgInfo(uid,msgitem);
			if(-1 == msgidx)
			{
				return false;
			}
			dyitem.windex = msgidx;
			break;
		}
		case TYPE_DY_HELP_BUY:
		{
			dyitem.op_uid = pattach->op_uid;
			dyitem.product_id = pattach->product_id;
			dyitem.coin = pattach->coin;
			dyitem.windex = FriendOrderManager::Instance()->GetFoIndex(pattach->op_uid,uid);
			break;
		}
		case TYPE_DY_FULL:
		{
			break;
		}
		case TYPE_DY_INVITE_ALLIANCE:
		{
			dyitem.product_id = pattach->product_id;	//此处productid 表示商会aid
			dyitem.op_uid = pattach->op_uid;
			break;
		}
		default:
		{
			error_log("no such DynamicInfo type id");
			has = false;
			break;
		}
	}

	//添加一条动态
	if(!AddOneDyInShm(uid,dyitem))
	{
		error_log("AddOneDyInShm fail");
		return false;
	}

	//设置新动态为true/false
	if(!DynamicInfoManager::Instance()->SetHasNewDy(uid,has))
	{
		error_log("SetHasNewMap fail");
		return false;
	}

	if(UserManager::Instance()->IsOnline(uid))
	{
		//判断并通知client有新动态
		NotifyNewDy2Client(uid);
	}

	return true;
}

int LogicDynamicInfoManager::Process(unsigned uid, ProtoDynamicInfo::HasNewDynamicInfoReq *reqmsg, ProtoDynamicInfo::HasNewDynamicInfoResp * respmsg)
{
	//改变动态状态为false
	DynamicInfoManager::Instance()->SetHasNewDy(uid,false);

	//回包
	respmsg->set_hasnewdy(false);

	return 0;
}

bool LogicDynamicInfoManager::AddOneDyInShm(unsigned uid,DyInfoItem & dyitem)
{
	if(DynamicInfoManager::Instance()->AddDyInfo(uid,dyitem))
	{
		return true;
	}
	return false;
}

bool LogicDynamicInfoManager::UpdateOffLine(unsigned uid,unsigned offtime)
{
	if(DynamicInfoManager::Instance()->UpdateOffLineTime(uid,offtime))
	{
		return true;
	}
	return false;
}

bool LogicDynamicInfoManager::CheckClearDemo()
{
	if(DynamicInfoManager::Instance()->CheckClearDemo())
	{
		return true;
	}
	return false;
}

bool LogicDynamicInfoManager::CheckClearDyInfo()
{
	if(DynamicInfoManager::Instance()->CheckClearDyInfo())
	{
		return true;
	}
	return false;
}

bool LogicDynamicInfoManager::Degrade2Normal(unsigned uid,unsigned dyidx)
{
	if(DynamicInfoManager::Instance()->DegradeDy(uid,dyidx))
	{
		return true;
	}
	return false;
}

int LogicDynamicInfoManager::Process(unsigned uid,ProtoDynamicInfo::ClickOrderHelpReq *reqmsg,ProtoDynamicInfo::ClickOrderHelpResp *respmsg)
{
	// get dyidx
	unsigned dyidx = reqmsg->id();
	if(dyidx < 0 || dyidx >= PER_USER_MAX_DYNAMIC_INFO)
	{
		return 0;
	}

	// get type_id
	unsigned type_id = reqmsg->typeid_();
	if(type_id < TYPE_DY_HELP_TREE || type_id > TYPE_DY_HELP_BUY)
	{
		return 0;
	}

	int index = DynamicInfoManager::Instance()->GetFriendOrderIndex(uid,dyidx);
	if(-1 == index)
	{
		respmsg->set_result(2);
	}
	else
	{
		respmsg->set_result(1);
		respmsg->set_index(index);
	}

	return 0;
}
