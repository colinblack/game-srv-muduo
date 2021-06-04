/*
 * LogicSignInRewards.cpp
 *
 *  Created on: 2018年5月25日
 *      Author: colin
 */
#include "LogicSignInRewards.h"


int SignInActivity::Process(unsigned uid, User::SignInRewardsReq* req, User::SignInRewardsResp* resp)
{
	if (!IsOn())
	{
		throw runtime_error("activity_not_open");
	}
	DataGameActivity& activity = DataGameActivityManager::Instance()->GetUserActivity(uid, actid);
	CheckVersion(activity);

	unsigned index = req->day() -1;

	unsigned mark = (activity.actdata[0] >> index) & 0x01;
	if(mark == 1)
	{
		error_log("gift is reward uid = %u",uid);
		throw std::runtime_error("gift is reward");
	}
	const UserCfg::User& userCfg = UserCfgWrap().User();

	for(int i = 0; i < userCfg.signin_size(); ++i)
	{
		if(req->day() == userCfg.signin(i).day())
		{
			for(int j = 0; j < userCfg.signin(i).prize_size(); ++j)
			{
				const CommonGiftConfig::CommonModifyItem& common_cfg = userCfg.signin(i).prize(j);
				LogicUserManager::Instance()->CommonProcess(uid, common_cfg, "daily_share_gift", resp->mutable_commons());
			}
			activity.actdata[0] |= (1 << index) & 0xFFFF;
			resp->set_marks(activity.actdata[0]);
			DataGameActivityManager::Instance()->UpdateActivity(activity);

			break;
		}
	}

	return 0;
}

int SignInActivity::ProcessRedPoint(unsigned uid, unsigned index)
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

void SignInActivity::SetMessage(DataGameActivity & activity, ProtoActivity::GameAcitivityCPP* msg)
{
	msg->set_id(activity.id);
	msg->set_version(activity.version);

	for(int i = 0; i < 4; ++i)
	{
		msg->add_actdata(activity.actdata[i]);
	}
}

int SignInActivity::SetMessage(unsigned uid, ProtoActivity::GameAcitivityAllCPP * msg)
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

		SetMessage(activity, msg->add_activities());
	}
	catch(runtime_error &e)
	{
		error_log("get activity data error. uid=%u,reason=%s", uid, e.what());
	}

	return 0;
}

void SignInActivity::CountLoginDaysInActiveity(unsigned uid)
{
	if (!IsOn())
	{
		return;
	}

	DataGameActivity& activity = DataGameActivityManager::Instance()->GetUserActivity(uid, actid);
	CheckVersion(activity);

	bool isToday = Time::IsToday(activity.actdata[2]);
	if(!isToday)
	{
		activity.actdata[1] += 1;
		activity.actdata[2] = Time::GetGlobalTime();
		DataGameActivityManager::Instance()->UpdateActivity(activity);
	}
}

void SignInActivity::CheckVersion(DataGameActivity & activity)
{
	int version = GetVersion();

	if (activity.version != version)
	{
		ResetActivity(activity);
	}
}

void SignInActivity::ResetActivity(DataGameActivity & activity)
{
	activity.version = GetVersion();
	for(int i = 0; i < 4; ++i)
	{
		activity.actdata[i] = 0;
	}
	DataGameActivityManager::Instance()->UpdateActivity(activity);
}

int SignInActivity::LoginCheck(unsigned uid)
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


