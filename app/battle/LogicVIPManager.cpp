#include "ServerInc.h"

bool LogicVIPManager::isVIP(unsigned uid)
{
	DBCUserBaseWrap user(uid);
	user.RefreshVIPLevel(false);

	return user.Obj().viplevel >= 1;
}

int LogicVIPManager::GetVIPLevel(unsigned uid)
{
	DBCUserBaseWrap user(uid);
	bool is_vip = isVIP(uid);
	if(is_vip)
		return user.Obj().viplevel;
	else
		return 0;
}

unsigned LogicVIPManager::GetVIPLevelByCharge(unsigned uid)
{
	DBCUserBaseWrap user(uid);
	unsigned vip_level = 0;
	int index = 0;

	//获取配置信息
	const ConfigVIP::VIPCfgInfo &vip_cfg = VIPCfgWrap().GetVIPInfoCfg();

	//通过配置信息、获取对应的VIP等级
	for(index = 0; index < vip_cfg.vipinfo().vipchargelevel_size(); index++)
	{
		if(user.Obj().acccharge < vip_cfg.vipinfo().vipchargelevel(index))
		{
			vip_level = index;
			break;
		}
	}

	if(index == vip_cfg.vipinfo().vipchargelevel_size())
	{
		vip_level = vip_cfg.vipinfo().vipchargelevel_size();
	}


	return vip_level;
}

unsigned LogicVIPManager::ReduceShipWaitTime(unsigned uid,const unsigned waittime)
{
	unsigned reduce_time = 0;

	//如果为vip用户、则根据对应的vip等级缩短相应的等待时间
	bool is_vip = isVIP(uid);
	if(is_vip)
	{
		unsigned vip_level = GetVIPLevel(uid);
		//获取里vip等待时间对应的缩减配置
		float reduce_percent = VIPCfgWrap().GetVIPInfoCfg().vipinfo().shipwaittime(vip_level - 1);
		reduce_time  = (unsigned)(waittime * (reduce_percent/100));
	}

	return reduce_time;
}

int LogicVIPManager::Process(unsigned uid, ProtoVIP::RandomVIPGiftReq* req, ProtoVIP::RandomVIPGiftResp* resp)
{
	unsigned vip_level = GetVIPLevel(uid);

	//------------判定是否已随机过礼包
	DBCUserBaseWrap userwrap(uid);
	unsigned vip_daily_gift_refresh_ts = userwrap.Obj().vip_daily_gift_refresh_ts;
	if(!Time::IsToday(vip_daily_gift_refresh_ts))
	{

		//------------从配置中重新获取数据

		if(vip_level == 0)//非VIP随机出VIP1下的材料数据
			vip_level = 1;

		const ConfigVIP::VIPCfgInfo & vip_cfg = VIPCfgWrap().GetVIPInfoCfg();
		map<unsigned,unsigned>items;
		items.clear();
		for(int i = 0; i < vip_cfg.vipinfo().vipgift(vip_level - 1).material_size(); i++)
		{
			//从每个材料库中随机物品与数量
			unsigned index = Math::GetRandomInt(vip_cfg.vipinfo().vipgift(vip_level - 1).material(i).props_size());
			unsigned props_id = vip_cfg.vipinfo().vipgift(vip_level - 1).material(i).props(index).id();
			unsigned props_cnt = vip_cfg.vipinfo().vipgift(vip_level - 1).material(i).props(index).count();
			items[props_id] = props_cnt;
		}
		ResetVIPGift(uid,items);

		//----------更新礼包刷新ts
		DataBase &base = BaseManager::Instance()->Get(uid);
		base.vip_daily_gift_refresh_ts = Time::GetGlobalTime();
		BaseManager::Instance()->UpdateDatabase(base);
	}

	//-----------返回礼包数据
	vector<unsigned>vipgift;
	DataVIPGiftManager::Instance()->GetIndexs(uid,vipgift);
	for(int i = 0; i < vipgift.size(); i++)
	{
		DataVIPGift &data = DataVIPGiftManager::Instance()->GetDataByIndex(vipgift[i]);
		if(data.props_cnt != 0 && data.id != 0)
			data.SetMessage(resp->add_item());
	}

	return 0;
}

int LogicVIPManager::Process(unsigned uid, ProtoVIP::GetVIPGiftReq* req, ProtoVIP::GetVIPGiftResp* resp)
{
	unsigned vip_level = GetVIPLevel(uid);
	//判定是否为vip用户
	if(vip_level == 0)
	{
		error_log("user is not vip.uid = %u",uid);
		throw std::runtime_error("user is not vip");
	}

	//判定今日是否已领取过
	DBCUserBaseWrap userwrap(uid);
	unsigned reward_ts =  userwrap.Obj().vip_reward_daily_gift_ts;
	bool isToday  = Time::IsToday(reward_ts);
	if(isToday)
	{
		error_log("gift is reward.uid = %u",uid);
		throw std::runtime_error("gift is reward");
	}

	//获取礼包配置
	const ConfigVIP::VIPCfgInfo & vip_cfg = VIPCfgWrap().GetVIPInfoCfg();
	const CommonGiftConfig::CommonModifyItem & common_cfg = vip_cfg.vipinfo().vipgift(vip_level - 1).reward();

	//构造新的CommonGiftConfig::CommonModifyItem，便于统一处理
	CommonGiftConfig::CommonModifyItem common;

	//获取金币配置
	CommonGiftConfig::BaseItem *base = common.mutable_based();
	base->set_coin(common_cfg.based().coin());

	//从数据表中获取物品配置
	vector<unsigned>vipgift;
	DataVIPGiftManager::Instance()->GetIndexs(uid,vipgift);
	for(int i = 0; i < vipgift.size(); i++)
	{
		DataVIPGift &data = DataVIPGiftManager::Instance()->GetDataByIndex(vipgift[i]);
		if(data.id != 0 && data.props_cnt != 0)
		{
			CommonGiftConfig::PropsItem *props = common.add_props();
			props->set_id(data.id);
			props->set_count(data.props_cnt);
		}
	}

	//统一添加处理
	int ret = LogicUserManager::Instance()->CommonProcess(uid,common,"vip_gift_reward",resp->mutable_commons());

	//更新领取的ts
	userwrap.UpdateVIPRewardTs(Time::GetGlobalTime());
	resp->set_rewardts(userwrap.Obj().vip_reward_daily_gift_ts);

	return 0;
}

int LogicVIPManager::Process(unsigned uid, ProtoVIP::VIPProductSpeedUpReq* req, ProtoVIP::VIPProductSpeedUpResp* resp)
{

	unsigned ud = req->ud();
	DataRoutineBase * proutine = LogicQueueManager::Instance()->GetRoutineObj(uid,ud,routine_type_build);
	if(proutine == NULL)
	{
		error_log("get routine error");
		throw std::runtime_error("get routine error");
	}

	//验证对应的生成设备是否存在
	if (!DataProduceequipManager::Instance()->IsExistItem(uid, ud))
	{
		error_log("invalid data, build is not exist. uid=%u,buildud=%u", uid, ud);
		throw runtime_error("build_not_exist");
	}

	//判定是否为vip用户
	bool is_vip = isVIP(uid);
	if(!is_vip)
	{
		error_log("user is not VIP. uid=%u",uid);
		throw std::runtime_error("user is not vip");
	}

	//获取当前用户免费剩余生产设备秒钻次数
	DBCUserBaseWrap user(uid);
	unsigned vip_level = GetVIPLevel(uid);
	const ConfigVIP::VIPCfgInfo &vip_cfg = VIPCfgWrap().GetVIPInfoCfg();
	int count = vip_cfg.vipinfo().productfreediamond(vip_level - 1) - user.Obj().vip_daily_speed_product_cnt;
	if(count <= 0)
	{
		error_log("vip free speedup is over");
		std::runtime_error("vip free speedup is over");
	}

	//结束当前ud的定时任务
	unsigned nowts = Time::GetGlobalTime();
	unsigned diffts = proutine->endts_ > nowts ? proutine->endts_ - nowts : 0;
	LogicQueueManager::Instance()->FinishRoutine(proutine, ud, diffts);

	//更新秒钻次数
	user.UpdateVIPFreeSpeedCnt();
	resp->set_speedupusecnt(user.Obj().vip_daily_speed_product_cnt);

	return 0;
}

int LogicVIPManager::AddStorageSpace(unsigned uid)
{
	DBCUserBaseWrap user(uid);
	user.RefreshVIPLevel(false);
	int vip_level = user.Obj().viplevel;

	//增加的货仓粮仓容量
	int add_space = 0;

	if(vip_level >= 1)
	{
		const ConfigVIP::VIPCfgInfo &vip_cfg = VIPCfgWrap().GetVIPInfoCfg();
		add_space = vip_cfg.vipinfo().storageaddcnt(vip_level - 1);
	}

	return add_space;
}

int LogicVIPManager::Process(unsigned uid, ProtoVIP::VIPRemoveOrderCDReq* req, ProtoVIP::VIPRemoveOrderCDResp* resp)
{
	unsigned ud = req->orderud();
	DataRoutineBase * proutine = LogicQueueManager::Instance()->GetRoutineObj(uid,ud,routine_type_order);
	if(proutine == NULL)
	{
		error_log("get routine error");
		throw std::runtime_error("get routine error");
	}

	proutine->CheckUd(ud);

	DBCUserBaseWrap user(uid);
	user.RefreshVIPLevel(false);

	//判定是否为vip用户
	int vip_level = user.Obj().viplevel;
	if(vip_level <= 0)
	{
		error_log("user is not VIP. uid=%u",uid);
		throw std::runtime_error("user is not vip");
	}

	//获取当前用户免费剩余的次数
	const ConfigVIP::VIPCfgInfo &vip_cfg = VIPCfgWrap().GetVIPInfoCfg();
	int count = vip_cfg.vipinfo().orderfreewaitcnt(vip_level - 1) - user.Obj().vip_daily_remove_ordercd_cnt;
	if(count <= 0)
	{
		error_log("vip free count is over");
		std::runtime_error("vip free count is over");
	}

	//结束当前ud的定时任务
	unsigned nowts = Time::GetGlobalTime();
	unsigned diffts = proutine->endts_ > nowts ? proutine->endts_ - nowts : 0;
	LogicQueueManager::Instance()->FinishRoutine(proutine, ud, diffts);

	//更新剩余次数
	user.UpdateVIPRemoveOrderCdCnt();
	resp->set_speedupusecnt(user.Obj().vip_daily_remove_ordercd_cnt);

	return 0;
}

int LogicVIPManager::Process(unsigned uid, ProtoVIP::VIPShelfUnLockReq* req, ProtoVIP::VIPShelfUnLockResp* resp)
{
	unsigned vip_level = GetVIPLevel(uid);
	if(0 == vip_level)
	{
		throw std::runtime_error("only_vip_unlock");
	}

	//获取该VIP等级可以扩展的货架数目
	unsigned add_shelf_cnt = VIPCfgWrap().GetVIPInfoCfg().vipinfo().shopaddshelfcnt(vip_level - 1);

	//校验该VIP货架是否已全部解锁
	unsigned cur_vip_shelf_cnt = LogicShopManager::Instance()->GetVIPShelfCnt(uid);
	if(add_shelf_cnt == cur_vip_shelf_cnt)
	{
		throw std::runtime_error("vip_shelf_unlock_maxed");
	}

	//校验通过、则解锁货架
	unsigned ud = DataShopManager::Instance()->GetNewShelfUd(uid);
	DataShop &shop = DataShopManager::Instance()->GetData(uid,ud);
	shop.vip_shelf_flag = 1;
	DataShopManager::Instance()->UpdateItem(shop);
	//添加返回
	shop.SetMessage(resp->mutable_shop());
	return 0;
}

int LogicVIPManager::Process(unsigned uid, ProtoVIP::VIPAddProductQueueReq* req, ProtoVIP::VIPAddProductQueueResp* resp)
{
	unsigned equd = req->ud();
	bool exsit = DataProduceequipManager::Instance()->IsExistItem(uid,req->ud());
	if(!exsit)
	{
		throw std::runtime_error("equip_is_not_exsit");
	}

	unsigned vip_level = GetVIPLevel(uid);
	if(0 == vip_level)
	{
		throw std::runtime_error("only_vip_unlock");
	}

	//获取对应VIP等级增加的生产队列
	const ConfigVIP::VIPCfgInfo &vip_cfg = VIPCfgWrap().GetVIPInfoCfg();
	unsigned add_shelf_max = vip_cfg.vipinfo().productaddboxcnt(vip_level - 1);

	//获取当前因VIP扩展的生产队列
	vector<unsigned>produce_queue;
	int count = 0;
	DataProduceequip & equip = DataProduceequipManager::Instance()->GetData(uid,equd);
	char *sourcedata = equip.shelfsource;
	for(int i = 0; i < equip.queuenum; i++)
	{
		int flag = *(reinterpret_cast<int*>(sourcedata + i * sizeof(int)));
		if(1 == flag)
			count++;
	}

	//判定VIP增加的队列是否已最大
	if(count >= add_shelf_max)
	{
		throw std::runtime_error("shelf_is_maxed");
	}

	//---------校验通过、添加生产队列
	int *pdata = (reinterpret_cast<int*>(sourcedata + equip.queuenum * sizeof(int)));
	*pdata = 1;
	equip.queuenum += 1;
	DataProduceequipManager::Instance()->UpdateItem(equip);

	//设置返回信息
	equip.SetMessage(resp->mutable_equip());
	return 0;
}

void LogicVIPManager::ResetVIPGift(unsigned uid,map<unsigned ,unsigned> items)
{
	//---重置数据时、因为不能直接先删除旧的数据、再添加新的数据(如果旧的数据跟新的数据主键一样、就会存取错误)
	vector<unsigned>vipgift;
	vector<unsigned>not_exsit_newdata;//不在新数据里的物品id
	vipgift.clear();
	not_exsit_newdata.clear();

	//获取不在新数据里的物品id
	DataVIPGiftManager::Instance()->GetIndexs(uid,vipgift);
	for(int i = 0; i < vipgift.size(); i++)
	{
		DataVIPGift &data = DataVIPGiftManager::Instance()->GetDataByIndex(vipgift[i]);
		map<unsigned,unsigned>::iterator it = items.find(data.id);
		if(it == items.end())
		{
			not_exsit_newdata.push_back(data.id);
		}
	}

	//删除不在新数据里的物品id
	for(int i = 0; i < not_exsit_newdata.size(); i++)
	{
		DataVIPGiftManager::Instance()->DelItem(uid,not_exsit_newdata[i]);
	}

	//添加新的数据
	map<unsigned,unsigned>::iterator it = items.begin();
	for(; it != items.end(); it++)
	{
		DataVIPGift &data = DataVIPGiftManager::Instance()->GetData(uid,it->first);
		data.props_cnt = it->second;
		DataVIPGiftManager::Instance()->UpdateItem(data);
	}
}

int LogicVIPManager::GetVIPRewardProductShelf(unsigned uid)
{
	int reward_cnt = 0;
	bool is_vip = isVIP(uid);
	if(is_vip)
	{
		//获取VIP用户增加的生产设备格子数
		unsigned vip_level = GetVIPLevel(uid);
		const ConfigVIP::VIPCfgInfo &vip_cfg = VIPCfgWrap().GetVIPInfoCfg();
		reward_cnt = vip_cfg.vipinfo().productaddboxcnt(vip_level - 1);
	}
	return reward_cnt;
}

float LogicVIPManager::VIPCropsSpeedUp(unsigned uid)
{
	float percent = 0;

	const ConfigVIP::VIPCfgInfo &vip_cfg = VIPCfgWrap().GetVIPInfoCfg();
	unsigned viplevel = GetVIPLevel(uid);
	if(viplevel != 0)
	{
		if(viplevel > vip_cfg.vipinfo().vipplatcropsspeedup_size())
		{
			error_log("vip_level_error");
			throw std::runtime_error("vip_level_error");
		}
		unsigned speedup = vip_cfg.vipinfo().vipplatcropsspeedup(viplevel - 1);
		percent = float(speedup) / 100;
	}
	return percent;
}

float  LogicVIPManager::VIPProduceSpeedUp(unsigned uid)
{
	float percent = 0;

	const ConfigVIP::VIPCfgInfo &vip_cfg = VIPCfgWrap().GetVIPInfoCfg();
	unsigned viplevel = GetVIPLevel(uid);
	if(viplevel != 0)
	{
		if(viplevel > vip_cfg.vipinfo().vipproducespeedup_size())
		{
			error_log("vip_level_error");
			throw std::runtime_error("vip_level_error");
		}
		unsigned speedup = vip_cfg.vipinfo().vipproducespeedup(viplevel - 1);
		percent = float(speedup) / 100;
	}
	return percent;
}

float LogicVIPManager::VIPOrderBonus(unsigned uid)
{
	float percent = 0;
	const ConfigVIP::VIPCfgInfo &vip_cfg = VIPCfgWrap().GetVIPInfoCfg();
	unsigned viplevel = GetVIPLevel(uid);
	if(viplevel != 0)
	{
		DataGameActivity& activity = DataGameActivityManager::Instance()->GetUserActivity(uid, userActId);
		unsigned usedcnt = activity.actdata[e_Activity_UserData_1_index_1];
		if(usedcnt < vip_cfg.vipinfo().viporderbonus().bonuscnt(viplevel - 1))
		{
			if(viplevel > vip_cfg.vipinfo().viporderbonus().orderbonus_size())
			{
				error_log("vip_level_error");
				throw std::runtime_error("vip_level_error");
			}
			unsigned speedup = vip_cfg.vipinfo().viporderbonus().orderbonus(viplevel - 1);
			percent = float(speedup) / 100;
		}
	}

	return percent;
}

float LogicVIPManager::VIPShipRewardBonus(unsigned uid)
{
	float percent = 0;
	const ConfigVIP::VIPCfgInfo &vip_cfg = VIPCfgWrap().GetVIPInfoCfg();
	unsigned viplevel = GetVIPLevel(uid);
	if(viplevel != 0)
	{
		DataGameActivity& activity = DataGameActivityManager::Instance()->GetUserActivity(uid, userActId);
		unsigned usedcnt = activity.actdata[e_Activity_UserData_1_index_2];
		if(usedcnt < vip_cfg.vipinfo().viporderbonus().bonuscnt(viplevel - 1)){
			if(viplevel > vip_cfg.vipinfo().vipshiprewardbonus().shiprewardbonus_size())
			{
				error_log("vip_level_error");
				throw std::runtime_error("vip_level_error");
			}
			unsigned speedup = vip_cfg.vipinfo().vipshiprewardbonus().shiprewardbonus(viplevel - 1);
			percent = float(speedup) / 100;
		}
	}

	return percent;
}

int LogicVIPManager::VIPAllianceCompetition(unsigned viplevel)
{
	int count = 0;
	const ConfigVIP::VIPCfgInfo &vip_cfg = VIPCfgWrap().GetVIPInfoCfg();
	if(viplevel != 0)
	{
		if(viplevel > vip_cfg.vipinfo().vipalliancetaskaddcnt_size())
		{
			error_log("vip_level_error");
			throw std::runtime_error("vip_level_error");
		}
		count = vip_cfg.vipinfo().vipalliancetaskaddcnt(viplevel -1);
	}
	return count;
}

int LogicVIPManager::VIPCompetitionIntegral(unsigned viplevel)
{
	int result = 1;
	const ConfigVIP::VIPCfgInfo &vip_cfg = VIPCfgWrap().GetVIPInfoCfg();
	if(viplevel != 0)
	{
		if(viplevel > vip_cfg.vipinfo().vipcompetitionintegral_size())
		{
			error_log("vip_level_error");
			throw std::runtime_error("vip_level_error");
		}
		result = vip_cfg.vipinfo().vipcompetitionintegral(viplevel -1);
	}
	return result;
}

float LogicVIPManager::VIPAnimalSpeedUp(unsigned uid)
{
	float percent = 0;
	const ConfigVIP::VIPCfgInfo &vip_cfg = VIPCfgWrap().GetVIPInfoCfg();
	unsigned viplevel = GetVIPLevel(uid);
	if(viplevel != 0)
	{
		if(viplevel > vip_cfg.vipinfo().vipanimalspeedup_size())
		{
			error_log("vip_level_error");
			throw std::runtime_error("vip_level_error");
		}
		unsigned speedup = vip_cfg.vipinfo().vipanimalspeedup(viplevel - 1);
		percent = float(speedup) / 100;
	}

	return percent;
}

