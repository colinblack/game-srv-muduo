#include "LogicRechargeActivity.h"


int RechargeActivity::SetMessage(unsigned uid, ProtoActivity::GameAcitivityAllCPP * msg)
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


void RechargeActivity::SetMessage(DataGameActivity & activity, ProtoActivity::GameAcitivityCPP* msg)
{
	msg->set_id(activity.id);
	msg->set_version(activity.version);

	for(int i = 0; i < 13; ++i)
	{
		msg->add_actdata(activity.actdata[i]);
	}

}

void RechargeActivity::CheckVersion(DataGameActivity & activity)
{
	int version = GetVersion();

	if (activity.version != version)
	{
		ResetActivity(activity);
	}
}

void RechargeActivity::ResetActivity(DataGameActivity & activity)
{
	activity.version = GetVersion();

	for(int i = 0; i < 13; ++i)
	{
		activity.actdata[i] = 0;  //重置奖励状态
	}

	DataGameActivityManager::Instance()->UpdateActivity(activity);
}


int RechargeActivity::AddDoubleCash(unsigned uid, unsigned cash)
{
	if (!IsOn())
	{
		return 0;
	}
	try
	{
		DataGameActivity& activity = DataGameActivityManager::Instance()->GetUserActivity(uid, actid);
		CheckVersion(activity);
		int cash_array[6] = {60, 120, 300, 500, 1980, 2980};
		DBCUserBaseWrap userwrap(uid);
		if(userwrap.Obj().register_platform == PT_FB)
		{
			cash_array[0] = 60;
			cash_array[1] = 125;
			cash_array[2] = 320;
			cash_array[3] = 650;
			cash_array[4] = 1980;
			cash_array[5] = 2980;

		}
		int i = 0;
		for(; i < sizeof(cash_array)/sizeof(cash_array[0]); ++i)
		{
			if(cash_array[i] == cash) break;
		}
		if(6 == i)
		{
			throw runtime_error("cash is wrong!");
		}

		if(activity.actdata[2*i+1] == 0)
		{
			activity.actdata[2*i+1] = cash_array[i];
		}
		else if(activity.actdata[2*i+1] != cash_array[i])
		{
			throw runtime_error("cash is wrong!");
		}

		//首充双倍
		if(activity.actdata[2*i] == 0)
		{
			activity.actdata[2*i] = 1;
			activity.uid = uid;
			activity.version = GetVersion();
			DataGameActivityManager::Instance()->UpdateActivity(activity);
			DataBase &base = BaseManager::Instance()->Get(uid);
			base.cash += cash;
			BaseManager::Instance()->UpdateDatabase(base);

			ProtoActivity::GameAcitivityRecharge* msg = new ProtoActivity::GameAcitivityRecharge;
			SetMessage(activity, msg->mutable_data());
			LogicManager::Instance()->sendMsg(uid, msg);
		}
	}
	catch(runtime_error &e)
	{
		error_log("get activity data error. uid=%u,reason=%s", uid, e.what());
	}

	return 0;
}

int RechargeActivity::ProcessRedPoint(unsigned uid, unsigned index) //小红点
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

int RechargeActivity::LoginCheck(unsigned uid)
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

