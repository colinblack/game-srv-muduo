#include "LogicFriendManager.h"
#include "BattleServer.h"


int LogicFriendManager::Process(unsigned uid, ProtoFriend::GetAllFriendsReq* req)
{
	ProtoFriend::GetAllFriendsResp* resp = new ProtoFriend::GetAllFriendsResp;

	GetConcernFriendInfo(uid,resp);
	GetFansFriendInfo(uid,resp);
	GetAidFriendInfo(uid,resp);

	return LMI->sendMsg(uid,resp)?0:R_ERROR;
}

void LogicFriendManager::GetConcernFriendInfo(unsigned uid,ProtoFriend::GetAllFriendsResp *resp)
{
	//获取关注人信息
	vector<unsigned> indexs;
	DataConcernManager::Instance()->GetIndexs(uid, indexs);
	for(int i = 0; i < indexs.size(); ++i)
	{
		DataConcern & data = DataConcernManager::Instance()->GetDataByIndex(indexs[i]);
		resp->add_concerns()->mutable_concernfolk()->set_folkuid(data.id);
	}
}

void LogicFriendManager::GetFansFriendInfo(unsigned uid,ProtoFriend::GetAllFriendsResp *resp)
{
	//获取关注人信息
	vector<unsigned> indexs;
	DataFansManager::Instance()->GetIndexs(uid, indexs);
	for(int i = 0; i < indexs.size(); ++i)
	{
		DataFans & data = DataFansManager::Instance()->GetDataByIndex(indexs[i]);

		resp->add_fans()->mutable_fan()->set_folkuid(data.id);
	}
}


void LogicFriendManager::GetAidFriendInfo(unsigned uid,ProtoFriend::GetAllFriendsResp *resp)
{
	//获取关注人信息
	vector<unsigned> indexs;
	DataAidRecordManager::Instance()->GetRecentAidRecord(uid, indexs);
	for(int i = 0; i < indexs.size(); ++i)
	{
		DataAidRecord & data = DataAidRecordManager::Instance()->GetDataByIndex(indexs[i]);
		resp->add_helpers()->set_folkuid(data.aid_id);
	}
}

//请求需要帮助的好友数据
int LogicFriendManager::Process(unsigned uid, ProtoFriend::GetFriendHelpInfoReq* req)
{
	map<unsigned,vector<unsigned> > zoneUids;
	zoneUids.clear();
	ProtoFriend::CSGetFriendHelpInfoResp *resp = new ProtoFriend::CSGetFriendHelpInfoResp;

	//获取关注人信息
	vector<unsigned> indexs;
	DataConcernManager::Instance()->GetIndexs(uid, indexs);
	for(int i = 0; i < indexs.size(); ++i)
	{
		DataConcern & data = DataConcernManager::Instance()->GetDataByIndex(indexs[i]);
		//记录需要跨服处理的uid
		if(CMI->IsNeedConnectByUID(data.id))
		{
			unsigned zone = Config::GetZoneByUID(data.id);
			zoneUids[zone].push_back(data.id);
		}
		else
		{
			if(LogicUserManager::Instance()->IsUserNeedHelp(data.id))
			{
				resp->add_othuid(data.id);
			}
		}

	}

	//获取粉丝信息
	indexs.clear();
	DataFansManager::Instance()->GetIndexs(uid, indexs);
	for(int i = 0; i < indexs.size(); ++i)
	{
		DataFans & data = DataFansManager::Instance()->GetDataByIndex(indexs[i]);
		//记录需要跨服处理的uid
		if(CMI->IsNeedConnectByUID(data.id))
		{
			unsigned zone = Config::GetZoneByUID(data.id);
			zoneUids[zone].push_back(data.id);
		}
		else
		{
			if(LogicUserManager::Instance()->IsUserNeedHelp(data.id))
			{
				resp->add_othuid(data.id);
			}
		}
	}



	for(map<unsigned,vector<unsigned> >::iterator it = zoneUids.begin(); it != zoneUids.end(); it++)
	{
		ProtoFriend::CSGetFriendHelpInfoReq *getFriendReq = new ProtoFriend::CSGetFriendHelpInfoReq;
		getFriendReq->set_myuid(uid);
		for(vector<unsigned>::iterator itor = it->second.begin(); itor != it->second.end(); itor++)
		{
			getFriendReq->add_othuid(*itor);
		}
		BMI->BattleConnectNoReplyByZoneID(it->first, getFriendReq);
	}


	return LMI->sendMsg(uid, resp) ? 0 : R_ERROR;
}

int LogicFriendManager::Process(ProtoFriend::CSGetFriendHelpInfoReq* req)
{
	ProtoFriend::CSGetFriendHelpInfoResp *resp = new ProtoFriend::CSGetFriendHelpInfoResp;
	for(int i = 0; i < req->othuid_size(); i++)
	{
		if(LogicUserManager::Instance()->IsUserNeedHelp(req->othuid(i)))
		{
			resp->add_othuid(req->othuid(i));
		}
	}
	return LMI->sendMsg(req->myuid(), resp) ? 0 : R_ERROR;
}



int LogicFriendManager::Process(unsigned uid, ProtoFriend::ConcernReq* req)
{
	unsigned othuid = req->othuid();

	if(CMI->IsNeedConnectByUID(othuid))
	{
		//--------------关注对方
		//判断对方是否已在你的关注列表中
		if (DataConcernManager::Instance()->IsExistItem(uid, othuid))
		{
			error_log("user already in concern list. uid=%u,othuid=%u", uid, othuid);
			throw runtime_error("user_already_concerned");
		}

		//对方uid参数验证
		if(!IsValidUid(othuid))
		{
			error_log("othuid is invalid. uid=%u,othuid=%u", uid, othuid);
			throw runtime_error("param_uid_invalid");
		}

		//获取好友配置
		DBCUserBaseWrap userwrap(uid);
		const ConfigFriend::LevelNums & friendcfg = FriendCfgWrap().GetLevelCfgByLevel(userwrap.Obj().viplevel);

		//判断好友数量是否达到上限
		vector<unsigned> indexs;
		DataConcernManager::Instance()->GetIndexs(uid, indexs);

		if (indexs.size() >= friendcfg.concern_nums())
		{
			error_log("concern nums max. uid=%u,othuid=%u", uid, othuid);
			throw runtime_error("concern_num_max");
		}
		//添加关注
		DataConcernManager::Instance()->GetData(uid, othuid);

		//-------------写入对方的粉丝
		ProtoFriend::CSConcernReq* m = new ProtoFriend::CSConcernReq;
		m->set_othuid(othuid);
		m->set_myuid(uid);
		int ret = BMI->BattleConnectNoReplyByUID(othuid, m);
		AddConcernDyInfoOverServer(uid,othuid);	//跨服访问添加动态消息
		return ret;
	}

	ProtoFriend::ConcernResp* resp = new ProtoFriend::ConcernResp;
	try{
		Concern(uid, othuid, resp);
		AddConcernDyInfo(uid,othuid);			//同服访问添加动态消息
	}catch(std::exception &e)
	{
		delete resp;
		error_log("failed:%s",e.what());
		throw std::runtime_error(e.what());
		return R_ERROR;
	}

	return LMI->sendMsg(uid,resp)?0:R_ERROR;
}

int LogicFriendManager::Process(ProtoFriend::CSConcernReq* req)
{
	unsigned myuid = req->othuid();
	unsigned othuid = req->myuid();

	ProtoFriend::CSConcernResp* folkmsg = new ProtoFriend::CSConcernResp;
	ProtoFriend::FolkCPP *folkcpp =  folkmsg->mutable_folkresp()->mutable_concern()->mutable_concernfolk();
	folkcpp->set_folkuid(myuid);

	//写入对方的粉丝
	//判断当前用户是否已在对方的粉丝列表中
	if (!DataFansManager::Instance()->IsExistItem(myuid, othuid))
	{
		//不在对方的粉丝列表，则添加
		//判断对方的粉丝列表是否已达上限
		vector<unsigned> indexs;
		indexs.clear();
		DataFansManager::Instance()->GetIndexs(myuid, indexs);
		DBCUserBaseWrap othuserwrap(myuid);
		const ConfigFriend::LevelNums & othfriendcfg = FriendCfgWrap().GetLevelCfgByLevel(othuserwrap.Obj().viplevel);

		if (indexs.size() >= othfriendcfg.fans_num())
		{
			error_log("other's fans nums max. myuid=%u,othuid=%u", othuid, myuid);
		}
		else
		{
			//未达上限，可以添加
			DataFansManager::Instance()->GetData(myuid, othuid);

			//判断对方是否在线，如果在线，则发送推送
			if (UserManager::Instance()->IsOnline(myuid))
			{
				//在线，则推送
				ProtoFriend::FansPushReq * pushmsg = new ProtoFriend::FansPushReq;
				ProtoFriend::FolkCPP *folkcpp1 = pushmsg->mutable_fan()->mutable_fan();
				folkcpp1->set_folkuid(myuid);

				//推送
				LogicManager::Instance()->sendMsg(myuid, pushmsg);
			}
		}
	}
	folkmsg->set_myuid(req->myuid());
	return BMI->BattleConnectNoReplyByUID(othuid, folkmsg);
}

int LogicFriendManager::Process(ProtoFriend::CSConcernResp* req)
{
	return LMI->sendMsg(req->myuid(),req->mutable_folkresp(),false)?0:R_ERROR;
}



int LogicFriendManager::Concern(unsigned uid, unsigned othuid, ProtoFriend::ConcernResp * resp)
{
	//关注对方
	//判断对方是否已在你的关注列表中
	if (DataConcernManager::Instance()->IsExistItem(uid, othuid))
	{
		error_log("user already in concern list. uid=%u,othuid=%u", uid, othuid);
		throw runtime_error("user_already_concerned");
	}

	//对方uid参数验证
	if(!IsValidUid(othuid))
	{
		error_log("othuid is invalid. uid=%u,othuid=%u", uid, othuid);
		throw runtime_error("param_uid_invalid");
	}

	//获取好友配置
	DBCUserBaseWrap userwrap(uid);
	const ConfigFriend::LevelNums & friendcfg = FriendCfgWrap().GetLevelCfgByLevel(userwrap.Obj().viplevel);

	//判断好友数量是否达到上限
	vector<unsigned> indexs;
	DataConcernManager::Instance()->GetIndexs(uid, indexs);

	if (indexs.size() >= friendcfg.concern_nums())
	{
		error_log("concern nums max. uid=%u,othuid=%u", uid, othuid);
		throw runtime_error("concern_num_max");
	}

	//添加关注
	DataConcernManager::Instance()->GetData(uid, othuid);
	ProtoFriend::FolkCPP *folkcpp = resp->mutable_concern()->mutable_concernfolk();
	folkcpp->set_folkuid(othuid);

	//写入对方的粉丝
	//判断当前用户是否已在对方的粉丝列表中
	if (!DataFansManager::Instance()->IsExistItem(othuid, uid))
	{
		//不在对方的粉丝列表，则添加
		//判断对方的粉丝列表是否已达上限
		indexs.clear();
		DataFansManager::Instance()->GetIndexs(othuid, indexs);
		DBCUserBaseWrap othuserwrap(othuid);
		const ConfigFriend::LevelNums & othfriendcfg = FriendCfgWrap().GetLevelCfgByLevel(othuserwrap.Obj().viplevel);

		if (indexs.size() >= othfriendcfg.fans_num())
		{
			error_log("other's fans nums max. uid=%u,othuid=%u", uid, othuid);
		}
		else
		{
			//未达上限，可以添加
			DataFansManager::Instance()->GetData(othuid, uid);

			//判断对方是否在线，如果在线，则发送推送
			if (UserManager::Instance()->IsOnline(uid))
			{
				//在线，则推送
				ProtoFriend::FansPushReq * pushmsg = new ProtoFriend::FansPushReq;

				ProtoFriend::FolkCPP *folkcpp1 =  pushmsg->mutable_fan()->mutable_fan();
				folkcpp1->set_folkuid(uid);

				//推送
				LogicManager::Instance()->sendMsg(othuid, pushmsg);
			}
		}
	}

	//将关注好友添加到任务列表中
	LogicTaskManager::Instance()->AddTaskData(uid,task_of_Focus_friend,1);

	//debug_log("user:[uid] fork other uid:[%u]", uid, othuid);

	return 0;
}

int LogicFriendManager::Process(unsigned uid, ProtoFriend::CancelConcernReq* req)
{
	unsigned othuid = req->othuid();
	if(CMI->IsNeedConnectByUID(othuid))
	{
		//判断对方是否已在你的关注列表中
		if (!DataConcernManager::Instance()->IsExistItem(uid, othuid))
		{
			error_log("user not in concern list. uid=%u,othuid=%u", uid, othuid);
			throw runtime_error("user_not_concerned");
		}
		//从关注列表中删除
		DataConcernManager::Instance()->DelItem(uid, othuid);

		//发送请求、将自己从对方的粉丝列表删除
		ProtoFriend::CSCancelConcernReq* m = new ProtoFriend::CSCancelConcernReq;
		m->set_othuid(othuid);
		m->set_myuid(uid);
		return BMI->BattleConnectNoReplyByUID(othuid, m);
	}

	ProtoFriend::CancelConcernResp* resp = new ProtoFriend::CancelConcernResp;
	try{
		CancelConcern(uid, othuid, resp);
	}catch(const std::exception &e)
	{
		delete resp;
		error_log("failed:%s",e.what());
		return R_ERROR;
	}
	return LMI->sendMsg(uid,resp)?0:R_ERROR;
}

int LogicFriendManager::Process(ProtoFriend::CSCancelConcernReq* req)
{
	unsigned myuid = req->othuid();
	unsigned othuid = req->myuid();

	//从粉丝列表中删除
	//判断用户是否在线
	OffUserSaveControl offuserctl(myuid);

	//到这里，内存中肯定有对方uid的数据
	if (DataFansManager::Instance()->IsExistItem(myuid, othuid))
	{
		//存在该uid的粉丝
		DataFansManager::Instance()->DelItem(myuid, othuid);

		//如果在线
		if (UserManager::Instance()->IsOnline(myuid))
		{
			//脱粉的推送
			ProtoFriend::StripFansPushReq * pushmsg = new ProtoFriend::StripFansPushReq;
			pushmsg->set_deluid(othuid);

			//推送
			LogicManager::Instance()->sendMsg(myuid, pushmsg);
		}
	}

	//返回处理消息
	ProtoFriend::CSCancelConcernResp* resp = new ProtoFriend::CSCancelConcernResp;
	resp->set_myuid(req->myuid());
	resp->mutable_resp()->set_deluid(req->othuid());
	return BMI->BattleConnectNoReplyByUID(othuid, resp);
}

int LogicFriendManager::Process(ProtoFriend::CSCancelConcernResp* req)
{
	return LMI->sendMsg(req->myuid(),req->mutable_resp(),false)?0:R_ERROR;
}


int LogicFriendManager::CancelConcern(unsigned uid, unsigned othuid, ProtoFriend::CancelConcernResp * resp)
{
	//判断对方是否已在你的关注列表中
	if (!DataConcernManager::Instance()->IsExistItem(uid, othuid))
	{
		error_log("user not in concern list. uid=%u,othuid=%u", uid, othuid);
		throw runtime_error("user_not_concerned");
	}

	//从关注列表中删除
	DataConcernManager::Instance()->DelItem(uid, othuid);

	//从粉丝列表中删除
	//判断用户是否在线
	OffUserSaveControl offuserctl(othuid);

	//到这里，内存中肯定有对方uid的数据
	if (DataFansManager::Instance()->IsExistItem(othuid, uid))
	{
		//存在该uid的粉丝
		DataFansManager::Instance()->DelItem(othuid, uid);

		//如果在线
		if (UserManager::Instance()->IsOnline(uid))
		{
			//脱粉的推送
			ProtoFriend::StripFansPushReq * pushmsg = new ProtoFriend::StripFansPushReq;
			pushmsg->set_deluid(uid);

			//推送
			LogicManager::Instance()->sendMsg(othuid, pushmsg);
		}
	}

	//设置删除的关注对象的uid
	resp->set_deluid(othuid);

	return 0;
}

int LogicFriendManager::Process(unsigned uid, ProtoFriend::RemoveFansReq *req, ProtoFriend::RemoveFansResp* resp)
{
	unsigned othuid = req->deluid();

	RemoveFans(uid, othuid, resp);

	return 0;
}

int LogicFriendManager::RemoveFans(unsigned uid, unsigned othuid, ProtoFriend::RemoveFansResp * resp)
{
	//判断是否是粉丝
	if (!DataFansManager::Instance()->IsExistItem(uid, othuid))
	{
		error_log("user aren't your fans. uid=%u,othuid=%u", uid, othuid);
		throw runtime_error("not_fans");
	}

	//删除粉丝
	DataFansManager::Instance()->DelItem(uid, othuid);

	resp->set_deluid(othuid);

	return 0;
}



bool LogicFriendManager::AddConcernDyInfo(unsigned uid,unsigned other_uid)
{
	//uid:关注者,other_uid:被关注者,关注好友会让被关注者增加一条被关注动态
	DynamicInfoAttach *pattach = new DynamicInfoAttach;
	pattach->op_uid = uid;
	if(LogicDynamicInfoManager::Instance()->ProduceOneDyInfo(other_uid,TYPE_DY_CONCERN,pattach))
	{
		return true;
	}
	return false;
}

bool LogicFriendManager::AddConcernDyInfoOverServer(unsigned uid,unsigned other_uid)
{
	ProtoDynamicInfo::RequestOtherUserMakeDy * msg = new ProtoDynamicInfo::RequestOtherUserMakeDy;
	string ret;
	msg->set_myuid(uid);
	msg->set_othuid(other_uid);
	msg->set_typeid_(TYPE_DY_CONCERN);
	msg->set_productid(0);
	msg->set_coin(0);
	msg->set_words(ret);

	return BMI->BattleConnectNoReplyByUID(other_uid, msg);
}
