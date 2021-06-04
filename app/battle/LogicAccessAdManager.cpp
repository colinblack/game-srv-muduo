#include "ServerInc.h"

int LogicAccessAdManager::Process(unsigned uid,ProtoAccessAd::GetLastViewAdTsReq *req,ProtoAccessAd::GetLastViewAdTsResp *resp)
{
	DataGameActivity& activity = DataGameActivityManager::Instance()->GetUserActivity(uid, actid);
	unsigned ts = activity.actdata[activiy_table_save_index_4];
	resp->set_ts(ts);
	return 0;
}

int LogicAccessAdManager::Process(unsigned uid,ProtoAccessAd::RewardViewAdReq *req,ProtoAccessAd::RewardViewAdResp *resp)
{
	DBCUserBaseWrap userwrap(uid);
	unsigned level = userwrap.Obj().level;
	const ConfigAccessAd::AccessAdCfg & accessad_cfg = ConfigManager::Instance()->accessad.m_config;
	//校验解锁等级
	if(level < accessad_cfg.access_ad().unlock_level())
	{
		throw std::runtime_error("level_unlock");
	}

	//---------校验ts是否已到
	DataGameActivity& activity = DataGameActivityManager::Instance()->GetUserActivity(uid, actid);
	unsigned ts = activity.actdata[activiy_table_save_index_4];
	unsigned cur_ts = Time::GetGlobalTime();
	if(ts > cur_ts)
	{
		throw std::runtime_error("reward_time_not_cd");
	}

	//-----------随机物品奖励信息

	//1.根据等级获取对应的物品库
	unsigned itemlib_index = 0;
	for(int i = 0; i < accessad_cfg.access_ad().reward_item_lib_size(); i++)
	{
		unsigned level_min = accessad_cfg.access_ad().reward_item_lib(i).level(0);
		unsigned level_max = accessad_cfg.access_ad().reward_item_lib(i).level(0);
		if(level >= level_min && level <= level_max)
		{
			itemlib_index = i;
			break;
		}
	}

	//2.根据获取到的库索引,去对应物品库按权重随机出物品
	vector<unsigned>itemlist,weight;
	itemlist.clear();
	weight.clear();

	unsigned itemlib_size = accessad_cfg.access_ad().reward_item_lib(itemlib_index).itemlib_size();
	for(int i = 0; i < itemlib_size; i++)
	{
		itemlist.push_back(accessad_cfg.access_ad().reward_item_lib(itemlib_index).itemlib(i).itemid());
		weight.push_back(accessad_cfg.access_ad().reward_item_lib(itemlib_index).itemlib(i).weight());
	}

	int target = 0;
	LogicCommonUtil::TurnLuckTable(weight,weight.size(),target);
	unsigned random_item_id = itemlist[target];

	//3.发放奖励
	CommonGiftConfig::CommonModifyItem common;
	CommonGiftConfig::PropsItem *propsbase = common.add_props();
	propsbase->set_count(1);
	propsbase->set_id(random_item_id);
	LogicUserManager::Instance()->CommonProcess(uid,common,"view_ad_reward",resp->mutable_commons());

	//4.修改ts
	activity.actdata[activiy_table_save_index_4] = Time::GetGlobalTime() + accessad_cfg.access_ad().reward_wait_time();
	DataGameActivityManager::Instance()->UpdateActivity(activity);
	resp->set_ts(activity.actdata[activiy_table_save_index_4]);
	return 0;
}

int LogicAccessAdManager::Process(unsigned uid,ProtoAccessAd::GetBallonInfoReq *req,ProtoAccessAd::GetBallonInfoResp *resp)
{
	const ConfigAccessAd::AccessAdCfg & accessad_cfg = ConfigManager::Instance()->accessad.m_config;
	DataGameActivity& activity = DataGameActivityManager::Instance()->GetUserActivity(uid, userDataId);
	unsigned usedCnt = activity.actdata[e_Activity_UserData_1_index_3];
	unsigned nextTs = activity.actdata[e_Activity_UserData_1_index_8];
	unsigned remainCnt = accessad_cfg.access_ad().daily_ballon_view_ad_cnt() - usedCnt;
	resp->mutable_ballon()->set_remaincnt(remainCnt);
	resp->mutable_ballon()->set_nextts(nextTs);
	return 0;
}

int LogicAccessAdManager::ResetViewAdCnt(unsigned uid)
{
	DataGameActivity& activity = DataGameActivityManager::Instance()->GetUserActivity(uid, userDataId);
	activity.actdata[e_Activity_UserData_1_index_3] = 0;//重置气球看广告次数
	activity.actdata[e_Activity_UserData_1_index_7] = 0;//重置稻草人看广告次数
	DataGameActivityManager::Instance()->UpdateActivity(activity);
	return 0;
}

int LogicAccessAdManager::Process(unsigned uid, ProtoAccessAd::CommonlViewAdReq* req, ProtoAccessAd::CommonlViewAdResp* resp)
{
	unsigned type = req->type();
	if(type >= type_of_max_view_ad || type < type_of_ballon_veiw_ad)
	{
		throw std::runtime_error("type_param_error");
	}

	if(type == type_of_ballon_veiw_ad)
	{
		HandleBallonViewAd(uid,resp);
	}
	else if(type == type_of_upgrage_veiw_ad)
	{
		HandleUpgardeViewAd(uid,resp);
	}

	return 0;
}

int LogicAccessAdManager::HandleBallonViewAd(unsigned uid,ProtoAccessAd::CommonlViewAdResp* resp)
{

	DBCUserBaseWrap userwrap(uid);
	unsigned level = userwrap.Obj().level;
	const ConfigAccessAd::AccessAdCfg & accessad_cfg = ConfigManager::Instance()->accessad.m_config;
	//校验解锁等级
	if(level < accessad_cfg.access_ad().balloon_unlock_level())
	{
		throw std::runtime_error("level_unlock");
	}

	//校验使用次数
	DataGameActivity& activity = DataGameActivityManager::Instance()->GetUserActivity(uid, userDataId);
	unsigned usedCnt = activity.actdata[e_Activity_UserData_1_index_3];
	unsigned nextTs = activity.actdata[e_Activity_UserData_1_index_8];
	if(usedCnt >= accessad_cfg.access_ad().daily_ballon_view_ad_cnt())
	{
		throw std::runtime_error("view_ad_cnt_is_over");
	}

	//校验cd是否已到
	if(nextTs > Time::GetGlobalTime())
	{
		throw std::runtime_error("ballon_is_cd");
	}


	//-----------随机物品奖励信息

	//1.根据等级获取对应的物品库
	unsigned itemlib_index = 0;
	for(int i = 0; i < accessad_cfg.access_ad().reward_item_lib_size(); i++)
	{
		unsigned level_min = accessad_cfg.access_ad().reward_item_lib(i).level(0);
		unsigned level_max = accessad_cfg.access_ad().reward_item_lib(i).level(0);
		if(level >= level_min && level <= level_max)
		{
			itemlib_index = i;
			break;
		}
	}

	//2.根据获取到的库索引,去对应物品库按权重随机出物品
	vector<unsigned>itemlist,weight;
	itemlist.clear();
	weight.clear();

	unsigned itemlib_size = accessad_cfg.access_ad().reward_item_lib(itemlib_index).itemlib_size();
	for(int i = 0; i < itemlib_size; i++)
	{
		itemlist.push_back(accessad_cfg.access_ad().reward_item_lib(itemlib_index).itemlib(i).itemid());
		weight.push_back(accessad_cfg.access_ad().reward_item_lib(itemlib_index).itemlib(i).weight());
	}

	int target = 0;
	LogicCommonUtil::TurnLuckTable(weight,weight.size(),target);
	unsigned random_item_id = itemlist[target];

	//3.发放奖励
	CommonGiftConfig::CommonModifyItem common;
	CommonGiftConfig::PropsItem *propsbase = common.add_props();
	propsbase->set_count(1);
	propsbase->set_id(random_item_id);
	LogicUserManager::Instance()->CommonProcess(uid,common,"ballon_view_ad_reward",resp->mutable_commons());

	//4.更新看广告次数
	activity.actdata[e_Activity_UserData_1_index_3] += 1;
	activity.actdata[e_Activity_UserData_1_index_8] = Time::GetGlobalTime() + accessad_cfg.access_ad().balloon_cd_time();
	DataGameActivityManager::Instance()->UpdateActivity(activity);
	resp->mutable_ballon()->set_remaincnt(accessad_cfg.access_ad().daily_ballon_view_ad_cnt() - activity.actdata[e_Activity_UserData_1_index_3]);
	resp->mutable_ballon()->set_nextts(activity.actdata[e_Activity_UserData_1_index_8]);
	return 0;
}

int LogicAccessAdManager::HandleUpgardeViewAd(unsigned uid,ProtoAccessAd::CommonlViewAdResp* resp)
{
	DBCUserBaseWrap userwrap(uid);
	unsigned level = userwrap.Obj().level;

	//校验当前等级是否已领取过
	DataGameActivity& activity = DataGameActivityManager::Instance()->GetUserActivity(uid, userDataId);
	unsigned reward_level = activity.actdata[e_Activity_UserData_1_index_4];
	if(level == reward_level)
	{
		throw std::runtime_error("already_reward");
	}

	//校验通过
	//1.发放奖励
	const LevelupUnlock::LevelupUnlockCfg & level_reward = LevelupUnlockCfgWrap().GetLevelupUnlockCfg();
	LogicUserManager::Instance()->CommonProcess(uid,level_reward.levels(level - 1).prize(0),"upgrade_view_ad_reward",resp->mutable_commons());

	//2.更新信息
	activity.actdata[e_Activity_UserData_1_index_4] = level;
	DataGameActivityManager::Instance()->UpdateActivity(activity);

	return 0;
}



int LogicAccessAdManager::Process(unsigned uid, ProtoAccessAd::GetScarecrowInfoReq* req, ProtoAccessAd::GetScarecrowInfoResp* resp)
{
	DBCUserBaseWrap userwrap(uid);
	const ConfigAccessAd::AccessAdCfg & accessad_cfg = ConfigManager::Instance()->accessad.m_config;
	//校验等级是否解锁
	if(userwrap.Obj().level < accessad_cfg.scarecrow_ad().unlock_level())
	{
		throw std::runtime_error("level_unlock");
	}

	DataGameActivity& activity = DataGameActivityManager::Instance()->GetUserActivity(uid, userDataId);
	unsigned workendts = activity.actdata[e_Activity_UserData_1_index_5];
	unsigned nextshowts = activity.actdata[e_Activity_UserData_1_index_6];
	unsigned usedcnt = activity.actdata[e_Activity_UserData_1_index_7];
	resp->mutable_scarecrow()->set_workendts(workendts);
	resp->mutable_scarecrow()->set_nextshowts(nextshowts);
	resp->mutable_scarecrow()->set_remaincnt(accessad_cfg.scarecrow_ad().daily_bonus_cnt() - usedcnt);
	return 0;
}

int LogicAccessAdManager::Process(unsigned uid, ProtoAccessAd::ScarecrowViewAdReq* req, ProtoAccessAd::ScarecrowViewAdResp* resp)
{
	DBCUserBaseWrap userwrap(uid);
	const ConfigAccessAd::AccessAdCfg & accessad_cfg = ConfigManager::Instance()->accessad.m_config;
	DataGameActivity& activity = DataGameActivityManager::Instance()->GetUserActivity(uid, userDataId);
	unsigned workendts = activity.actdata[e_Activity_UserData_1_index_5];
	unsigned nextshowts = activity.actdata[e_Activity_UserData_1_index_6];
	unsigned usedcnt = activity.actdata[e_Activity_UserData_1_index_7];

	//校验等级是否解锁
	if(userwrap.Obj().level < accessad_cfg.scarecrow_ad().unlock_level())
	{
		throw std::runtime_error("level_unlock");
	}

	//校验今日次数是否已用完
	if(usedcnt >= accessad_cfg.scarecrow_ad().daily_bonus_cnt())
	{
		throw std::runtime_error("bonus_cnt_is_over");
	}

	//校验是否处于工作中
	if(workendts != 0 && workendts >= Time::GetGlobalTime())
	{
		//证明稻草人处于农作物加成中
		throw std::runtime_error("scrare_is_working");
	}

	//表明需要扣钻处理
	if(req->has_iscostdimaond() && req->iscostdimaond() == 1)
	{
		LogicUserManager::Instance()->CommonProcess(uid,UserCfgWrap().User().diamondcost().daocaoren_open_cost(),"xianren_open_cost",resp->mutable_commons());
	}

	//校验通过、进行处理
	//1.更新数据
	activity.actdata[e_Activity_UserData_1_index_5] = Time::GetGlobalTime() + accessad_cfg.scarecrow_ad().bonus_time();
	activity.actdata[e_Activity_UserData_1_index_6] = Time::GetGlobalTime() + accessad_cfg.scarecrow_ad().bonus_time() + accessad_cfg.scarecrow_ad().cd_time();
	activity.actdata[e_Activity_UserData_1_index_7] += 1;
	DataGameActivityManager::Instance()->UpdateActivity(activity);

	//2.农地作物剩余时间缩减对应的百分比
	vector<unsigned>cropsland;
	cropsland.clear();
	DataCroplandManager::Instance()->GetIndexs(uid,cropsland);
	for(int i = 0; i < cropsland.size(); i++)
	{
		DataCropland & cropland = DataCroplandManager::Instance()->GetDataByIndex(cropsland[i]);
		if(cropland.plant != 0) {
			if(cropland.harvest_time > Time::GetGlobalTime())
				cropland.harvest_time = cropland.harvest_time - (unsigned)(cropland.harvest_time - Time::GetGlobalTime()) * ScarecrowBonus(uid);
			DataCroplandManager::Instance()->UpdateItem(cropland);

			//重新添加到定时队列里
			set<unsigned>validlands;
			validlands.clear();
			validlands.insert(cropland.id);
			unsigned endts = cropland.harvest_time;
			LogicQueueManager::Instance()->JoinRoutine<DataCropProduceRoutine>(uid, endts, routine_type_build, validlands);

			cropland.SetMessage(resp->add_cropland());
		}
	}


	//3.设置返回
	resp->mutable_scarecrow()->set_workendts(activity.actdata[e_Activity_UserData_1_index_5]);
	resp->mutable_scarecrow()->set_nextshowts(activity.actdata[e_Activity_UserData_1_index_6]);
	resp->mutable_scarecrow()->set_remaincnt(accessad_cfg.scarecrow_ad().daily_bonus_cnt() - activity.actdata[e_Activity_UserData_1_index_7]);
	return 0;
}

float LogicAccessAdManager::ScarecrowBonus(unsigned uid)
{
	float percent = 0;
	const ConfigAccessAd::AccessAdCfg & accessad_cfg = ConfigManager::Instance()->accessad.m_config;


	DataGameActivity& activity = DataGameActivityManager::Instance()->GetUserActivity(uid, userDataId);
	unsigned workendts = activity.actdata[e_Activity_UserData_1_index_5];
	if(workendts >= Time::GetGlobalTime())
	{
		unsigned speedup = accessad_cfg.scarecrow_ad().bonus_percent();
		percent = (float )speedup / 100;
	}
	return percent;
}
