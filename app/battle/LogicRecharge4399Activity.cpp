#include "ServerInc.h"

int Recharge4399ActivityManager::SetMessage(unsigned uid, ProtoActivity::GameAcitivityAllCPP * msg)
{
	if (!IsOn())
	{
		return 0;
	}

	DataGameActivity & activity = DataGameActivityManager::Instance()->GetUserActivity(uid, actid);

	//校验版本
	CheckVersion(activity);
	try
	{
		FullMessage(activity,msg->add_activities());
	}
	catch(runtime_error &e)
	{
		error_log("get activity data error. uid=%u,reason=%s", uid, e.what());
	}
	return 0;
}

int Recharge4399ActivityManager::FullMessage(const DataGameActivity & activity,ProtoActivity::GameAcitivityCPP *msg)
{
	msg->set_id(activity.id);
	msg->set_version(GetVersion());
	for(int i = 0; i < DB_GAME_DATA_NUM; ++i)
	{
		msg->add_actdata(activity.actdata[i]);
	}
	return 0;
}

int Recharge4399ActivityManager::OnCharge(unsigned uid,unsigned cash)
{
	if(!IsOn())
		return 0;

	int index = -1;
	unsigned left = 0;
	unsigned max = ConfigManager::Instance()->m_4399_recharge.size();
	if(max < 1)
	{
		error_log("config_error_in_recharge_4399");
		return 0;
	}

	if(cash < ConfigManager::Instance()->m_4399_recharge[0])
	{
		return 0;
	}
	else if(cash >= ConfigManager::Instance()->m_4399_recharge[max - 1])
	{
		index = max - 1;
	}
	else
	{
		for(uint8_t idx = 0; idx < max;idx++)
		{
			if(cash >= left &&
					cash < ConfigManager::Instance()->m_4399_recharge[idx])
			{
				index = idx - 1;
				break;
			}
			else
			{
				left = ConfigManager::Instance()->m_4399_recharge[idx];
			}
		}
	}

	if(index < 0)
		return 0;

	if(index >= DB_GAME_DATA_NUM / 2)
	{
		error_log("config_error_in_recharge_4399,index=%u",index);
		return 0;
	}

	DataGameActivity & activity = DataGameActivityManager::Instance()->GetUserActivity(uid, actid);

	//校验版本
	CheckVersion(activity);

	//获取档位的索引值
	unsigned pos = 0;
	bool has_pos = false;
	if(activity.actdata[index * 2] == 0)
	{
		pos = index;
		has_pos = true;
	}
	else
	{
		for(int i = index - 1; i >= 0; i--)
		{
			if(activity.actdata[i * 2] == 0)
			{
				pos = i;
				has_pos = true;
				break;
			}
		}
	}

	if(!has_pos)
		return 0;

	activity.actdata[pos * 2] = 1;
	activity.actdata[pos * 2 + 1] = 0;
	DataGameActivityManager::Instance()->UpdateActivity(activity);

	ProtoActivity::GameAcitivity4399Recharge* msg = new ProtoActivity::GameAcitivity4399Recharge;
	FullMessage(activity, msg->mutable_data());
	LogicManager::Instance()->sendMsg(uid, msg);

	return 0;
}

void Recharge4399ActivityManager::CheckVersion(DataGameActivity & activity)
{
	unsigned version = GetVersion();
	if (activity.version != version)
	{
		ResetActivity(activity);
	}
}

void Recharge4399ActivityManager::ResetActivity(DataGameActivity & activity)
{
	//重置活动数据
	activity.version = GetVersion();
	for(int i = 0; i < DB_GAME_DATA_NUM; ++i)
	{
		activity.actdata[i] = 0;
	}
	DataGameActivityManager::Instance()->UpdateActivity(activity);
}

int Recharge4399ActivityManager::Process(unsigned uid, ProtoActivity::Reward4399RechargeGiftReq* req, ProtoActivity::Reward4399RechargeGiftResp* resp)
{
	//获取活动数据
	DataGameActivity & activity = DataGameActivityManager::Instance()->GetUserActivity(uid, actid);
	const ConfigActivity::ActivityCfg & activitycfg = ConfigManager::Instance()->activitydata.m_config;

	//校验版本
	CheckVersion(activity);

	//校验参数是否合法
	unsigned index = req->index() - 1;
	if(index < 0 || index >= ConfigManager::Instance()->m_4399_recharge.size())
	{
		throw std::runtime_error("index_param_error");
	}

	//-----------校验通过、做领取处理
	if((activity.actdata[index * 2] != 1) || (activity.actdata[index * 2 + 1] != 0))
	{
		throw std::runtime_error("already_reward");
	}

	//1.发放奖励
	LogicUserManager::Instance()->CommonProcess(activity.uid,activitycfg.charge_4399().reward(index),"4399_recharge_reward",resp->mutable_commons());
	//2.重置标记位
	activity.actdata[index * 2 + 1] = 1;
	DataGameActivityManager::Instance()->UpdateActivity(activity);
	FullMessage(activity,resp->mutable_rechargeactivity());
	return 0;
}
