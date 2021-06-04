#include "LogicActivityTencent.h"

int LogicActivityTencent::Process(unsigned uid,ProtoActivityTencent::RewardStatusReq *req,ProtoActivityTencent::RewardStatusResp *resp)
{
	//等级校验
	DBCUserBaseWrap userwrap(uid);

	//获取数据
	DataGameActivity& activity = DataGameActivityManager::Instance()->GetUserActivity(uid, actid);
	FillActivityStatus(activity, resp);
	return 0;
}

int LogicActivityTencent::Process(unsigned uid,ProtoActivityTencent::GetBlueDailyAward *req, ProtoActivityTencent::GetRewardResp *resp)
{
	DBCUserBaseWrap userwrap(uid);
	DataGameActivity& activity = DataGameActivityManager::Instance()->GetUserActivity(uid, actid);

	uint32_t dailyStatus = activity.actdata[reward_daily_status];
	if(getBit(dailyStatus, reward_daily_status_id_blue) == 1)
	{
		throw std::runtime_error("blue_daily_already_reward");
	}

	bool normalFlag = false;
	CommonGiftConfig::CommonModifyItem rewardNormal;
	if(((userwrap.Obj().blue_info >> TYPE_BLUE) & 0x1) == 1)
	{
		uint32_t blueLevel = userwrap.Obj().blue_info >> 16;
		ActivityTencentCfgWrap().GetNormalBlueDailyReward(blueLevel, &rewardNormal);
		normalFlag = true;
	}
	bool superFlag = false;
	CommonGiftConfig::CommonModifyItem rewardSuper;
	if(((userwrap.Obj().blue_info >> TYPE_LUXURY_BLUE) & 0x1) == 1)
	{
		ActivityTencentCfgWrap().GetSuperBlueDailyReward(&rewardSuper);
		superFlag = true;
	}
	bool yearFlag = false;
	CommonGiftConfig::CommonModifyItem rewardYear;
	if(((userwrap.Obj().blue_info >> TYPE_YEAR_BLUE) & 0x1) == 1)
	{
		ActivityTencentCfgWrap().GetYearBlueDailyReward(&rewardYear);
		yearFlag = true;
	}

	setBit(dailyStatus, reward_daily_status_id_blue);
	activity.actdata[reward_daily_status] = dailyStatus;

	DataGameActivityManager::Instance()->UpdateActivity(activity);
	FillActivityStatus(activity, resp->mutable_status());
	if(normalFlag)
	{
		LogicUserManager::Instance()->CommonProcess(uid,rewardNormal,"get_normal_blue_daily_award",resp->mutable_reward()->mutable_common());
	}
	if(superFlag)
	{
		LogicUserManager::Instance()->CommonProcess(uid,rewardSuper,"get_super_blue_daily_award",resp->mutable_reward()->mutable_common());
	}
	if(yearFlag)
	{
		LogicUserManager::Instance()->CommonProcess(uid,rewardYear,"get_year_blue_daily_award",resp->mutable_reward()->mutable_common());
	}
	return 0;
}


int LogicActivityTencent::Process(unsigned uid,ProtoActivityTencent::GetBlueGrowAward *req, ProtoActivityTencent::GetRewardResp *resp)
{
	DBCUserBaseWrap userwrap(uid);
	DataGameActivity& activity = DataGameActivityManager::Instance()->GetUserActivity(uid, actid);
	uint32_t id = req->id();

	uint32_t status = activity.actdata[reward_grow_blue_status];
	if(getBit(status, id) == 1)
	{
		throw std::runtime_error("blue_grow_already_reward");
	}
	CommonGiftConfig::CommonModifyItem common;
	ActivityTencentCfgWrap().GetBlueGrowReward(id, userwrap.Obj().level,  &common);

	setBit(status, id);
	activity.actdata[reward_grow_blue_status] = status;

	DataGameActivityManager::Instance()->UpdateActivity(activity);
	FillActivityStatus(activity, resp->mutable_status());
	LogicUserManager::Instance()->CommonProcess(uid,common,"get_blue_grow_award",resp->mutable_reward()->mutable_common());
	return 0;
}


int LogicActivityTencent::Process(unsigned uid,ProtoActivityTencent::GetQQgamePrivilegeDailyAward *req, ProtoActivityTencent::GetRewardResp *resp)
{
	DBCUserBaseWrap userwrap(uid);
	DataGameActivity& activity = DataGameActivityManager::Instance()->GetUserActivity(uid, actid);

	uint32_t dailyStatus = activity.actdata[reward_daily_status];
	if(getBit(dailyStatus, reward_daily_status_id_privilege) == 1)
	{
		throw std::runtime_error("qqgame_privilege_daily_already_reward");
	}
	CommonGiftConfig::CommonModifyItem common;
	ActivityTencentCfgWrap().GetQQgamePrivilegeDailyReward(&common);

	setBit(dailyStatus, reward_daily_status_id_privilege);
	activity.actdata[reward_daily_status] = dailyStatus;

	DataGameActivityManager::Instance()->UpdateActivity(activity);
	FillActivityStatus(activity, resp->mutable_status());
	LogicUserManager::Instance()->CommonProcess(uid,common,"get_qqgame_privilege_daily_award",resp->mutable_reward()->mutable_common());
	return 0;
}

int LogicActivityTencent::Process(unsigned uid,ProtoActivityTencent::GetQQgamePrivilegeGrowAward *req, ProtoActivityTencent::GetRewardResp *resp)
{
	DBCUserBaseWrap userwrap(uid);
	DataGameActivity& activity = DataGameActivityManager::Instance()->GetUserActivity(uid, actid);
	uint32_t id = req->id();
	uint32_t status = activity.actdata[reward_grow_privilege_status];
	if(getBit(status, id) == 1)
	{
		throw std::runtime_error("qqgame_privilege_grow_already_reward");
	}
	CommonGiftConfig::CommonModifyItem common;
	ActivityTencentCfgWrap().GetQQgamePrivilegeGrowReward(id, userwrap.Obj().level,  &common);

	setBit(status, id);
	activity.actdata[reward_grow_privilege_status] = status;

	DataGameActivityManager::Instance()->UpdateActivity(activity);
	FillActivityStatus(activity, resp->mutable_status());
	LogicUserManager::Instance()->CommonProcess(uid,common,"get_qqgame_privilege_grow_award",resp->mutable_reward()->mutable_common());
	return 0;
}
int LogicActivityTencent::FillActivityStatus(DataGameActivity& activity, ProtoActivityTencent::RewardStatusResp *resp)
{
	uint32_t dailyStatus = activity.actdata[reward_daily_status];
	uint32_t growBlueStatus = activity.actdata[reward_grow_blue_status];
	uint32_t growPrivilegeStatus = activity.actdata[reward_grow_privilege_status];
	resp->set_bluedaily(getBit(dailyStatus, reward_daily_status_id_blue));
	resp->set_qqgameprivilegedaily(getBit(dailyStatus, reward_daily_status_id_privilege));
	resp->set_bluegrow(growBlueStatus);
	resp->set_qqgameprivilegegrow(growPrivilegeStatus);
	return 0;
}

//重置活动数据
void LogicActivityTencent::ResetActivity(unsigned uid)
{
	DataGameActivity& activity = DataGameActivityManager::Instance()->GetUserActivity(uid, actid);
	activity.actdata[reward_daily_status] = 0;
	DataGameActivityManager::Instance()->UpdateActivity(activity);
}
int LogicActivityTencent::getBit(uint32_t flag, uint32_t offset)
{
	if(offset >= 32)
	{
		throw std::runtime_error("offset_out_of_limit");
	}
	return (flag >> offset) & 0x1;
}
int LogicActivityTencent::setBit(uint32_t& flag, uint32_t offset)
{
	if(offset >= 32)
	{
		throw std::runtime_error("offset_out_of_limit");
	}
	flag |= (0x1 << offset);
	return 0;
}


