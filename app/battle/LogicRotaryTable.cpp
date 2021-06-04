#include "LogicRotaryTable.h"

#define BIT_COUNT 16

unsigned RotaryTableActivity::Get16Low(const unsigned & data)
{
	unsigned num = data & 0x0000ffff;
	return num;
}

unsigned RotaryTableActivity::Get16High(const unsigned & data)
{
	unsigned num = data >> BIT_COUNT;
	return num;
}

void RotaryTableActivity::SetSaveData(unsigned & data, const unsigned & high,const unsigned & low)
{
	data = 0;
	data = ((data >> BIT_COUNT) | high) << BIT_COUNT;
	data = data | low;
}

int RotaryTableActivity::GetRandom(int start,int end)
{
	int random = 0;

	//在合法的区域中生成随机数
	srand((unsigned)time(NULL));
	random = rand()%(end- start + 1) + start;//[start-end]区间的随机数

	return random;
}

int RotaryTableActivity::GetRandomItemList(unsigned uid,vector<unsigned> data,int size,vector<unsigned> & rlt,DataGameActivity & activity)
{
	if(data.size() < size)
	{
		ResetActivity(uid);
		throw std::runtime_error("random_item_error");
	}

	for(int i = 0; i < size; i++)
	{
		int index = GetRandom(1,data.size()) - 1;
		rlt.push_back(data[index]);
		data.erase(data.begin() + index);
	}
	return 0;
}

int RotaryTableActivity::Process(unsigned uid,ProtoRotaryTable::GetRotaryTableInfoReq *req,ProtoRotaryTable::GetRotaryTableInfoResp *resp)
{
	//等级校验
	DBCUserBaseWrap userwrap(uid);
	const ConfigRotaryTable::RotaryTableCPP & rotarytablecpp = ConfigManager::Instance()->rotarytable.m_config.rotarytablebase();
	/*
	unsigned unlock_level = rotarytablecpp.unlock_level();
	unsigned user_level = userwrap.Obj().level;
	if(user_level < unlock_level)
	{
		throw runtime_error("user_level_unlock");
	}
	*/
	//获取数据
	DataGameActivity& activity = DataGameActivityManager::Instance()->GetUserActivity(uid, actid);
	if(activity.actdata[0] == 0)
	{
		//如果没有数据,则重新生成数据(activity.actdata理论上任何一个数据都不应该为0,除非数据为空)
		ResetActivity(uid);
		SetActivityData(uid,activity);
	}

	//返回转盘基本信息
	unsigned rotary_size = ConfigManager::Instance()->rotarytable.m_config.gridcommon_size();
	for(int i = 0; i < rotary_size; i++)
	{
		ProtoRotaryTable::RotaryTableCPP *rotarycpp = resp->add_rotarytable();
		rotarycpp->set_gridid(i + 1);
		unsigned itemid = Get16High(activity.actdata[i]);
		unsigned itemcnt = Get16Low(activity.actdata[i]);
		if(itemid == item_type_cash || itemid == item_type_coin)
		{
			rotarycpp->set_griditemtype(itemid);
		}
		else
		{
			rotarycpp->set_griditemtype(item_type_props);
			rotarycpp->set_griditemid(itemid);
		}
		rotarycpp->set_griditemcnt(itemcnt);
	}

	//返回抽奖次数信息
	resp->mutable_drawinfo()->set_freedrawcnt(activity.actdata[daily_free_draw_count_save_index]);
	resp->mutable_drawinfo()->set_usedfreedrawcnt(activity.actdata[daily_used_free_draw_count_save_index]);
	resp->mutable_drawinfo()->set_usedfriendlydrawcnt(activity.actdata[daily_friendly_draw_count_save_index]);
	return 0;
}

void RotaryTableActivity::SetActivityData(unsigned uid,DataGameActivity & activity)
{
	/*依次对activity.actdata里的前十位数组元素赋值*/
	DBCUserBaseWrap userwrap(uid);
	const ConfigRotaryTable::RotaryTableItemCPP & rotarytableitem = ConfigManager::Instance()->rotarytable.m_config.griditeminfo();
	unsigned itemid = 0;
	unsigned itemcnt = 0;
	unsigned index = 0;
	//---------1.存储钻石信息
	itemid = item_type_cash;
	itemcnt = GetRandom(rotarytableitem.diamondcnt(0),rotarytableitem.diamondcnt(1));
	SetSaveData(activity.actdata[index++],itemid,itemcnt);

	//---------2.存储金币信息
	itemid = item_type_coin;
	itemcnt = userwrap.Obj().level * GetRandom(rotarytableitem.coinbasek(0),rotarytableitem.coinbasek(1));
	SetSaveData(activity.actdata[index++],itemid,itemcnt);

	//---------3.加速卡信息
	itemid = 60089;
	itemcnt = GetRandom(rotarytableitem.speedcardcnt(0),rotarytableitem.speedcardcnt(1));
	SetSaveData(activity.actdata[index++],itemid,itemcnt);

	//--------4.已解锁的农作物
	vector<unsigned>crops;
	crops.clear();
	unsigned location = 1;//粮仓里的物品
	ItemCfgWrap().GetUnlockItem(userwrap.Obj().level,crops,location);
	itemid = crops[GetRandom(1,crops.size()) - 1];
	itemcnt = rotarytableitem.cropbasek();
	SetSaveData(activity.actdata[index++],itemid,itemcnt);

	//-------5.从两个配置库中获取已解锁物品，分别存储到两个格子中
	vector<unsigned>itemids,randomitem;
	for(int i = 0; i < rotarytableitem.randomitemlib_size(); i++)
	{
		itemids.clear();
		randomitem.clear();
		unsigned size = 1;
		for(int j = 0; j < rotarytableitem.randomitemlib(i).itemid_size(); j++)
		{
			unsigned propsid = rotarytableitem.randomitemlib(i).itemid(j);
			unsigned unlock_level = ItemCfgWrap().GetPropsItem(propsid).unlock_level();

			if(userwrap.Obj().level >= unlock_level)
				itemids.push_back(propsid);
		}

		GetRandomItemList(uid,itemids,size,randomitem,activity);
		for(int i = 0; i < size; i ++)
		{
			itemid = randomitem[i];
			itemcnt = 1;
			SetSaveData(activity.actdata[index++],itemid,itemcnt);
		}
	}

	//-------6.剔除类型五中的物品配置库,然后从已解锁的货仓库中随机4个物品，分别放入四个格子中
	itemids.clear();
	randomitem.clear();
	location = 2;
	ItemCfgWrap().GetUnlockItem(userwrap.Obj().level,itemids,location);

	for(int i = 0; i < rotarytableitem.randomitemlib_size(); i++)
	{
		for(int j = 0; j < rotarytableitem.randomitemlib(i).itemid_size(); j++)
		{
			unsigned propsid = rotarytableitem.randomitemlib(i).itemid(j);
			vector<unsigned>::iterator it = find(itemids.begin(),itemids.end(),propsid);
			if(it != itemids.end())
				itemids.erase(it);
		}
	}

	unsigned size = 4;
	GetRandomItemList(uid,itemids,size,randomitem,activity);
	for(int i = 0; i < size; i ++)
	{
		itemid = randomitem[i];
		itemcnt = 1;
		SetSaveData(activity.actdata[index++],itemid,itemcnt);
	}
	DataGameActivityManager::Instance()->UpdateActivity(activity);

}

int RotaryTableActivity::Process(unsigned uid,ProtoRotaryTable::DrawRotaryTableReq *req,ProtoRotaryTable::DrawRotaryTableResp *resp)
{
	//---------获取抽奖方式
	DBCUserBaseWrap userwrap(uid);
	DataGameActivity& activity = DataGameActivityManager::Instance()->GetUserActivity(uid, actid);
	const ConfigRotaryTable::RotaryTableCfg & rotarytablecfg = ConfigManager::Instance()->rotarytable.m_config;
	unsigned free_draw_max = rotarytablecfg.rotarytablebase().daily_free_draw_cnt() + rotarytablecfg.rotarytablebase().daily_login_draw_cnt();
	unsigned friendly_draw_max = rotarytablecfg.rotarytablebase().daily_friendly_draw_cnt();
	string reason = "";

	//优先使用免费抽奖机会
	if(activity.actdata[daily_used_free_draw_count_save_index] < free_draw_max && activity.actdata[daily_free_draw_count_save_index] > 0)
	{
		activity.actdata[daily_used_free_draw_count_save_index] += 1;//使用次数加1
		activity.actdata[daily_free_draw_count_save_index] -= 1;//剩余次数减1
		reason = "free_rotary_table_reward";
	}else if(activity.actdata[daily_friendly_draw_count_save_index] < friendly_draw_max)
	{
		unsigned friendly_value = userwrap.Obj().friendly_value;
		if(rotarytablecfg.rotarytablebase().cost_friendly_value_draw() <= friendly_value)
		{
			activity.actdata[daily_friendly_draw_count_save_index] += 1;//使用次数加1
			userwrap.Obj().friendly_value -= rotarytablecfg.rotarytablebase().cost_friendly_value_draw();//扣掉友情值
			reason = "friendly_rotary_table_reward";
		}
		else
		{
			throw std::runtime_error("no_draw_conditon");
		}
	}else {
		throw std::runtime_error("no_draw_conditon");
	}
	//增加错误调试日志、外网观察
	if(activity.actdata[daily_used_free_draw_count_save_index] >= free_draw_max)
	{
		debug_log("%u,%u",activity.actdata[daily_used_free_draw_count_save_index],free_draw_max);
	}

	DataBase & database = BaseManager::Instance()->Get(uid);
	BaseManager::Instance()->UpdateDatabase(database);
	DataGameActivityManager::Instance()->UpdateActivity(activity);


	//-------------按权重抽奖
	vector<unsigned>weights;
	weights.clear();
	for(int i = 0; i < rotarytablecfg.gridcommon_size(); i++)
	{
		weights.push_back(rotarytablecfg.gridcommon(i).weight());
	}

	int target = 0;
	LogicCommonUtil::TurnLuckTable(weights,weights.size(),target);

	resp->set_gridid(target + 1);

	//-------------奖励
	unsigned itemid = Get16High(activity.actdata[target]);
	unsigned itemcnt = Get16Low(activity.actdata[target]);

	CommonGiftConfig::CommonModifyItem common;
	if(itemid == item_type_cash)
	{
		//加钻石
		CommonGiftConfig::BaseItem *base = common.mutable_based();
		base->set_cash(itemcnt);

		//西山居上报
		if(LogicXsgReportManager::Instance()->IsWXPlatform(uid))
			LogicXsgReportManager::Instance()->XSGRotaryDrawReport(uid,1,itemcnt);


	}else if(itemid == item_type_coin)
	{
		//加金币
		CommonGiftConfig::BaseItem * base = common.mutable_based();
		base->set_coin(itemcnt);

		//西山居上报
		if(LogicXsgReportManager::Instance()->IsWXPlatform(uid))
			LogicXsgReportManager::Instance()->XSGRotaryDrawReport(uid,2,itemcnt);
	}else
	{
		//加物品
		CommonGiftConfig::PropsItem * propsbase = common.add_props();
		propsbase->set_id(itemid);
		propsbase->set_count(itemcnt);

		//西山居上报
		if(LogicXsgReportManager::Instance()->IsWXPlatform(uid))
			LogicXsgReportManager::Instance()->XSGRotaryDrawReport(uid,itemid,itemcnt);
	}
	LogicUserManager::Instance()->CommonProcess(uid,common,reason,resp->mutable_commons());

	//-----------返回抽奖次数信息
	resp->mutable_drawinfo()->set_freedrawcnt(activity.actdata[daily_free_draw_count_save_index]);
	resp->mutable_drawinfo()->set_usedfreedrawcnt(activity.actdata[daily_used_free_draw_count_save_index]);
	resp->mutable_drawinfo()->set_usedfriendlydrawcnt(activity.actdata[daily_friendly_draw_count_save_index]);
	resp->set_curfriendlyvalue(userwrap.Obj().friendly_value);
	return 0;
}


void RotaryTableActivity::ResetActivity(unsigned uid)
{
	DataGameActivity& activity = DataGameActivityManager::Instance()->GetUserActivity(uid, actid);
	for(int i = 0; i < DB_GAME_DATA_NUM; ++i)
	{
		activity.actdata[i] = 0;
	}
	//每日重置数据时,将登录奖励的免费次数加上
	const ConfigRotaryTable::RotaryTableCfg & rotarytablecfg = ConfigManager::Instance()->rotarytable.m_config;
	activity.actdata[daily_free_draw_count_save_index] = rotarytablecfg.rotarytablebase().daily_login_draw_cnt();
	DataGameActivityManager::Instance()->UpdateActivity(activity);
}

int RotaryTableActivity::Process(unsigned uid,ProtoRotaryTable::ShareReq *req,ProtoRotaryTable::ShareResp *resp)
{
	AddShareCount(uid);

	DataGameActivity& activity = DataGameActivityManager::Instance()->GetUserActivity(uid, actid);
	resp->mutable_drawinfo()->set_freedrawcnt(activity.actdata[daily_free_draw_count_save_index]);
	resp->mutable_drawinfo()->set_usedfreedrawcnt(activity.actdata[daily_used_free_draw_count_save_index]);
	resp->mutable_drawinfo()->set_usedfriendlydrawcnt(activity.actdata[daily_friendly_draw_count_save_index]);
	return 0;
}

void RotaryTableActivity::AddShareCount(unsigned uid)
{
	DataGameActivity& activity = DataGameActivityManager::Instance()->GetUserActivity(uid, actid);
	activity.actdata[daily_free_draw_count_save_index] += 1;
	DataGameActivityManager::Instance()->UpdateActivity(activity);

}



