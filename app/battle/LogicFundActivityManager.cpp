#include "ServerInc.h"

int LogicFundActivityManager::SetMessage(unsigned uid, ProtoActivity::GameAcitivityAllCPP * msg)
{
	//无需判定活动是否开启,因为领取长期有效,需给前端返回活动数据
	/*
	if (!IsOn())
	{
		return 0;
	}
	*/

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

int LogicFundActivityManager::FullMessage(const DataGameActivity & activity,ProtoActivity::GameAcitivityCPP *msg)
{
	msg->set_id(activity.id);
	msg->set_version(GetVersion());
	for(int i = 0; i < DB_GAME_DATA_NUM; ++i)
	{
		msg->add_actdata(activity.actdata[i]);
	}
	return 0;
}

void LogicFundActivityManager::CheckVersion(DataGameActivity & activity)
{
	int version = GetVersion();
	if (activity.version != version)
	{
		ResetActivity(activity);
	}
}

void LogicFundActivityManager::ResetActivity(DataGameActivity & activity)
{
	//发放未领取的奖励
	int ret = AutoReward(activity);

	//重置活动数据
	activity.version = GetVersion();
	for(int i = 0; i < DB_GAME_DATA_NUM; ++i)
	{
		activity.actdata[i] = 0;
	}
	DataGameActivityManager::Instance()->UpdateActivity(activity);
}

int LogicFundActivityManager::AutoReward(DataGameActivity & activity)
{
	unsigned buy_ts = activity.actdata[index_of_buy_ts];
	//如果id=0;未购买过基金，无需发放奖励
	if(buy_ts == 0)
		return 1;

	//对未领取的进行自动发放
	const ConfigActivity::ActivityCfg & activitycfg = ConfigManager::Instance()->activitydata.m_config;
	ProtoActivity::PushAutoRewardFundMsg *msg = new ProtoActivity::PushAutoRewardFundMsg;
	for(int i = 0; i < activitycfg.daily_fund().reward_size(); i++)
	{
		unsigned pos = (i + INT_BITS) / INT_BITS;
		unsigned right = (i + INT_BITS) % INT_BITS;
		unsigned flag = (activity.actdata[pos] >> right) & 1;
		if(flag == 0)
		{
			//发放物品
			const ConfigActivity::DailyFundCPP & fundcfg = activitycfg.daily_fund();
			LogicUserManager::Instance()->CommonProcess(activity.uid,fundcfg.reward(i),"fund_auto_reward",msg->add_commons());
		}
	}
	if(msg->commons_size() == 0)
		delete msg;
	else
		LMI->sendMsg(activity.uid,msg);
	return 0;
}

int LogicFundActivityManager::Process(unsigned uid, ProtoActivity::FundPurchaseReq* req, ProtoActivity::FundPurchaseResp* resp)
{
	//判断活动是否开启
	if (!IsOn())
	{
		return 0;
	}

	//获取活动数据
	DataGameActivity & activity = DataGameActivityManager::Instance()->GetUserActivity(uid, actid);
	if(activity.actdata[index_of_buy_ts] != 0)
	{
		throw std::runtime_error("already_purchase_fund");
	}

	//校验版本
	CheckVersion(activity);

	//--------校验通过、做购买处理
	const ConfigActivity::ActivityCfg & activitycfg = ConfigManager::Instance()->activitydata.m_config;

	//1.扣除钻石
	LogicUserManager::Instance()->CommonProcess(uid,activitycfg.daily_fund().diamond(),"DayFundPurchase",resp->mutable_commons());

	//2.初始化数据
	activity.actdata[index_of_buy_ts] = Time::GetGlobalTime();
	DataGameActivityManager::Instance()->UpdateActivity(activity);
	FullMessage(activity,resp->mutable_fundactivity());

	return 0;
}

int LogicFundActivityManager::Process(unsigned uid, ProtoActivity::RewardFundGiftReq* req, ProtoActivity::RewardFundGiftResp* resp)
{
	//获取活动数据
	DataGameActivity & activity = DataGameActivityManager::Instance()->GetUserActivity(uid, actid);
	const ConfigActivity::ActivityCfg & activitycfg = ConfigManager::Instance()->activitydata.m_config;

	//校验版本
	CheckVersion(activity);

	//校验是否购买过基金
	if(activity.actdata[index_of_buy_ts] == 0)
	{
		throw std::runtime_error("haven't_purchase_fund");
	}

	//校验参数是否合法
	unsigned index = req->index() - 1;
	if(index < 0 || index >= activitycfg.daily_fund().reward_size())
	{
		throw std::runtime_error("index_param_error");
	}

	//-----------校验通过、做领取处理
	unsigned pos = (index + INT_BITS) / INT_BITS;
	unsigned offset = (index + INT_BITS) % INT_BITS;
	if(((activity.actdata[pos] >> offset) & 1) == 1)
	{
		throw std::runtime_error("already_reward");
	}

	//1.发放奖励
	LogicUserManager::Instance()->CommonProcess(activity.uid,activitycfg.daily_fund().reward(index),"daily_fund_reward",resp->mutable_commons());
	//2.重置标记位
	activity.actdata[pos] |= (1 << offset);
	DataGameActivityManager::Instance()->UpdateActivity(activity);
	FullMessage(activity,resp->mutable_fundactivity());
	return 0;
}
