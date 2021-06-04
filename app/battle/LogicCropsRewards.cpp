/*
 * LogicCropsRewards.cpp
 *
 *  Created on: 2018年5月25日
 *      Author: colin
 */

#include "LogicCropsRewards.h"



int CropsActivity::GetExtraCropNumber(unsigned uid)
{
	DataGameActivity& activity = DataGameActivityManager::Instance()->GetUserActivity(uid, actid);
	CheckVersion(activity);
	if (!IsOn())
	{
		return 0;
	}

	return 1;
}

void CropsActivity::CheckVersion(DataGameActivity & activity)
{
	int version = GetVersion();

	if (activity.version != version)
	{
		ResetActivity(activity);
	}
}

void CropsActivity::ResetActivity(DataGameActivity & activity)
{
	activity.version = GetVersion();
	activity.actdata[0] = 0;
	DataGameActivityManager::Instance()->UpdateActivity(activity);
}

int CropsActivity::ProcessRedPoint(unsigned uid, unsigned index) //小红点
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


int CropsActivity::SetMessage(unsigned uid, ProtoActivity::GameAcitivityAllCPP * msg)
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


void CropsActivity::SetMessage(DataGameActivity & activity, ProtoActivity::GameAcitivityCPP* msg)
{
	msg->set_id(activity.id);
	msg->set_version(activity.version);
	msg->add_actdata(activity.actdata[0]);
}


int CropsActivity::LoginCheck(unsigned uid)
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
