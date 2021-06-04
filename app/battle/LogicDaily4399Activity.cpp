#include "ServerInc.h"

int Daily4399ActivityManager::SetMessage(unsigned uid, ProtoActivity::GameAcitivityAllCPP * msg)
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

int Daily4399ActivityManager::FullMessage(const DataGameActivity & activity,ProtoActivity::GameAcitivityCPP *msg)
{
	msg->set_id(activity.id);
	msg->set_version(GetVersion());
	for(int i = 0; i < DB_GAME_DATA_NUM; ++i)
	{
		msg->add_actdata(activity.actdata[i]);
	}
	return 0;
}

void Daily4399ActivityManager::CheckLogin(unsigned uid, unsigned last_offline_time)
{
	if(!IsOn())
	{
		return;
	}

	unsigned now = Time::GetGlobalTime();
	int through = Time::ThroughDay(last_offline_time,now);
	if(!through)
	{
		return;
	}

	DataGameActivity & activity = DataGameActivityManager::Instance()->GetUserActivity(uid, actid);
	CheckVersion(activity);

	if(IsDartyActData(activity))
	{
		ResetActivity(activity);	//跨天重置活动数据
	}
}

void Daily4399ActivityManager::CheckDaily()
{
	if(!IsOn())
	{
		return;
	}

	vector<unsigned> Users;
	UserManager::Instance()->GetOnlineUsers(Users);

	vector<unsigned>::iterator it = Users.begin();
	for(; it != Users.end(); it++)
	{
		unsigned uid = *it;
		DataGameActivity & activity = DataGameActivityManager::Instance()->GetUserActivity(uid, actid);
		if(IsDartyActData(activity))
		{
			ResetActivity(activity);	//跨天重置活动数据

			ProtoActivity::GameAcitivity4399Daily* msg = new ProtoActivity::GameAcitivity4399Daily;
			FullMessage(activity, msg->mutable_data());
			LogicManager::Instance()->sendMsg(uid, msg);
		}
	}
}

bool Daily4399ActivityManager::IsDartyActData(DataGameActivity & activity)
{
	//最后一位存储是否领取过终身卡令的标记,不重置
	for(int i = 0; i < DB_GAME_DATA_NUM - 1; ++i)
	{
		if(activity.actdata[i])
			return true;
	}
	return false;
}

int Daily4399ActivityManager::OnCharge(unsigned uid,unsigned reCash)
{
	if(!IsOn())
		return 0;

	DataGameActivity & activity = DataGameActivityManager::Instance()->GetUserActivity(uid, actid);

	//校验版本
	CheckVersion(activity);

	//当天累计充值
	unsigned cash = activity.actdata[DB_GAME_DATA_NUM - 2] + reCash;
	activity.actdata[DB_GAME_DATA_NUM - 2] = cash;
	DataGameActivityManager::Instance()->UpdateActivity(activity);

	int index = -1;
	unsigned left = 0;
	unsigned max = ConfigManager::Instance()->m_4399_daily.size();
	if(max < 1)
	{
		error_log("config_error_in_daily_4399");
		return 0;
	}
	if(cash < ConfigManager::Instance()->m_4399_daily[0])
	{
		ProtoActivity::GameAcitivity4399Daily* msg = new ProtoActivity::GameAcitivity4399Daily;
		FullMessage(activity, msg->mutable_data());
		LogicManager::Instance()->sendMsg(uid, msg);
		return 0;
	}
	else if(cash >= ConfigManager::Instance()->m_4399_daily[max - 1])
	{
		index = max - 1;
	}
	else
	{
		for(uint8_t idx = 0; idx < max;idx++)
		{
			if(cash >= left &&
					cash < ConfigManager::Instance()->m_4399_daily[idx])
			{
				index = idx - 1;
				break;
			}
			else
			{
				left = ConfigManager::Instance()->m_4399_daily[idx];
			}
		}
	}

	if(index < 0)
		return 0;

	if(index >= DB_GAME_DATA_NUM / 2)
	{
		error_log("config_error_in_daily_4399,index=%u",index);
		return 0;
	}

	for(uint8_t i = 0; i <= index; i++)
	{
		if(!activity.actdata[2 * i])
		{
			activity.actdata[2 * i] = 1;		//可领
			activity.actdata[2 * i + 1] = 0;	//未领
		}
	}

	DataGameActivityManager::Instance()->UpdateActivity(activity);

	ProtoActivity::GameAcitivity4399Daily* msg = new ProtoActivity::GameAcitivity4399Daily;
	FullMessage(activity, msg->mutable_data());
	LogicManager::Instance()->sendMsg(uid, msg);

	return 0;
}

void Daily4399ActivityManager::CheckVersion(DataGameActivity & activity)
{
	unsigned version = GetVersion();
	if (activity.version != version)
	{
		ResetActivity(activity);
	}
}

void Daily4399ActivityManager::ResetActivity(DataGameActivity & activity)
{
	//重置活动数据
	activity.version = GetVersion();
	for(int i = 0; i < DB_GAME_DATA_NUM - 1; ++i)
	{
		activity.actdata[i] = 0;	//最后一位存储是否领取过终身卡令的标记,不重置
	}
	DataGameActivityManager::Instance()->UpdateActivity(activity);
}

int Daily4399ActivityManager::Process(unsigned uid, ProtoActivity::Reward4399DailyGiftReq* req, ProtoActivity::Reward4399DailyGiftResp* resp)
{
	//获取活动数据
	DataGameActivity & activity = DataGameActivityManager::Instance()->GetUserActivity(uid, actid);
	const ConfigActivity::ActivityCfg & activitycfg = ConfigManager::Instance()->activitydata.m_config;

	//校验版本
	CheckVersion(activity);

	//校验参数是否合法
	unsigned index = req->index() - 1;
	if(index < 0 || index >= ConfigManager::Instance()->m_4399_daily.size())
	{
		throw std::runtime_error("index_param_error");
	}

	//-----------校验通过、做领取处理
	if((activity.actdata[index * 2] != 1) || (activity.actdata[index * 2 + 1] != 0))
	{
		throw std::runtime_error("already_reward");
	}

	//0.终身卡令特殊处理
	if(index == activitycfg.daily_4399().reward_size() - 1)
	{
		activity.actdata[DB_GAME_DATA_NUM - 1] = 1;
	}

	//1.发放奖励
	LogicUserManager::Instance()->CommonProcess(activity.uid,activitycfg.daily_4399().reward(index),"4399_daily_recharge_reward",resp->mutable_commons());
	//2.重置标记位
	activity.actdata[index * 2 + 1] = 1;
	DataGameActivityManager::Instance()->UpdateActivity(activity);
	FullMessage(activity,resp->mutable_rechargeactivity());
	return 0;
}

int Daily4399ActivityManager::Process(unsigned uid, ProtoActivity::UseCardReq* req, ProtoActivity::UseCardResp* resp)
{
	unsigned type = req->type();
	if(type != e_card_month && type != e_card_life)
	{
		throw std::runtime_error("type_param_error");
	}

	const ConfigActivity::ActivityCfg & activitycfg = ConfigManager::Instance()->activitydata.m_config;

	if(e_card_month == type)
	{
		//月卡处理
		const ConfigCard::MonthCardCPP &monthcard = ConfigManager::Instance()->card.m_config.monthcard();

		//1.消耗月卡令
		LogicUserManager::Instance()->CommonProcess(uid,activitycfg.daily_4399().month_card(),"Use_Month_Card",resp->mutable_commons());

		//2.添加月卡处理
		LogicCardManager::Instance()->HandleCardPurchase(uid,MONTH_CARD_ID);
	}
	else if(e_card_life == type)
	{
		//终生卡处理
		const ConfigCard::LifeCardCPP &liftcard = ConfigManager::Instance()->card.m_config.lifecard();

		//1.消耗终身卡令
		LogicUserManager::Instance()->CommonProcess(uid,activitycfg.daily_4399().life_card(),"Use_Life_Card",resp->mutable_commons());

		//2.添加终生卡处理
		LogicCardManager::Instance()->HandleCardPurchase(uid,LIFE_CARD_ID);
	}

	return 0;
}
