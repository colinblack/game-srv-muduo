#include "LogicShippingManager.h"
#include "ServerInc.h"

void DataShippingRoutine::GetPriceAndATime(unsigned ud, int & cash, int & diffts, int &type)
{
	//计算立即召唤需要的时间
	//如何区分解锁和提前到达呢?
	//根据状态
	DataShipping & dataship = DataShippingManager::Instance()->GetData(uid_);
	unsigned nowts = Time::GetGlobalTime();

	diffts = endts_ > nowts ? endts_ - nowts : 0;
	const ConfigShipping::ShippingItem & shipcfg = ShippingCfgWrap().GetShippingCfg();

	//判断船此时的状态
	if (LogicShippingManager::status_unlock == dataship.status)
	{
		//根据diffts的比例，计算花的钻石数目
		cash = ceil( (static_cast<double>(diffts)/shipcfg.unlock_time()) * shipcfg.unlock_speed_price());
	}
	else if (LogicShippingManager::status_wait_arrive == dataship.status)
	{
		cash = ceil( (static_cast<double>(diffts)/shipcfg.prepare_time()) * shipcfg.speed_prepare_price());
	}
	else
	{
		error_log("shipping status error, cann't speed up. uid=%u", uid_);
		throw runtime_error("status_error");
	}
}

void DataShippingRoutine::SingleRoutineEnd(unsigned ud, ProtoPush::PushBuildingsCPP * msg)
{
	//解锁/到港/离港
	//获取航运数据
	DataShipping & dataship = DataShippingManager::Instance()->GetData(uid_);

	//判断船此时的状态
	if (LogicShippingManager::status_unlock == dataship.status)
	{
		//解锁完毕，更改状态
		dataship.status = LogicShippingManager::status_unveil; //等待揭幕
		dataship.unlock_endts = 0;

		DataShippingManager::Instance()->UpdateItem(dataship);
		dataship.SetMessage(msg->mutable_shipping());
	}
	else if (LogicShippingManager::status_wait_arrive == dataship.status)
	{
		dataship.status = LogicShippingManager::status_packing; //装货中

		//设置船的到港时间
		dataship.arrive_ts = Time::GetGlobalTime();

		DataShippingManager::Instance()->UpdateItem(dataship);
		dataship.SetMessage(msg->mutable_shipping());

		//获取航运配置
		const ConfigShipping::ShippingItem & shipcfg = ShippingCfgWrap().GetShippingCfg();
		unsigned endts = dataship.arrive_ts + shipcfg.pack_time();
		//加入到定时任务中
		debug_log("ship_join_routine uid=%u endts=%u ats=%u status=%u", uid_, endts, dataship.arrive_ts, dataship.status);
		LogicQueueManager::Instance()->JoinRoutine<DataShippingRoutine>(uid_, endts, routine_type_ship, 0);
	}
	else if (LogicShippingManager::status_packing == dataship.status)
	{
		//船的自动离港
		LogicShippingManager::Instance()->AutoLeaveDock(dataship, msg);
	}
}

LogicShippingManager::LogicShippingManager()
{
	userActId = e_Activity_UserData_1;
}

int LogicShippingManager::CheckLogin(unsigned uid)
{
	//修复错误数据
	vector<unsigned>indexs;
	DataShippingboxManager::Instance()->GetIndexs(uid,indexs);
	for(int i = 0; i < indexs.size(); i++)
	{
		DataShippingbox & box = DataShippingboxManager::Instance()->GetDataByIndex(indexs[i]);
		if(box.propsid == 60032 && box.count == 0)
			box.count = 1;
		DataShippingboxManager::Instance()->UpdateItem(box);
	}

	//加载数据
	DataShippingManager::Instance()->LoadBuffer(uid);

	//判断是否存在航运数据
	if (!DataShippingManager::Instance()->IsExist(uid))
	{
		return 0;
	}

	DataShipping & dataship = DataShippingManager::Instance()->GetData(uid);

	unsigned endts = 0;

	//判断解锁时间是否大于0
	if (dataship.unlock_endts > 0)
	{
		endts = dataship.unlock_endts;
	}
	else if (status_wait_arrive == dataship.status)
	{
		endts = dataship.arrive_ts;
	}
	else if (status_packing == dataship.status)
	{
		//获取航运配置
		const ConfigShipping::ShippingItem & shipcfg = ShippingCfgWrap().GetShippingCfg();
		endts = dataship.arrive_ts + shipcfg.pack_time();
	}

	if (endts > 0)
	{
		//加入到定时任务中
		debug_log("ship_join_routine uid=%u endts=%u ats=%u status=%u", uid, endts, dataship.arrive_ts, dataship.status);
		LogicQueueManager::Instance()->JoinRoutine<DataShippingRoutine>(uid, endts, routine_type_ship, 0);
	}

	return 0;
}
// 设置船帮助状态
int LogicShippingManager::SetShippingboxAidStatus(DataShippingbox& box, int8_t status)
{
	box.aid_status = status;
	if(box.aid_status == aid_stauts_public || box.aid_status == aid_status_commercial
		|| box.aid_status == aid_stauts_pulic_done || box.aid_status == aid_status_commercial_done)
	{
		LogicAllianceManager::Instance()->UpdateMemberHelpTs(box.uid);
	}
	return 0;
}

int LogicShippingManager::Process(unsigned uid, ProtoShipping::UnlockDockReq* req, ProtoShipping::UnlockDockResp* resp)
{
	UnlockDock(uid, resp);

	return 0;
}

int LogicShippingManager::UnlockDock(unsigned uid, ProtoShipping::UnlockDockResp * resp)
{
	//设置航运信息
	DataShipping & dataship = DataShippingManager::Instance()->GetData(uid);

	//判断航运是否已经解锁
	if (dataship.status != status_init)
	{
		error_log("unlock request already sent. uid=%u", uid);
		throw runtime_error("unlock_req_sent");
	}

	//获取航运配置
	const ConfigShipping::ShippingItem & shipcfg = ShippingCfgWrap().GetShippingCfg();

	//判断用户等级是否满足要求
	if(BaseManager::Instance()->Get(uid).level < shipcfg.unlock_level())
	{
		error_log("user level not enough. uid=%u", uid);
		throw runtime_error("user_level_not_enough");
	}

	//解锁消耗
	LogicUserManager::Instance()->CommonProcess(uid, shipcfg.unlock_cost(), "UnlockDock", resp->mutable_commons());

	unsigned endts = Time::GetGlobalTime() + shipcfg.unlock_time();
	dataship.unlock_endts = endts;
	dataship.status = status_unlock;

	DataShippingManager::Instance()->UpdateItem(dataship);
	dataship.SetMessage(resp->mutable_shipping());

	//加入到定时任务中
	LogicQueueManager::Instance()->JoinRoutine<DataShippingRoutine>(uid, endts, routine_type_ship, 0);

	return 0;
}

int LogicShippingManager::Process(unsigned uid, ProtoShipping::UnveilDockReq* req, ProtoShipping::UnveilDockResp* resp)
{
	UnveilDock(uid, resp);

	return 0;
}

int LogicShippingManager::UnveilDock(unsigned uid, ProtoShipping::UnveilDockResp * resp)
{
	//判断航运是否存在
	if (!DataShippingManager::Instance()->IsExist(uid))
	{
		error_log("shipping not exist. uid=%u", uid);
		throw runtime_error("shipping_not_exist");
	}

	//判断航运是否处于待揭幕状态
	DataShipping & dataship = DataShippingManager::Instance()->GetData(uid);

	if (status_unveil != dataship.status)
	{
		error_log("shipping status error. uid=%u", uid);
		throw runtime_error("shipping_status_error");
	}

	//获取航运配置
	const ConfigShipping::ShippingItem & shipcfg = ShippingCfgWrap().GetShippingCfg();

	unsigned endts = Time::GetGlobalTime() + shipcfg.pack_time();

	//揭幕，然后分配货箱
	DistributeBoxes(dataship);

	//加入到定时任务中
	LogicQueueManager::Instance()->JoinRoutine<DataShippingRoutine>(uid, endts, routine_type_ship, 0);

	//更新航运状态
	dataship.arrive_ts = Time::GetGlobalTime();
	dataship.status = status_packing;

	DataShippingManager::Instance()->UpdateItem(dataship);

	//设置前端消息
	dataship.SetMessage(resp->mutable_shipping());

	DataShippingboxManager::Instance()->FullSpecialMessage(uid, resp->mutable_shipboxes());

	return 0;
}

int LogicShippingManager::GetShippingItem(unsigned level, int boxnum, vector<unsigned> & items, vector<unsigned> & count)
{
	//获取用户等级下标
	const ConfigShipping::ShippingItem & shipcfg = ShippingCfgWrap().GetShippingCfg();
	int level_index = 0;

	for(int i = shipcfg.level_items_size() - 1; i >= 0; --i)
	{
		if (level >= shipcfg.level_items(i).level())
		{
			level_index = i;
			break;
		}
	}

	//遍历该等级库上的所有物品
	map<unsigned, unsigned> itemIndexs;
	vector<unsigned> allitem;

	for(int i = 0; i < shipcfg.level_items(level_index).items_size(); ++i)
	{
		//遍历物品库
		unsigned itemid = shipcfg.level_items(level_index).items(i).id();

		//获取item配置
		unsigned unlocklevel = ItemCfgWrap().GetPropsItem(itemid).unlock_level();

		if (unlocklevel > level)
		{
			continue;
		}

		allitem.push_back(itemid);
		itemIndexs[itemid] = i;
	}

	//打乱产品集合
	random_shuffle(allitem.begin(), allitem.end());

	//抽取n个产品
	for(int i = 0; i < shipcfg.item_kind_num() && i < allitem.size(); ++i)
	{
		unsigned itemid = allitem[i];

		unsigned itemindex = itemIndexs[itemid];

		//判断数量配置是否正确
		if (boxnum > shipcfg.level_items(level_index).items(itemindex).count_size())
		{
			error_log("count config error. levelindex=%u,itemindex=%u, boxnum=%u", level_index, itemindex, boxnum);
			throw runtime_error("config_error");
		}

		int singlecount = shipcfg.level_items(level_index).items(itemindex).count(boxnum - 1);

		items.push_back(itemid);
		count.push_back(singlecount);
	}

	return 0;
}

int LogicShippingManager::Process(unsigned uid, ProtoShipping::PackBoxReq* req, ProtoShipping::PackBoxResp* resp)
{
	unsigned boxid = req->boxid();

	PackBox(uid, req->type(),boxid, resp);

	return 0;
}

int LogicShippingManager::PackBox(unsigned uid, unsigned type,unsigned boxid, ProtoShipping::PackBoxResp * resp)
{
	//判断航运是否存在
	if (!DataShippingManager::Instance()->IsExist(uid))
	{
		error_log("shipping not exist. uid=%u", uid);
		throw runtime_error("shipping_not_exist");
	}

	//判断船是否处于装货中
	DataShipping & shipping = DataShippingManager::Instance()->GetData(uid);

	if (shipping.status != status_packing)
	{
		error_log("shipping not exist. uid=%u", uid);
		throw runtime_error("shipping_not_packing");
	}

	//判断航运箱子是否存在
	if (!DataShippingboxManager::Instance()->IsExistItem(uid, boxid))
	{
		error_log("ship box not exist. uid=%u,boxid=%u", uid, boxid);
		throw runtime_error("box_not_exist");
	}

	DataShippingbox & box = DataShippingboxManager::Instance()->GetData(uid, boxid);

	//装箱
	if(type == common_pack_box){
		//普通装箱
		PackBoxUnderlying(uid, type,box, resp->mutable_commons());
	}
	else if(type == view_ad_pack_box) {
		//看广告装箱
		PackBoxUnderlying(uid, type,box);
		resp->set_viewadpackcnt(shipping.view_ad_pack_cnt);
	}
	else {
		throw std::runtime_error("pack_box_type_error");
	}

	unsigned propsid = 0;
	unsigned propscnt = 0;
	FullBoxReward(uid,propsid,propscnt);
//	debug_log("packbox:uid=%u,propsid=%u,propscnt=%u",uid,propsid,propscnt);
	if(propsid != 0 && propscnt != 0)
	{
		resp->mutable_point()->set_propsid(propsid);
		resp->mutable_point()->set_propscnt(propscnt);

		//存储点券信息
		DataGameActivity& activity = DataGameActivityManager::Instance()->GetUserActivity(uid, userActId);
		activity.actdata[e_Activity_UserData_1_index_9] =  propsid;
		activity.actdata[e_Activity_UserData_1_index_10] =  propscnt;
		DataGameActivityManager::Instance()->UpdateActivity(activity);
	}


	box.SetMessage(resp->mutable_boxes());

	return 0;
}

bool LogicShippingManager::IsFullBox(unsigned uid)
{
	bool is_full = false;
	vector<unsigned> boxlist;
	boxlist.clear();
	DataShippingboxManager::Instance()->GetIndexs(uid,boxlist);

	int i = 0;
	for(; i < boxlist.size(); i++)
	{
		DataShippingbox & box = DataShippingboxManager::Instance()->GetDataByIndex(boxlist[i]);
		if(box.propsid != 0 && box.count != 0){
			if(box.status == box_status_empty)
				break;
		}
	}
	if(i == boxlist.size())
		is_full = true;
	return is_full;
}

int LogicShippingManager::FullBoxReward(unsigned uid,unsigned & propsid,unsigned & propscnt)
{
	bool is_full = IsFullBox(uid);
//	debug_log("IsFullBox;uid=%u,is_full=%d",uid,is_full);
	if(is_full)
	{
		const ConfigShipping::Shipping & shipcfg = ConfigManager::Instance()->shipping.m_config;
		vector<unsigned>weights;
		weights.clear();
		for(int i = 0; i < shipcfg.point_size(); i++)
		{
			weights.push_back(shipcfg.point(i).weight());
		}

		//按权重随机出物品
		int target = 0;
		LogicCommonUtil::TurnLuckTable(weights,weights.size(),target);
		if(target >= 0 && target < shipcfg.point_size())
		{
			propsid = shipcfg.point(target).reward().props(0).id();
			propscnt = shipcfg.point(target).reward().props(0).count();
		}

	}
	return 0;
}

int LogicShippingManager::Process(unsigned uid, ProtoShipping::SeekAidReq* req, ProtoShipping::SeekAidResp* resp)
{
	unsigned boxid = req->boxid();
	unsigned type = req->type();

	SeekAid(uid, boxid, type, resp);

	return 0;
}

int LogicShippingManager::SeekAid(unsigned uid, unsigned boxid, unsigned type, ProtoShipping::SeekAidResp * resp)
{
	//判断航运箱子是否存在
	if (!DataShippingboxManager::Instance()->IsExistItem(uid, boxid))
	{
		error_log("ship box not exist. uid=%u,boxid=%u", uid, boxid);
		throw runtime_error("box_not_exist");
	}

	DataShippingbox & box = DataShippingboxManager::Instance()->GetData(uid, boxid);

	//判断箱子是否已发出援助
	if (aid_status_none != box.aid_status)
	{
		error_log("ship box already seek aid. uid=%u,boxid=%u", uid, boxid);
		throw runtime_error("aid_already_seeked");
	}

	DataShipping & dataship = DataShippingManager::Instance()->GetData(uid);
	//获取航运配置
	const ConfigShipping::ShippingItem & shipcfg = ShippingCfgWrap().GetShippingCfg();

	//判断援助类型是否正确，以及次数是否足够
	if (aid_type_public == type)
	{
		//判断次数是否足够
		if (dataship.public_aid_times >= shipcfg.public_aid_limit())
		{
			error_log("public aid times not enough. uid=%u,boxid=%u", uid, boxid);
			throw runtime_error("public_aid_times_not_enough");
		}

		//航运援助次数+1
		dataship.public_aid_times += 1;

		//箱子状态修改
		//box.aid_status = aid_stauts_public;
		LogicShippingManager::Instance()->SetShippingboxAidStatus(box, aid_stauts_public);

	}
	else if (aid_type_commercial == type)
	{
		//判断是否已有公会
		DBCUserBaseWrap userwrap(uid);

		if (0 == userwrap.Obj().alliance_id)
		{
			error_log("user not in alliance. uid=%u", uid);
			throw runtime_error("not_in_alliance");
		}

		//判断公会援助次数是否足够
		if (dataship.commercial_aid_times >= shipcfg.commercial_aid_limit())
		{
			error_log("public aid times not enough. uid=%u,boxid=%u", uid, boxid);
			throw runtime_error("commercial_aid_times_not_enough");
		}

		//航运援助次数+1
		dataship.commercial_aid_times += 1;

		//箱子状态修改
		//box.aid_status = aid_status_commercial;
		LogicShippingManager::Instance()->SetShippingboxAidStatus(box, aid_status_commercial);
	}
	else
	{
		error_log("type param error. uid=%u,boxid=%u,type=%u", uid, boxid, type);
		throw runtime_error("param_error");
	}

	//更新航运
	DataShippingManager::Instance()->UpdateItem(dataship);

	//更新航运箱子
	DataShippingboxManager::Instance()->UpdateItem(box);

	dataship.SetMessage(resp->mutable_shipping());
	box.SetMessage(resp->mutable_boxes());

	//公会援助已满
	if (dataship.commercial_aid_times == shipcfg.commercial_aid_limit())
	{
		AddCAidDyInfo(uid);
	}

	return 0;
}

int LogicShippingManager::Process(unsigned uid, ProtoShipping::OfferAidReq* req)
{
	unsigned othuid = req->othuid();
	unsigned boxid = req->boxid();
	if(CMI->IsNeedConnectByUID(othuid))
	{
		ProtoShipping::OfferAidResp* resp = new ProtoShipping::OfferAidResp;
		unsigned propsid = req->propsid();
		unsigned propscnt = req->propscnt();
		unsigned coin   = req->coin();
		unsigned exp    = req->exp();
		try{
			//--------------一开始进行资源扣除、如果请求在对面校验不通过、则默认此次跟系统做了交易
			//扣除物品
			CommonGiftConfig::CommonModifyItem cfg;
			CommonGiftConfig::PropsItem* propscfg = cfg.add_props();
			propscfg->set_id(propsid);
			int count = propscnt;
			propscfg->set_count(-count);

			//添加金币和经验
			cfg.mutable_based()->set_coin(coin);
			cfg.mutable_based()->set_exp(exp);

			//扣除与奖励一起
			LogicUserManager::Instance()->CommonProcess(uid, cfg, "Shipping_Pack", resp->mutable_commons());

			//---------------------获取自身的航运数据
			DataShipping & shipping = DataShippingManager::Instance()->GetData(uid);

			//获取航运配置
			const ConfigShipping::ShippingItem & shipcfg = ShippingCfgWrap().GetShippingCfg();

			//获得额外的徽章奖励
			shipping.badge += shipcfg.aid_extra_reward();

			DataShippingManager::Instance()->UpdateItem(shipping);
			shipping.SetMessage(resp->mutable_selfshipping());

			resp->set_othboxid(boxid);

			//---------------------推送数据
			LMI->sendMsg(uid,resp)?0:R_ERROR;

		}catch(const std::exception &e)
		{
			delete resp;
			error_log("failed:%s",e.what());
			return R_ERROR;
		}

		//------------------------发送请求
		DBCUserBaseWrap userwrap(uid);
		unsigned allianceid = userwrap.Obj().alliance_id;
		ProtoShipping::CSOfferAidReq* m = new ProtoShipping::CSOfferAidReq;
		m->set_othuid(othuid);
		m->set_myuid(uid);
		m->set_myallianceid(allianceid);
		m->mutable_box()->set_boxid(boxid);
		m->mutable_box()->set_propsid(propsid);
		m->mutable_box()->set_count(propscnt);
		m->mutable_box()->set_aidstatus(0);
		m->mutable_box()->set_status(0);
		m->mutable_box()->set_coin(coin);
		m->mutable_box()->set_exp(exp);
		m->mutable_box()->set_fig(userwrap.Obj().fig);
		m->mutable_box()->set_name(userwrap.Obj().name);

		int ret = BMI->BattleConnectNoReplyByUID(othuid, m);

		//跨服援助装船 添加动态消息
		AddShipDyInfoOverServer(uid,othuid);

		return ret;
	}

	ProtoShipping::OfferAidResp* resp = new ProtoShipping::OfferAidResp;
	try{
		OfferAid(uid, othuid, boxid, resp);
		AddShipDyInfo(uid,othuid);	//援助装船 添加动态消息 同服
	}catch(const std::exception &e)
	{
		delete resp;
		error_log("failed:%s",e.what());
		return R_ERROR;
	}
	return LMI->sendMsg(uid,resp)?0:R_ERROR;
}

int LogicShippingManager::Process(ProtoShipping::CSOfferAidReq* req)
{
	//加载别的玩家用户的档
	OffUserSaveControl offuserctl(req->othuid());
	unsigned boxid = req->mutable_box()->boxid();
	//判断航运箱子是否存在
	if (!DataShippingboxManager::Instance()->IsExistItem(req->othuid(), boxid))
	{
		error_log("ship box not exist. uid=%u,boxid=%u", req->othuid(), boxid);
		throw runtime_error("box_not_exist");
	}

	DataShippingbox & box = DataShippingboxManager::Instance()->GetData(req->othuid(), boxid);

	//判断箱子是否处于援助中
	if (box.aid_status != aid_stauts_public && box.aid_status != aid_status_commercial)
	{
		error_log("ship box not in seeking aid status. uid=%u,boxid=%u", req->othuid(), boxid);
		throw runtime_error("box_status_error");
	}

	//如果是公会援助，则要判断公会是否一致.
	if (box.aid_status == aid_status_commercial)
	{
		//商会援助
		//判断是否同一商会
		DBCUserBaseWrap othuserwrap(req->othuid());

		if (req->myallianceid() != othuserwrap.Obj().alliance_id)
		{
			error_log("not in same alliance. uid=%u,othuid=%u", req->myuid(), req->othuid());
			throw runtime_error("alliance_not_same");
		}
		else if (0 == req->myallianceid())
		{
			error_log("both not in alliance. uid=%u,othuid=%u", req->myuid(), req->othuid());
			throw runtime_error("not_in_alliance");
		}
	}

	//装箱
	unsigned boxuid = box.uid;

	//判断箱子是否已装货
	if (box.status != box_status_empty)
	{
		error_log("ship box not empty. uid=%u,boxid=%u", boxuid, boxid);
		throw runtime_error("box_not_empty");
	}

	//判断箱子中的物品是否为空
	if (box.propsid == 0 || 0 == box.count)
	{
		error_log("ship box need non-zero props. uid=%u,boxid=%u", boxuid, boxid);
		throw runtime_error("box_data_error");
	}
	//设置箱子状态
	box.status = box_status_full;
	//设置该箱子的援助uid
	box.aid_uid =  req->myuid();
	//更新箱子的援助状态
//	box.aid_status += 2;   //至于为什么是+2，这是根据状态的值决定的。
	LogicShippingManager::Instance()->SetShippingboxAidStatus(box, box.aid_status + 2);
	DataShippingboxManager::Instance()->UpdateItem(box);

	//添加援助记录
	DataAidRecordManager::Instance()->AddAidRecord(req->othuid(), req->myuid(), Time::GetGlobalTime());


	if(UserManager::Instance()->IsOnline(req->othuid()))
	{
		//推送航运箱子
		ProtoShipping::PushShipBox *msg = new ProtoShipping::PushShipBox;
		msg->mutable_box()->set_boxid(boxid);
		msg->mutable_box()->set_propsid(req->mutable_box()->propsid());
		msg->mutable_box()->set_count(req->mutable_box()->count());
		msg->mutable_box()->set_aidstatus(box.aid_status);
		msg->mutable_box()->set_status(box.status);
		msg->mutable_box()->set_coin(req->mutable_box()->coin());
		msg->mutable_box()->set_exp(req->mutable_box()->exp());
		msg->mutable_box()->set_fig(req->mutable_box()->fig());
		msg->mutable_box()->set_name(req->mutable_box()->name());
		LMI->sendMsg(req->othuid(),msg);
	}

	//若满箱、则推送满箱信息
	unsigned propsid = 0;
	unsigned propscnt = 0;
	FullBoxReward(req->othuid(),propsid,propscnt);
	if(propsid != 0 && propscnt != 0)
	{
		//存储点券信息
		DataGameActivity& activity = DataGameActivityManager::Instance()->GetUserActivity(req->othuid(), userActId);
		activity.actdata[e_Activity_UserData_1_index_9] =  propsid;
		activity.actdata[e_Activity_UserData_1_index_10] =  propscnt;
		DataGameActivityManager::Instance()->UpdateActivity(activity);

		if(UserManager::Instance()->IsOnline(req->othuid()))
		{
			//消息推送
			ProtoShipping::PushFullBoxShipMsg *msg = new ProtoShipping::PushFullBoxShipMsg;
			msg->mutable_point()->set_propsid(propsid);
			msg->mutable_point()->set_propscnt(propscnt);
			LMI->sendMsg(req->othuid(),msg);
		}
	}

	ProtoShipping::CSOfferAidResp* resp = new ProtoShipping::CSOfferAidResp;
	resp->set_myuid(req->myuid());
	return BMI->BattleConnectNoReplyByUID(req->myuid(), resp);
}

int LogicShippingManager::Process(ProtoShipping::CSOfferAidResp* req)
{
	//更新用户的援助次数
	LogicUserManager::Instance()->UpdateHelpTimes(req->myuid());
	LogicAllianceManager::Instance()->AddRaceOrderProgress(req->myuid(), alliance_race_task_of_help, 1);
	return 0;
}

int LogicShippingManager::OfferAid(unsigned uid, unsigned othuid, unsigned boxid, ProtoShipping::OfferAidResp * resp)
{
	//加载别的玩家用户的档
	OffUserSaveControl offuserctl(othuid);

	//判断航运箱子是否存在
	if (!DataShippingboxManager::Instance()->IsExistItem(othuid, boxid))
	{
		error_log("ship box not exist. uid=%u,boxid=%u", othuid, boxid);
		throw runtime_error("box_not_exist");
	}

	DataShippingbox & box = DataShippingboxManager::Instance()->GetData(othuid, boxid);

	//判断箱子是否处于援助中
	if (box.aid_status != aid_stauts_public && box.aid_status != aid_status_commercial)
	{
		error_log("ship box not in seeking aid status. uid=%u,boxid=%u", othuid, boxid);
		throw runtime_error("box_status_error");
	}

	//如果是公会援助，则要判断公会是否一致.
	if (box.aid_status == aid_status_commercial)
	{
		//商会援助
		//判断是否同一商会
		DBCUserBaseWrap userwrap(uid);
		DBCUserBaseWrap othuserwrap(othuid);

		if (userwrap.Obj().alliance_id != othuserwrap.Obj().alliance_id)
		{
			error_log("not in same alliance. uid=%u,othuid=%u", uid, othuid);
			throw runtime_error("alliance_not_same");
		}
		else if (0 == userwrap.Obj().alliance_id)
		{
			error_log("both not in alliance. uid=%u,othuid=%u", uid, othuid);
			throw runtime_error("not_in_alliance");
		}
	}

	//装箱
	PackBoxUnderlying(uid, common_pack_box,box, resp->mutable_commons());

	//满箱点券处理
	unsigned propsid = 0;
	unsigned propscnt = 0;
	FullBoxReward(othuid,propsid,propscnt);
	if(propsid != 0 && propscnt != 0)
	{
		//存储点券信息
		DataGameActivity& activity = DataGameActivityManager::Instance()->GetUserActivity(othuid, userActId);
		activity.actdata[e_Activity_UserData_1_index_9] =  propsid;
		activity.actdata[e_Activity_UserData_1_index_10] =  propscnt;
		DataGameActivityManager::Instance()->UpdateActivity(activity);

		if(UserManager::Instance()->IsOnline(othuid)){
			//消息推送
			ProtoShipping::PushFullBoxShipMsg *msg = new ProtoShipping::PushFullBoxShipMsg;
			msg->mutable_point()->set_propsid(propsid);
			msg->mutable_point()->set_propscnt(propscnt);
			LMI->sendMsg(othuid,msg);
		}
	}

	//设置该箱子的援助uid
	box.aid_uid = uid;
	//更新箱子的援助状态
	//box.aid_status += 2;   //至于为什么是+2，这是根据状态的值决定的。
	LogicShippingManager::Instance()->SetShippingboxAidStatus(box, box.aid_status + 2);

	DataShippingboxManager::Instance()->UpdateItem(box);

	//获取自身的航运数据
	DataShipping & shipping = DataShippingManager::Instance()->GetData(uid);

	//获取航运配置
	const ConfigShipping::ShippingItem & shipcfg = ShippingCfgWrap().GetShippingCfg();

	//获得额外的徽章奖励
	shipping.badge += shipcfg.aid_extra_reward();

	DataShippingManager::Instance()->UpdateItem(shipping);
	shipping.SetMessage(resp->mutable_selfshipping());

	box.SetMessage(resp->mutable_othboxes());

	DataBase & base = BaseManager::Instance()->Get(uid);
	resp->mutable_othboxes()->set_name(base.name);
	resp->mutable_othboxes()->set_fig(base.fig);

	//添加援助记录
	DataAidRecordManager::Instance()->AddAidRecord(othuid, uid, Time::GetGlobalTime());

	//更新用户的援助次数
	LogicUserManager::Instance()->UpdateHelpTimes(uid);

	LogicAllianceManager::Instance()->AddRaceOrderProgress(uid, alliance_race_task_of_help, 1);
	//推送航运箱子
	ProtoShipping::PushShipBox *msg = new ProtoShipping::PushShipBox;
	box.SetMessage(msg->mutable_box());
	msg->mutable_box()->set_fig(base.fig);
	msg->mutable_box()->set_name(base.name);
	LMI->sendMsg(othuid,msg);

	return 0;
}

int LogicShippingManager::PackBoxUnderlying(unsigned uid, unsigned type,DataShippingbox & box, DataCommon::CommonItemsCPP* msg)
{
	unsigned boxuid = box.uid;
	unsigned boxid = box.id;

	//判断箱子是否已装货
	if (box.status != box_status_empty)
	{
		error_log("ship box not empty. uid=%u,boxid=%u", boxuid, boxid);
		throw runtime_error("box_not_empty");
	}

	//判断箱子中的物品是否为空
	if (box.propsid == 0 || 0 == box.count)
	{
		error_log("ship box need non-zero props. uid=%u,boxid=%u", boxuid, boxid);
		throw runtime_error("box_data_error");
	}

	if(type == common_pack_box)
	{
		//普通装箱
		//扣除物品
		CommonGiftConfig::CommonModifyItem cfg;
		CommonGiftConfig::PropsItem* propscfg = cfg.add_props();
		propscfg->set_id(box.propsid);
		int count = box.count;
		propscfg->set_count(-count);

		//添加金币和经验
		unsigned coin = box.coin;
		unsigned exp = box.exp;
		ShipBonus(uid,coin,exp);
		cfg.mutable_based()->set_coin(coin);
		cfg.mutable_based()->set_exp(exp);

		//扣除与奖励一起
		LogicUserManager::Instance()->CommonProcess(uid, cfg, "Shipping_Pack", msg);
	}else if(type == view_ad_pack_box)
	{
		//看广告装箱

		const ConfigShipping::ShippingItem & shipcfg = ShippingCfgWrap().GetShippingCfg();
		DataShipping & shipping = DataShippingManager::Instance()->GetData(uid);

		//根据船运徽章获取每艘船看广告次数
		int index = 0;
		for(index = shipcfg.boxes_size() - 1; index >= 0; index--)
		{
			if(shipping.badge >= shipcfg.boxes(index).badge_cond())
				break;
		}
		unsigned view_ad_cnt_max = shipcfg.boxes(index).view_ad_cnt();

		//校验是否超过次数
		if(shipping.view_ad_pack_cnt >= view_ad_cnt_max)
		{
			throw std::runtime_error("view_ad_cnt_is_over");
		}

		shipping.view_ad_pack_cnt += 1;
		DataShippingManager::Instance()->UpdateItem(shipping);
	}

	//设置箱子状态
	box.status = box_status_full;

	DataShippingboxManager::Instance()->UpdateItem(box);

	return 0;
}

int LogicShippingManager::Process(unsigned uid, ProtoShipping::LeaveDockReq* req, ProtoShipping::LeaveDockResp* resp)
{
	LeaveDock(uid, resp);

	return 0;
}

int LogicShippingManager::LeaveDock(unsigned uid, ProtoShipping::LeaveDockResp * resp)
{
	//手动离港
	//判断航运的状态是否处于装货中
	DataShipping & shipping = DataShippingManager::Instance()->GetData(uid);

	if (shipping.status != status_packing)
	{
		error_log("shipping not in packing. uid=%u", uid);
		throw runtime_error("shipping_not_packing");
	}

	//获取航运配置
	const ConfigShipping::ShippingItem & shipcfg = ShippingCfgWrap().GetShippingCfg();

	//判断时间是否超出
	unsigned nowts = Time::GetGlobalTime();

	int diff_ts = shipping.arrive_ts + shipcfg.pack_time() - nowts;

	if (diff_ts < 0)
	{
		error_log("shipping already gone. uid=%u", uid);
		throw runtime_error("ship_already_gone");
	}

	//判断航运箱子是否都已装满
	vector<unsigned> indexs;
	DataShippingboxManager::Instance()->GetIndexs(uid, indexs);
	bool iscomplete = true;
	map<unsigned, unsigned> ItemExp;
	unsigned exp = 0;

	for(int i = 0; i < indexs.size(); ++i)
	{
		DataShippingbox & box = DataShippingboxManager::Instance()->GetDataByIndex(indexs[i]);

		if (0 == box.propsid)
		{
			break;
		}

		if (box.status == box_status_empty)
		{
			iscomplete = false;
			break;
		}

		if (!ItemExp.count(box.propsid))
		{
			ItemExp[box.propsid] = 1;
			exp += box.exp;
		}
	}

	if (!iscomplete)
	{
		error_log("shipping box not all packed. uid=%u", uid);
		throw runtime_error("box_not_all_packed");
	}

	//发放经验奖励
	CommonGiftConfig::CommonModifyItem cfg;
	cfg.mutable_based()->set_exp(exp);

	//VIP特权加成
	unsigned coin = 0;
	ShipBonus(uid,coin,exp);

	unsigned viplevel = LogicVIPManager::Instance()->GetVIPLevel(uid);
	if(viplevel > 0) {
		DataGameActivity& activity = DataGameActivityManager::Instance()->GetUserActivity(uid, userActId);
		const ConfigVIP::VIPCfgInfo &vip_cfg = VIPCfgWrap().GetVIPInfoCfg();
		if(activity.actdata[e_Activity_UserData_1_index_2] < vip_cfg.vipinfo().viporderbonus().bonuscnt(viplevel - 1))
			activity.actdata[e_Activity_UserData_1_index_2] += 1;
		int remaincnt = vip_cfg.vipinfo().viporderbonus().bonuscnt(viplevel - 1) - activity.actdata[e_Activity_UserData_1_index_2];
		resp->mutable_vipshipbonus()->set_remaincnt(remaincnt);
	}

	//处理满箱奖励
	DataGameActivity& activity = DataGameActivityManager::Instance()->GetUserActivity(uid, userActId);
	unsigned point_id = activity.actdata[e_Activity_UserData_1_index_9];
	unsigned point_cnt =  activity.actdata[e_Activity_UserData_1_index_10];
	if(point_id != 0 && point_cnt != 0)
	{
		CommonGiftConfig::PropsItem * propsitem = cfg.add_props();
		propsitem->set_id(point_id);
		propsitem->set_count(point_cnt);

		//重置信息
		activity.actdata[e_Activity_UserData_1_index_9] = 0;
		activity.actdata[e_Activity_UserData_1_index_10] = 0;

		debug_log("levelDock;uid=%u",uid);
		DataGameActivityManager::Instance()->UpdateActivity(activity);
	}

	LogicUserManager::Instance()->CommonProcess(uid, cfg, "Shipping_Finish", resp->mutable_commons());


	//航运徽章奖励。徽章奖励 = 初始 + 提前的小时数
	unsigned badgenum = shipcfg.badge_init() + (int)ceil(static_cast<double>(diff_ts)/3600);
	shipping.badge += badgenum;

	//重置航运的援助次数
	shipping.Reset();

	//设置航运的下次到达时间
	unsigned prepare_time = GetPrePareTime(uid);
	shipping.arrive_ts = Time::GetGlobalTime() + prepare_time;

	shipping.status = status_wait_arrive;

	DataShippingManager::Instance()->UpdateItem(shipping);

	shipping.SetMessage(resp->mutable_shipping());

	//分发箱子
	DistributeBoxes(shipping);
	DataShippingboxManager::Instance()->FullSpecialMessage(uid, resp->mutable_shipboxes());

	//加入到定时任务中
	debug_log("ship_join_routine uid=%u endts=%u ats=%u status=%u", uid, shipping.arrive_ts, shipping.arrive_ts, shipping.status);
	LogicQueueManager::Instance()->JoinRoutine<DataShippingRoutine>(uid, shipping.arrive_ts, routine_type_ship, 0);

	//航运发船添加到任务系统中
	LogicTaskManager::Instance()->AddTaskData(uid,task_of_start_ship,1);
	LogicAllianceManager::Instance()->AddRaceOrderProgress(uid, alliance_race_task_of_start_ship, 1);
	//更新玩家当日货船发出次数
	LogicMailDogManager::Instance()->UpdateMailDogData(uid,update_ship_cnt_daily,1);

	return 0;
}

int LogicShippingManager::Process(unsigned uid, ProtoShipping::SetPlayStatusReq* req, ProtoShipping::SetPlayStatusResp* resp)
{
	SetPlayStatus(uid, resp);

	return 0;
}

int LogicShippingManager::SetPlayStatus(unsigned uid, ProtoShipping::SetPlayStatusResp * resp)
{
	DataShipping & shipping = DataShippingManager::Instance()->GetData(uid);

	shipping.play_status = 1;

	DataShippingManager::Instance()->UpdateItem(shipping);

	shipping.SetMessage(resp->mutable_shipping());

	return 0;
}

int LogicShippingManager::AutoLeaveDock(DataShipping & shipping, ProtoPush::PushBuildingsCPP * msg)
{
	//获取航运配置
	const ConfigShipping::ShippingItem & shipcfg = ShippingCfgWrap().GetShippingCfg();
	unsigned uid = shipping.uid;

	//判断航运箱子是否都已装满
	vector<unsigned> indexs;
	DataShippingboxManager::Instance()->GetIndexs(uid, indexs);
	int emptybox = 0;
	map<unsigned, unsigned> ItemExp;
	unsigned exp = 0;
	bool is_full_box = IsFullBox(uid);

	if (is_full_box)
	{
		//全部装满，才有经验奖励
		//发放经验奖励
		CommonGiftConfig::CommonModifyItem cfg;
		cfg.mutable_based()->set_exp(exp);

		//VIP特权加成
		unsigned coin = 0;
		ShipBonus(uid,coin,exp);

		unsigned viplevel = LogicVIPManager::Instance()->GetVIPLevel(uid);
		if(viplevel > 0) {
			DataGameActivity& activity = DataGameActivityManager::Instance()->GetUserActivity(uid, userActId);
			const ConfigVIP::VIPCfgInfo &vip_cfg = VIPCfgWrap().GetVIPInfoCfg();
			if(activity.actdata[e_Activity_UserData_1_index_2] < vip_cfg.vipinfo().viporderbonus().bonuscnt(viplevel - 1))
				activity.actdata[e_Activity_UserData_1_index_2] += 1;
			int remaincnt = vip_cfg.vipinfo().viporderbonus().bonuscnt(viplevel - 1) - activity.actdata[e_Activity_UserData_1_index_2];

			ProtoShipping::PushShipBonusInfoMsg *msg = new ProtoShipping::PushShipBonusInfoMsg;
			msg->mutable_vipshipbonus()->set_remaincnt(remaincnt);
			LMI->sendMsg(uid,msg);
		}

		//处理满箱奖励
		DataGameActivity& activity = DataGameActivityManager::Instance()->GetUserActivity(uid, userActId);
		unsigned point_id = activity.actdata[e_Activity_UserData_1_index_9];
		unsigned point_cnt =  activity.actdata[e_Activity_UserData_1_index_10];
		if(point_id != 0 && point_cnt != 0)
		{
			CommonGiftConfig::PropsItem * propsitem = cfg.add_props();
			propsitem->set_id(point_id);
			propsitem->set_count(point_cnt);

			//重置信息
			activity.actdata[e_Activity_UserData_1_index_9] = 0;
			activity.actdata[e_Activity_UserData_1_index_10] = 0;
			DataGameActivityManager::Instance()->UpdateActivity(activity);
//			debug_log("autolevel;uid=%u",uid);
		}

		LogicUserManager::Instance()->CommonProcess(uid, cfg, "Shipping_Finish", msg->mutable_commons());

		//航运发船添加到任务系统中
		LogicTaskManager::Instance()->AddTaskData(uid,task_of_start_ship,1);

		//更新玩家当日货船发出次数
		LogicMailDogManager::Instance()->UpdateMailDogData(uid,update_ship_cnt_daily,1);
	}

	//航运徽章奖励.徽章惩罚，徽章数 = 初始-空箱子个数
	unsigned badgenum = shipcfg.badge_init() - emptybox;
	shipping.badge += badgenum;

	//判断上次应结束时间距离当前时间间隔了多久
	unsigned prepare_time = GetPrePareTime(uid);
	unsigned nowts = Time::GetGlobalTime();

	//重置航运的援助次数
	shipping.Reset();


	if (nowts > shipping.arrive_ts + shipcfg.pack_time() + prepare_time)
	{
		//表明上一搜船已打包完、并且已离港.此时不管不管船到达多久，都视为刚到,并处于装货中
		shipping.arrive_ts = nowts;
		shipping.status = status_packing;
	}
	else if(nowts >= shipping.arrive_ts + shipcfg.pack_time() && nowts <= shipping.arrive_ts + shipcfg.pack_time() + prepare_time)
	{
		//表明上一艘传已离港、并等待下一艘船到达
		shipping.arrive_ts = shipping.arrive_ts + shipcfg.pack_time() + prepare_time;
		shipping.status = status_wait_arrive;

	}
	else if(nowts < shipping.arrive_ts + shipcfg.pack_time())
	{
		//表明上一搜船还处于打包中,不做任何处理
	}

	DataShippingManager::Instance()->UpdateItem(shipping);

	shipping.SetMessage(msg->mutable_shipping());

	//分发箱子
	DistributeBoxes(shipping);
	DataShippingboxManager::Instance()->FullSpecialMessage(uid, msg->mutable_shipboxes());

	//加入到定时任务中
	unsigned endts = 0;
	if(shipping.status == status_packing)
	{
		endts = shipping.arrive_ts + shipcfg.pack_time();
	}
	else if(shipping.status == status_wait_arrive)
	{
		endts = shipping.arrive_ts;
	}
	else
	{
		error_log("ship_status_error,status=%d",shipping.status);
		throw std::runtime_error("ship_status_error");
	}
	debug_log("ship_join_routine uid=%u endts=%u ats=%u status=%u", uid, endts, shipping.arrive_ts, shipping.status);
	LogicQueueManager::Instance()->JoinRoutine<DataShippingRoutine>(uid, endts, routine_type_ship, 0);

	return 0;
}

unsigned LogicShippingManager::GetPrePareTime(unsigned uid)
{
	//获取船的等待时间
	const ConfigShipping::ShippingItem & shipcfg = ShippingCfgWrap().GetShippingCfg();

	unsigned waittime = shipcfg.prepare_time();

	//vip减少船的等待时间
	unsigned reducetime = LogicVIPManager::Instance()->ReduceShipWaitTime(uid, waittime);
	//debug_log("uid=%u,waittime=%u,reducetime=%u",uid,waittime,reducetime);

	return reducetime < waittime ? waittime - reducetime: 0;
}

int LogicShippingManager::DistributeBoxes(DataShipping & dataship)
{
	unsigned uid = dataship.uid;

	//分配箱子之前，把之前的旧箱子数据全部清空
	DataShippingboxManager::Instance()->ResetBoxes(uid);

	//根据徽章数，随机选择产生的箱子个数
	//获取航运配置
	const ConfigShipping::ShippingItem & shipcfg = ShippingCfgWrap().GetShippingCfg();
	int box_index = 0;

	for(int i = shipcfg.boxes_size() - 1; i >= 0 ; --i)
	{
		if (dataship.badge >= shipcfg.boxes(i).badge_cond())
		{
			box_index = i;
			break;
		}
	}

	//在两个之间获取随机值
	int boxnum = LogicCommonUtil::GetRandomBetweenAB((int)shipcfg.boxes(box_index).num_range(0u), (int)shipcfg.boxes(box_index).num_range(1u));

	//获取已解锁的物品集合
	unsigned level = BaseManager::Instance()->Get(uid).level;

	vector<unsigned> items;
	vector<unsigned> counts;

	//获取航运的物品
	GetShippingItem(level, boxnum, items, counts);

	map<unsigned, pair<unsigned, unsigned> > itemCoinExp;

	//往航运箱子中填充数据
	for(int i = 0; i < shipcfg.item_kind_num() * boxnum; ++i)
	{
		int itemindex = i/boxnum;
		unsigned boxid = i+1;

		DataShippingbox & box = DataShippingboxManager::Instance()->GetData(uid, boxid);
		unsigned itemid = items[itemindex];
		unsigned count = counts[itemindex];
		//设置箱子的数据
		box.propsid = itemid;
		box.count = count;

		//计算该箱子内，物品的金币和经验
		if (itemCoinExp.count(itemid))
		{
			box.coin = itemCoinExp[itemid].first;
			box.exp = itemCoinExp[itemid].second;
		}
		else
		{
			const ConfigItem::PropItem & sProp = ItemCfgWrap().GetPropsItem(itemid);

			double basecoin = Math::Abs((int)sProp.price().based().coin());
			double baseexp = Math::Abs((int)sProp.price().based().coin());  //原始经验=原始价格
			double ratio = shipcfg.reward_coefficient().multiple();

			int min = shipcfg.reward_coefficient().randomk_range(0u);
			int max = shipcfg.reward_coefficient().randomk_range(1u);

			double random_ratio = LogicCommonUtil::GetRandomBetweenAB(min, max)/static_cast<double>(10);  //0.2-0.8之间随机
			double coe = sProp.coefficient();  //产品系数
			box.coin = (uint32_t)ceil( basecoin * ratio * count * random_ratio * coe);
			box.exp = (uint32_t)ceil(baseexp * ratio * count * (1 - random_ratio) * coe);

			itemCoinExp[itemid].first = box.coin;
			itemCoinExp[itemid].second = box.exp;
		}

		DataShippingboxManager::Instance()->UpdateItem(box);
	}

	return 0;
}

bool LogicShippingManager::AddShipDyInfo(unsigned uid,unsigned other_uid)
{
	//uid:援助者,other_uid:被援助者,援助好友装船会让被访问者增加一条动态
	DynamicInfoAttach *pattach = new DynamicInfoAttach;
	pattach->op_uid = uid;
	if(LogicDynamicInfoManager::Instance()->ProduceOneDyInfo(other_uid,TYPE_DY_SHIP,pattach))
	{
		return true;
	}
	return false;
}

bool LogicShippingManager::AddShipDyInfoOverServer(unsigned uid,unsigned other_uid)
{
	ProtoDynamicInfo::RequestOtherUserMakeDy * msg = new ProtoDynamicInfo::RequestOtherUserMakeDy;
	string ret;
	msg->set_myuid(uid);
	msg->set_othuid(other_uid);
	msg->set_typeid_(TYPE_DY_SHIP);
	msg->set_productid(0);
	msg->set_coin(0);
	msg->set_words(ret);

	return BMI->BattleConnectNoReplyByUID(other_uid, msg);
}

bool LogicShippingManager::AddCAidDyInfo(unsigned uid)
{
	//uid:援助者,给玩家推送动态消息：公会援助已满
	DynamicInfoAttach *pattach = new DynamicInfoAttach;
	pattach->op_uid = uid;
	if(LogicDynamicInfoManager::Instance()->ProduceOneDyInfo(uid,TYPE_DY_FULL,pattach))
	{
		return true;
	}
	return false;
}

int LogicShippingManager::Process(unsigned uid, ProtoShipping::GetShipBonusInfoReq* req, ProtoShipping::GetShipBonusInfoResp* resp)
{
	unsigned viplevel = LogicVIPManager::Instance()->GetVIPLevel(uid);
	int remaincnt = 0;
	if(viplevel > 0) {
		DataGameActivity& activity = DataGameActivityManager::Instance()->GetUserActivity(uid, userActId);
		const ConfigVIP::VIPCfgInfo &vip_cfg = VIPCfgWrap().GetVIPInfoCfg();
		unsigned usedcnt = activity.actdata[e_Activity_UserData_1_index_2];
		remaincnt = vip_cfg.vipinfo().vipshiprewardbonus().bonuscnt(viplevel - 1) - usedcnt;
	}
	resp->mutable_vipshipbonus()->set_remaincnt(remaincnt);

	//返回航运满箱时点券信息
	DataGameActivity& activity = DataGameActivityManager::Instance()->GetUserActivity(uid, userActId);
	if(activity.actdata[e_Activity_UserData_1_index_9] != 0 && activity.actdata[e_Activity_UserData_1_index_10] != 0)
	{
		resp->mutable_point()->set_propsid(activity.actdata[e_Activity_UserData_1_index_9]);
		resp->mutable_point()->set_propscnt(activity.actdata[e_Activity_UserData_1_index_10]);
	}
	return 0;
}

int LogicShippingManager::ResetShipBonusCnt(unsigned uid)
{
	unsigned viplevel = LogicVIPManager::Instance()->GetVIPLevel(uid);
	int remaincnt = 0;
	if(viplevel > 0) {
		DataGameActivity& activity = DataGameActivityManager::Instance()->GetUserActivity(uid, userActId);
		activity.actdata[e_Activity_UserData_1_index_2] = 0;
		DataGameActivityManager::Instance()->UpdateActivity(activity);
	}
	return 0;
}

int LogicShippingManager::ShipBonus(unsigned uid,unsigned &coin,unsigned &exp)
{
	//VIP特权加成
	coin += ceil(coin * (LogicVIPManager::Instance()->VIPShipRewardBonus(uid) + LogicFriendWorkerManager::Instance()->GetShipRewardPercent(uid)));
	exp += ceil(exp * (LogicVIPManager::Instance()->VIPShipRewardBonus(uid) + LogicFriendWorkerManager::Instance()->GetShipRewardPercent(uid)));

	return 0;
}
