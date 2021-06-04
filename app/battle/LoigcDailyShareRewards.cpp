/*
 * LoigcDailyShareReward.cpp
 *
 *  Created on: 2018年5月24日
 *      Author: colin
 */
#include "LogicDailyShareRewards.h"


int DailyShareActivity::Process(unsigned uid, User::ShareRewardsReq* req, User::ShareRewardsResp* resp)
{
	if (!IsOn())
	{
		throw runtime_error("activity_not_open");
	}

	//记录分享次数
	RotaryTableActivity::Instance()->AddShareCount(uid);

	//判定今日是否已领取过
	DBCUserBaseWrap userwrap(uid);
	unsigned reward_ts =  userwrap.Obj().share_reward_daily_gift_ts;
	bool isToday  = Time::IsToday(reward_ts);
	if(isToday)
	{
		error_log("gift is reward.uid = %u",uid);
		throw std::runtime_error("gift is reward");
	}

	DataGameActivity& activity = DataGameActivityManager::Instance()->GetUserActivity(uid, actid);
	CheckVersion(activity);
	const UserCfg::User& userCfg = UserCfgWrap().User();
	const UserCfg::DailyShareGifts gifts = userCfg.sharegift();
	for(int i = 0; i < gifts.rewards_size(); ++i)
	{
		if(this->reward_id == gifts.rewards(i).id())
		{
			for(int j = 0; j < gifts.rewards(i).prize_size(); ++j)
			{
				const CommonGiftConfig::CommonModifyItem& common_cfg = gifts.rewards(i).prize(j);
				LogicUserManager::Instance()->CommonProcess(uid, common_cfg, "daily_share_gift", resp->mutable_commons());
			}
			//更新领取时间
			userwrap.UpdateShareRewardTs(Time::GetGlobalTime());
			resp->set_rewardts(userwrap.Obj().share_reward_daily_gift_ts);
			DataGameActivityManager::Instance()->UpdateActivity(activity);

			break;
		}
	}

	return 0;
}


int DailyShareActivity::Process(unsigned uid, User::ShareTotalRewardsReq* req, User::ShareTotalRewardsResp* resp)
{
	if (!IsOn())
	{
		throw runtime_error("activity_not_open");
	}

	//判定今日是否已领取过
	DataGameActivity& activity = DataGameActivityManager::Instance()->GetUserActivity(uid, actid);
	CheckVersion(activity);
	const UserCfg::User& userCfg = UserCfgWrap().User();
	const UserCfg::DailyShareGifts gifts = userCfg.sharegift();
	unsigned day = req->day();
	unsigned index = 0;
	for(int i = 0; i < gifts.extra_size(); ++i)
	{
		if(day == gifts.extra(i).day())
		{
			index = i;
			break;
		}
	}
	unsigned mark =  (activity.actdata[3] >> index) & 0x01;
	if(1 == mark)
	{
		error_log("gift is reward.uid = %u",uid);
		throw std::runtime_error("gift is reward");
	}

	for(int j = 0; j < gifts.extra(index).prize_size(); ++j)
	{
		const CommonGiftConfig::CommonModifyItem& common_cfg = gifts.extra(index).prize(j);
		LogicUserManager::Instance()->CommonProcess(uid, common_cfg, "daily_total_share_gift", resp->mutable_commons());
	}

	activity.actdata[3] |= (1 << index) & 0xFFFF;
	DataGameActivityManager::Instance()->UpdateActivity(activity);

	resp->set_marks(activity.actdata[3]);

	return 0;
}


int DailyShareActivity::Process(unsigned uid, User::DaliyShareReq* req, User::DaliyShareResp* resp)
{
	if (!IsOn())
	{
		throw runtime_error("activity_not_open");
	}

	//判定今日是否已领取过
	DataGameActivity& activity = DataGameActivityManager::Instance()->GetUserActivity(uid, actid);
	CheckVersion(activity);
	unsigned reward_ts =  activity.actdata[2];
	bool isToday  = Time::IsToday(reward_ts);
	if(isToday)
	{
		error_log("has shared uid = %u",uid);
		throw std::runtime_error("has_shared");
	}

	unsigned now = Time::GetGlobalTime();
	activity.actdata[2] = now;
	activity.actdata[1] += 1;
	DataGameActivityManager::Instance()->UpdateActivity(activity);
	resp->set_rewardts(now);
	resp->set_days(activity.actdata[1]);

	return 0;
}

int DailyShareActivity::ProcessRedPoint(unsigned uid, unsigned index) //小红点
{
	if (!IsOn())
	{
		return 0;
	}
	try
	{
		//获取活动数据
		DataGameActivity & activity = DataGameActivityManager::Instance()->GetUserActivity(uid, actid);
		CheckVersion(activity);
		if(0 == activity.actdata[index]){
			activity.actdata[index] = 1;
			DataGameActivityManager::Instance()->UpdateActivity(activity);
		}
	}
	catch(runtime_error &e)
	{
		error_log("get activity data error. uid=%u,reason=%s", uid, e.what());
	}

	return 0;
}

void DailyShareActivity::GetRewardId(DataGameActivity & activity)
{
	bool isToday  = Time::IsToday(activity.actdata[4]);
	if(isToday)
	{
		return;
	}
	activity.actdata[4] = Time::GetGlobalTime();
	const UserCfg::User& userCfg = UserCfgWrap().User();
	const UserCfg::DailyShareGifts gifts = userCfg.sharegift();
	//按照权重充奖励id中随机抽取一个
	std::vector<unsigned> weights;
	for(int i = 0; i < gifts.rewards_size(); ++i)
	{
		weights.push_back(gifts.rewards(i).weight());
	}

	int index = 0;
	LogicCommonUtil::TurnLuckTable(weights,weights.size(),index);

	this->reward_id = gifts.rewards(index).id();
	activity.actdata[0] = this->reward_id;
	DataGameActivityManager::Instance()->UpdateActivity(activity);
}


int DailyShareActivity::SetMessage(unsigned uid, ProtoActivity::GameAcitivityAllCPP * msg)
{
	//判断活动是否开启
	if (!IsOn())
	{
		return 0;
	}

	try
	{
		//获取活动数据
		DataGameActivity & activity = DataGameActivityManager::Instance()->GetUserActivity(uid, actid);
		GetRewardId(activity);
		SetMessage(activity, msg->add_activities());
	}
	catch(runtime_error &e)
	{
		error_log("get activity data error. uid=%u,reason=%s", uid, e.what());
	}

	return 0;
}

void DailyShareActivity::SetMessage(DataGameActivity & activity, ProtoActivity::GameAcitivityCPP* msg)
{
	msg->set_id(activity.id);
	msg->set_version(activity.version);
	for(int i = 0; i < 6; ++i)
	{
		msg->add_actdata(activity.actdata[i]);
	}

}
void DailyShareActivity::CheckVersion(DataGameActivity & activity)
{
	int version = GetVersion();
	if (activity.version != version)
	{
		ResetActivity(activity);
	}
}


void DailyShareActivity::ResetActivity(DataGameActivity & activity)
{
	activity.version = GetVersion();
	activity.actdata[0] = 1;
	activity.actdata[1] = 0;
	activity.actdata[2] = 0;
	activity.actdata[3] = 0;
	activity.actdata[4] = 0;
	activity.actdata[5] = 0;
	DataGameActivityManager::Instance()->UpdateActivity(activity);
}


int DailyShareActivity::LoginCheck(unsigned uid)
{
	if (!IsOn())
	{
		return 0;
	}

	try
	{
		//获取活动数据
		DataGameActivity & activity = DataGameActivityManager::Instance()->GetUserActivity(uid, actid);

		CheckVersion(activity);
	}
	catch(runtime_error &e)
	{
		error_log("get activity data error. uid=%u,reason=%s", uid, e.what());
	}

	return 0;
}


