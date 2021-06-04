#include "BattleServer.h"

int LogicFriendWorkerManager::Process(unsigned uid, ProtoFriendWorker::SetFriendWorkerReq* req)
{
	DBCUserBaseWrap userwrap(uid);
	//校验
	unsigned othuid = req->othuid();
	if(!IsValidUid(othuid))
	{
		throw std::runtime_error("uid_param_error");
	}

	unsigned propsid = 0;
	unsigned source_type = req->source(0);
	unsigned source_flag = req->source(1);
	if(req->has_propsid())
	{
		propsid = req->propsid();
	}

	if(userwrap.Obj().inviteuid == 0)
	{
		if(CMI->IsNeedConnectByUID(othuid))
		{
			ProtoFriendWorker::CSSetFriendWorkerReq* req = new ProtoFriendWorker::CSSetFriendWorkerReq;
			req->set_othuid(othuid);
			req->set_myuid(uid);
			req->set_propisid(propsid);
			req->set_sourceflag(source_flag);
			req->set_sourcetype(source_type);
			int ret = BMI->BattleConnectNoReplyByUID(othuid, req);
			return ret;
		}


		ProtoFriendWorker::SetFriendWorkerResp* resp = new ProtoFriendWorker::SetFriendWorkerResp;
		//1.更新对方数据
		unsigned add_result = 0;
		try{
			add_result = AddFriendWorker(othuid,uid,propsid,source_type,source_flag);
		}catch(const std::exception& e){
			delete resp;
			error_log("set %s", e.what());
			throw std::runtime_error(e.what());
		}

		if(0 == add_result) {
			//2.更新自己数据
			userwrap.Obj().inviteuid = othuid;
			DataBase &database = BaseManager::Instance()->Get(uid);
			BaseManager::Instance()->UpdateDatabase(database);

			if(othuid > 0)
			{
				USER_LOG("[invite]uid=%u,inviteuid=%u", uid, othuid);
			}

			//3.设置返回
			resp->set_status(result_set_friendworker_success);
			return LMI->sendMsg(uid, resp)?0:R_ERROR;
		}else {
			resp->set_status(result_set_friendworker_error);
			return LMI->sendMsg(uid, resp)?0:R_ERROR;
		}
	}
	else
	{
		ProtoFriendWorker::SetFriendWorkerResp* resp = new ProtoFriendWorker::SetFriendWorkerResp;
		resp->set_status(result_set_friendworker_repeated);
		return LMI->sendMsg(uid, resp)?0:R_ERROR;
	}
	return 0;
}

int LogicFriendWorkerManager::Process(ProtoFriendWorker::CSSetFriendWorkerReq* req)
{
	unsigned add_result = 0;
	try{
		add_result = AddFriendWorker(req->othuid(),req->myuid(),req->propisid(),req->sourcetype(),req->sourceflag());
	}catch(const std::exception& e)
	{
		throw std::runtime_error(e.what());
	}

	ProtoFriendWorker::CSSetFriendWorkerResp* resp = new ProtoFriendWorker::CSSetFriendWorkerResp;
	resp->set_myuid(req->myuid());
	resp->set_othuid(req->othuid());
	resp->set_stauts(add_result);
	BMI->BattleConnectNoReplyByUID(req->myuid(), resp);

	return 0;
}

int LogicFriendWorkerManager::Process(ProtoFriendWorker::CSSetFriendWorkerResp* req)
{
	unsigned myuid = req->myuid();
	unsigned othuid = req->othuid();
	unsigned status = req->stauts();

	if(status == 0) {
		DBCUserBaseWrap userwrap(myuid);
		//1.更新自己数据
		userwrap.Obj().inviteuid = othuid;
		DataBase &database = BaseManager::Instance()->Get(myuid);
		BaseManager::Instance()->UpdateDatabase(database);
		//2.设置返回
		ProtoFriendWorker::SetFriendWorkerResp* resp = new ProtoFriendWorker::SetFriendWorkerResp;
		resp->set_status(result_set_friendworker_success);
		return LMI->sendMsg(myuid, resp)?0:R_ERROR;
	}else {
		ProtoFriendWorker::SetFriendWorkerResp* resp = new ProtoFriendWorker::SetFriendWorkerResp;
		resp->set_status(result_set_friendworker_error);
		return LMI->sendMsg(myuid, resp)?0:R_ERROR;
	}
}

int LogicFriendWorkerManager::AddFriendWorker(unsigned uid, unsigned inviteduid,unsigned propsid,unsigned sourceType,unsigned sourceFlag)
{
	int Ret = UserManager::Instance()->LoadArchives(uid);
	if(Ret == 0)
	{
		//1.添加长工数据
		DataFriendWorker & friendworker = DataFriendWorkerManager::Instance()->GetData(uid,inviteduid);
		friendworker.invite_ts = Time::GetGlobalTime();
		DataFriendWorkerManager::Instance()->UpdateItem(friendworker);

		//2.添加物品
		if(propsid != 0)
		{
			const ConfigFriendWorker::FriendWorkerCPP & workercfg = ConfigManager::Instance()->friendworker.m_config.worker();

			CommonGiftConfig::CommonModifyItem common;
			CommonGiftConfig::PropsItem *propsitem = common.add_props();
			propsitem->set_id(propsid);
			propsitem->set_count(workercfg.invite_friend_reward_item_cnt());

			ProtoFriendWorker::PushInviteReardMsg *msg = new ProtoFriendWorker::PushInviteReardMsg;
			LogicUserManager::Instance()->CommonProcess(uid,common,"invite_reward",msg->mutable_commons());
			if(UserManager::Instance()->IsOnline(uid))
			{
				LMI->sendMsg(uid,msg);
			}
			else{
				delete msg;
			}
		}
		if(sourceType != 0)
		{
			if(1 == sourceType)
			{
				//有效分享扩充商店格子
				LogicShopManager::Instance()->InviteUnlockShopShelf(uid);
			}
			else if(2 == sourceType)
			{
				//有效分享解锁宠物
				LogicPetManager::Instance()->InviteUnlockPet(uid);
			}
			else if(3 == sourceType)
			{
				//有效分享解锁生产设备格子
			}
		}
		return 0;

	}else {
		error_log("load_data_error.uid =%u,othuid=%u",uid,inviteduid);
		return R_ERROR;
	}
}

int LogicFriendWorkerManager::Process(unsigned uid, ProtoFriendWorker::GetWorkerSpeedUpReq* req, ProtoFriendWorker::GetWorkerSpeedUpResp* resp)
{
	vector<unsigned>results;
	results.clear();
	DataFriendWorkerManager::Instance()->GetIndexs(uid,results);

	for(int i = 0; i < results.size(); i++)
	{
		DataFriendWorker & friendworker = DataFriendWorkerManager::Instance()->GetDataByIndex(results[i]);
		ProtoFriendWorker::FriendWorkerCPP *msg = resp->add_friendworker();
		friendworker.SetMessage(msg);
	}
	return 0;
}

int LogicFriendWorkerManager::Process(unsigned uid, ProtoFriendWorker::SelectWorkerReq* req, ProtoFriendWorker::SelectWorkerResp* resp)
{
	unsigned workeruid = req->workeruid();
	unsigned pos = req->pos();

	const ConfigFriendWorker::FriendWorkerCPP & workercfg = ConfigManager::Instance()->friendworker.m_config.worker();
	//校验
	//1.校验pos是否合法
	vector<unsigned>results;
	results.clear();
	DataFriendWorkerManager::Instance()->GetIndexs(uid,results);
	if(pos < 1 || pos > workercfg.speedup_solt_max() || pos > results.size())
	{
		throw std::runtime_error("pos_param_error");
	}
	//2.校验长工是否合法
	bool is_exsit = DataFriendWorkerManager::Instance()->IsExistItem(uid,workeruid);
	if(!is_exsit)
	{
		throw std::runtime_error("worker_is_not_exsit");
	}
	DataFriendWorker & worker = DataFriendWorkerManager::Instance()->GetData(uid,workeruid);
	if(worker.pos != 0 || worker.endts != 0)
	{
		throw std::runtime_error("worker_is_working");
	}

	//校验通过、进行相关处理
	worker.pos = pos;
	worker.endts = workercfg.worker_time() + Time::GetGlobalTime();
	DataFriendWorkerManager::Instance()->UpdateItem(worker);
	worker.SetMessage(resp->mutable_friendworker());
	return 0;
}

int LogicFriendWorkerManager::Process(unsigned uid, ProtoFriendWorker::ThanksWorkerReq* req, ProtoFriendWorker::ThanksWorkerResp* resp)
{
	unsigned workeruid = req->workeruid();

	//校验
	//1.校验长工是否合法
	bool is_exsit = DataFriendWorkerManager::Instance()->IsExistItem(uid,workeruid);
	if(!is_exsit)
	{
		throw std::runtime_error("worker_is_not_exsit");
	}
	//2.校验ts是否正确
	DataFriendWorker & worker = DataFriendWorkerManager::Instance()->GetData(uid,workeruid);
	if(worker.endts > Time::GetGlobalTime())
	{
		throw std::runtime_error("worker_is_working");
	}
	//3.校验pos是否合法
	const ConfigFriendWorker::FriendWorkerCPP & workercfg = ConfigManager::Instance()->friendworker.m_config.worker();
	if(worker.pos < 1 || worker.pos > workercfg.speedup_solt_max() )
	{
		throw std::runtime_error("pos_param_error");
	}

	//校验通过、进行相关处理
	//1.奖励友情值
	DBCUserBaseWrap userwrap(uid);
	string content;
	//String::Format(content, ConfigManager::Instance()->language.m_config.worker_reward().c_str(), userwrap.Obj().name,workercfg.reward_friendly_value());
	String::Format(content,"{\"t\":\"A001\",\"c\":[\"%s\",\"%d\"]}",userwrap.Obj().name,workercfg.reward_friendly_value());
	CommonGiftConfig::CommonModifyItem common;
	CommonGiftConfig::BaseItem * baseitem = common.mutable_based();
	baseitem->set_friend_(workercfg.reward_friendly_value());
	LogicSysMailManager::Instance()->DoReward(workeruid,content,common);
	//2.更新数据
	worker.endts = 0;
	worker.pos = 0;
	DataFriendWorkerManager::Instance()->UpdateItem(worker);
	worker.SetMessage(resp->mutable_friendworker());
	return 0;
}

float LogicFriendWorkerManager::GetCropsSpeedUpPercent(unsigned uid)
{
	float percent = 0;
	const ConfigFriendWorker::FriendWorkerSpeedUpCPP & workerspeedupcfg = ConfigManager::Instance()->friendworker.m_config.worker_speed_up();

	//校验是否有农地加速
	vector<unsigned>results;
	results.clear();
	DataFriendWorkerManager::Instance()->GetIndexs(uid,results);
	for(int i = 0; i < results.size(); i++)
	{
		DataFriendWorker & worker = DataFriendWorkerManager::Instance()->GetDataByIndex(results[i]);
		if(worker.pos == pos_of_crops_speedup_slot && worker.endts >= Time::GetGlobalTime())
		{
			percent = (float)workerspeedupcfg.crops_speed_up_percent() / 100;
			break;
		}
	}
	return percent;
}

float LogicFriendWorkerManager::GetOrderReardPercent(unsigned uid)
{
	float percent = 0;
	const ConfigFriendWorker::FriendWorkerSpeedUpCPP & workerspeedupcfg = ConfigManager::Instance()->friendworker.m_config.worker_speed_up();

	vector<unsigned>results;
	results.clear();
	DataFriendWorkerManager::Instance()->GetIndexs(uid,results);
	for(int i = 0; i < results.size(); i++)
	{
		DataFriendWorker & worker = DataFriendWorkerManager::Instance()->GetDataByIndex(results[i]);
		if(worker.pos == pos_of_order_reward_slot && worker.endts >= Time::GetGlobalTime())
		{
			percent = (float)workerspeedupcfg.order_reward_percent() / 100;
			break;
		}
	}
	return percent;
}

float LogicFriendWorkerManager::GetAnimalSpeedUpPercent(unsigned uid)
{
	float percent = 0;
	const ConfigFriendWorker::FriendWorkerSpeedUpCPP & workerspeedupcfg = ConfigManager::Instance()->friendworker.m_config.worker_speed_up();

	vector<unsigned>results;
	results.clear();
	DataFriendWorkerManager::Instance()->GetIndexs(uid,results);
	for(int i = 0; i < results.size(); i++)
	{
		DataFriendWorker & worker = DataFriendWorkerManager::Instance()->GetDataByIndex(results[i]);
		if(worker.pos == pos_of_animal_speedup_slot && worker.endts >= Time::GetGlobalTime())
		{
			percent = (float)workerspeedupcfg.animal_speed_up_percent() / 100;
			break;
		}
	}
	return percent;
}

float LogicFriendWorkerManager::GetShipRewardPercent(unsigned uid)
{
	float percent = 0;
	const ConfigFriendWorker::FriendWorkerSpeedUpCPP & workerspeedupcfg = ConfigManager::Instance()->friendworker.m_config.worker_speed_up();

	vector<unsigned>results;
	results.clear();
	DataFriendWorkerManager::Instance()->GetIndexs(uid,results);
	for(int i = 0; i < results.size(); i++)
	{
		DataFriendWorker & worker = DataFriendWorkerManager::Instance()->GetDataByIndex(results[i]);
		if(worker.pos == pos_of_ship_reward_slot && worker.endts >= Time::GetGlobalTime())
		{
			percent = (float)workerspeedupcfg.ship_speed_up_percent() / 100;
			break;
		}
	}
	return percent;
}

float LogicFriendWorkerManager::GetProductSpeedUpPercent(unsigned uid)
{
	float percent = 0;
	const ConfigFriendWorker::FriendWorkerSpeedUpCPP & workerspeedupcfg = ConfigManager::Instance()->friendworker.m_config.worker_speed_up();

	vector<unsigned>results;
	results.clear();
	DataFriendWorkerManager::Instance()->GetIndexs(uid,results);
	for(int i = 0; i < results.size(); i++)
	{
		DataFriendWorker & worker = DataFriendWorkerManager::Instance()->GetDataByIndex(results[i]);
		if(worker.pos == pos_of_product_speedup_slot && worker.endts >= Time::GetGlobalTime())
		{
			percent = (float)workerspeedupcfg.product_speed_up_percent() / 100;
			break;
		}
	}
	return percent;
}
