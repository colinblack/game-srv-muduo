#include "ServerInc.h"

int LogicCustomerManager::GetRandom(int start,int end)
{
	int random = 0;

	//在合法的区域中生成随机数
	srand((unsigned)time(NULL));
	random = rand()%(end- start + 1) + start;//[start-end]区间的随机数

	return random;
}

void LogicCustomerManager::FromMessage(unsigned propsid,unsigned propscnt,unsigned ts,ProtoNPCCustomer::NPCCustomerCPP *msg)
{
	if(propsid != 0 && propscnt !=0)
	{
		msg->set_propsid(propsid);
		msg->set_propscnt(propscnt);
	}
	msg->set_nextts(ts);
}


int LogicCustomerManager::Process(unsigned uid,ProtoNPCCustomer::GetNPCCustomerReq *req,ProtoNPCCustomer::GetNPCCustomerResp *resp)
{
	DBCUserBaseWrap userwrap(uid);
	unsigned level = userwrap.Obj().level;

	const ConfigNPCCustomer::NPCCustomerCPP & cfg = ConfigManager::Instance()->npccustomer.m_config.npccustomer();

	if(level < cfg.unlocklevel())
	{
		throw std::runtime_error("user_level_error");
	}

	GetNPCCustomerInfo(uid,userwrap.Obj().npc_customer1_propsid,userwrap.Obj().npc_customer1_propscnt,userwrap.Obj().npc_customer1_next_visit_ts,resp);
	GetNPCCustomerInfo(uid,userwrap.Obj().npc_customer2_propsid,userwrap.Obj().npc_customer2_propscnt,userwrap.Obj().npc_customer2_next_visit_ts,resp);

	//更新base表
	DataBase & base = BaseManager::Instance()->Get(uid);
	BaseManager::Instance()->UpdateDatabase(base);
	return 0;
}

int LogicCustomerManager::GetNPCCustomerInfo(unsigned uid,unsigned & propsid,unsigned & propscnt,unsigned & nextts,ProtoNPCCustomer::GetNPCCustomerResp *resp)
{
	bool is_empty = NPCCustomerIsEmpty(propsid,propscnt);
	bool update = false;
	if(is_empty)
	{
		if(Time::GetGlobalTime() >= nextts)
		{
			unsigned itemid = 0;
			unsigned itemcount = 0;
			RandomNPCItemInfo(uid,itemid,itemcount);
			if(itemid != 0 && itemcount != 0)
			{
				propsid = itemid;
				propscnt = itemcount;
			}
			update = true;
		}
	}


	if(update && propsid == 0)
	{
		//如果ts到了，但并没有随到物品,则更新ts
		unsigned waittime = GetWaittime(uid);
		nextts = Time::GetGlobalTime() + waittime;
	}

	//设置返回信息
	FromMessage(propsid,propscnt,nextts,resp->add_customer());

	return 0;
}


int LogicCustomerManager::ResetNPCCustomerItem(unsigned uid,unsigned propsid)
{
	DBCUserBaseWrap userwrap(uid);
	if(userwrap.Obj().npc_customer1_propsid == propsid){
		userwrap.Obj().npc_customer1_propsid = 0;
		userwrap.Obj().npc_customer1_propscnt = 0;
	}
	else if(userwrap.Obj().npc_customer2_propsid == propsid) {
		userwrap.Obj().npc_customer2_propsid = 0;
		userwrap.Obj().npc_customer2_propscnt = 0;
	}
	return 0;
}

int LogicCustomerManager::RandomNPCItemInfo(unsigned uid,unsigned & propsid,unsigned & propscnt)
{
	//记录当前已有的npcid
	set<unsigned> npcidList;
	npcidList.clear();
	DBCUserBaseWrap userwrap(uid);
	GetNPCCustomerIdListByItemId(userwrap.Obj().npc_customer1_propsid,userwrap.Obj().npc_customer1_propscnt,npcidList);
	GetNPCCustomerIdListByItemId(userwrap.Obj().npc_customer2_propsid,userwrap.Obj().npc_customer2_propscnt,npcidList);

	//获取当前物品库
	vector<unsigned>propsList;
	DataItemManager::Instance()->GetPropsIdList(uid,propsList);
	if(propsList.size() == 0)
		return 1;

	//从当前物品库中剔除配置中不会被随机到的物品
	const ConfigNPCCustomer::NPCCustomerCPP & cfg = ConfigManager::Instance()->npccustomer.m_config.npccustomer();
	for(int i = 0; i < cfg.notincludeitem_size(); i++)
	{
		vector<unsigned>::iterator it = find(propsList.begin(),propsList.end(),cfg.notincludeitem(i));
		if(it != propsList.end())
			propsList.erase(it);
	}

	//如果当前物品列表不为空、则从当前物品库中剔除现有npc顾客对应的物品id
	if(propsList.size() == 0)
		return 1;
	set<unsigned> ::iterator npcidList_it = npcidList.begin();
	for(; npcidList_it != npcidList.end() && propsList.size() != 0; npcidList_it ++)
	{
		//获取npcid对应的物品列表
		vector<unsigned> npcitemlist;
		npcitemlist.clear();
		ItemCfgWrap().GetPropsListByNPCId(*npcidList_it,npcitemlist);

		//从当前物品库中剔除
		for(int j = 0; j < npcitemlist.size() && propsList.size() != 0; j++)
		{
			vector<unsigned>::iterator it = find(propsList.begin(),propsList.end(),npcitemlist[j]);
			if(it != propsList.end())
				propsList.erase(it);
		}
	}

	//如果当前物品列表不为空、则随机出物品
	if(propsList.size() == 0)
		return 1;
	unsigned pos = GetRandom(1,propsList.size());
	propsid = propsList[pos - 1];
	propscnt = DataItemManager::Instance()->GetItemCount(uid,propsid);
	if(propscnt == 0)
	{
		//理论上、propscnt=0的数据在数据表中应该不会存在
		error_log("data_error.propsid=%u,propscnt=%u",propsid,propscnt);
		return 2;
	}
	//获取配置中物品数量的百分比
	unsigned percent_min = cfg.npcitemcntpercnt(0);
	unsigned percent_max = cfg.npcitemcntpercnt(1);

	//实际物品数量按百分比向下取整,最小为1
	unsigned random_percent = GetRandom(percent_min,percent_max);
//	debug_log("propsid=%u,propscnt=%u,random_percent=%u",propsid,propscnt,random_percent);
	propscnt = (propscnt * random_percent / 100) ? (propscnt * random_percent / 100) : 1;

	return 0;

}

int LogicCustomerManager::GetNPCCustomerIdListByItemId(unsigned itemid,unsigned itemcnt , set<unsigned> & npcidList)
{
	unsigned npcid = 0;
	if(itemid != 0 && itemcnt != 0)
	{
		npcid = ItemCfgWrap().GetNPCIdByItemId(itemid);
		if(npcid)
			npcidList.insert(npcid);
	}
	return 0;
}

bool LogicCustomerManager::NPCCustomerIsEmpty(unsigned propsid,unsigned propscnt)
{
	bool result = false;
	if(propsid == 0 && propscnt == 0)
		result = true;
	return result;
}

int LogicCustomerManager::Process(unsigned uid,ProtoNPCCustomer::SellPropsReq *req,ProtoNPCCustomer::SellPropsResp *resp)
{
	unsigned propsid = req->propsid();

	//通过物品获取物品数量
	int propscnt = 0;
	DBCUserBaseWrap userwrap(uid);
	if(userwrap.Obj().npc_customer1_propsid == propsid)
		propscnt = userwrap.Obj().npc_customer1_propscnt;
	else if(userwrap.Obj().npc_customer2_propsid == propsid)
		propscnt = userwrap.Obj().npc_customer2_propscnt;
	else
	{
		error_log("itemid_is_error.propsid=%u,propscnt=%u",propsid,propscnt);
		throw std::runtime_error("itemid_is_error");
	}

	//校验库存中对应的物品是否满足
	unsigned itemcnt = DataItemManager::Instance()->GetItemCount(uid,propsid);
	if(propscnt > itemcnt)
	{
		error_log("item_is_not_enough.propsid=%u,propscnt=%u,sellcnt=%u",propsid,propscnt,itemcnt);
		throw std::runtime_error("item_is_not_enough");
	}

	//构造通用CommonGiftConfig
	CommonGiftConfig::CommonModifyItem common;
	CommonGiftConfig::PropsItem * propsitem = common.add_props();
	propsitem->set_id(propsid);
	propsitem->set_count(-propscnt);

	CommonGiftConfig::BaseItem * base = common.mutable_based();
	const ConfigNPCCustomer::NPCCustomerCPP & cfg = ConfigManager::Instance()->npccustomer.m_config.npccustomer();
	const ConfigItem::PropItem & propitemcfg = ItemCfgWrap().GetPropsItem(propsid);
	int basePrice = -propitemcfg.price().based().coin();//价格取正数
	float priceK    = cfg.pricebase();
	unsigned exp = cfg.exp();
	int totalPrice = ceil(basePrice * propscnt * priceK);//向上取整

	base->set_coin(totalPrice);
	base->set_exp(exp);

	LogicUserManager::Instance()->CommonProcess(uid,common,"sell_npccustomer",resp->mutable_commons());

	//设置返回信息
	unsigned waittime = GetWaittime(uid);
	if(propsid == userwrap.Obj().npc_customer1_propsid)
	{
		userwrap.Obj().npc_customer1_next_visit_ts = Time::GetGlobalTime() + waittime;
		resp->set_ts(userwrap.Obj().npc_customer1_next_visit_ts);
	}else if(propsid == userwrap.Obj().npc_customer2_propsid)
	{
		userwrap.Obj().npc_customer2_next_visit_ts = Time::GetGlobalTime() + waittime;
		resp->set_ts(userwrap.Obj().npc_customer2_next_visit_ts);
	}

	unsigned npcid = ItemCfgWrap().GetNPCIdByItemId(propsid);
	resp->set_npcid(npcid);

	//重置npc顾客数据
	ResetNPCCustomerItem(uid,propsid);

	//更新base表
	DataBase & dbbase = BaseManager::Instance()->Get(uid);
	BaseManager::Instance()->UpdateDatabase(dbbase);
	return 0;
}

int LogicCustomerManager::Process(unsigned uid,ProtoNPCCustomer::RefuseSellPropsReq *req,ProtoNPCCustomer::RefuseSellPropsResp *resp)
{
	unsigned propsid = req->propsid();

	//校验物品是否存在此对应的物品id
	DBCUserBaseWrap userwrap(uid);
	if(userwrap.Obj().npc_customer1_propsid != propsid && userwrap.Obj().npc_customer2_propsid != propsid)
	{
		error_log("itemid_is_error.npc_customer1_propsid=%u,npc_customer2_propsid=%u,propsid=%u",userwrap.Obj().npc_customer1_propsid,userwrap.Obj().npc_customer2_propsid,propsid);
		throw std::runtime_error("itemid_is_error");
	}

	//设置返回信息
	unsigned waittime = GetWaittime(uid);
	if(propsid == userwrap.Obj().npc_customer1_propsid)
	{
		userwrap.Obj().npc_customer1_next_visit_ts = Time::GetGlobalTime() + waittime;
		resp->set_ts(userwrap.Obj().npc_customer1_next_visit_ts);
	}else if(propsid == userwrap.Obj().npc_customer2_propsid)
	{
		userwrap.Obj().npc_customer2_next_visit_ts = Time::GetGlobalTime() + waittime;
		resp->set_ts(userwrap.Obj().npc_customer2_next_visit_ts);
	}

	unsigned npcid = ItemCfgWrap().GetNPCIdByItemId(propsid);
	resp->set_npcid(npcid);

	//重置npc顾客信息
	ResetNPCCustomerItem(uid,propsid);

	//更新base表
	DataBase & dbbase = BaseManager::Instance()->Get(uid);
	BaseManager::Instance()->UpdateDatabase(dbbase);

	return 0;
}

int LogicCustomerManager::GetWaittime(unsigned uid)
{
	const ConfigNPCCustomer::NPCCustomerCPP & cfg = ConfigManager::Instance()->npccustomer.m_config.npccustomer();
	DBCUserBaseWrap userwrap(uid);
	unsigned level = userwrap.Obj().level;
	for(int i = 0; i < cfg.levelts_size(); i++)
	{
		if(level >= cfg.levelts(i).level(0) && level <= cfg.levelts(i).level(1))
		{
			return cfg.levelts(i).ts();
		}
	}
	return 0;
}

int LogicCustomerManager::GetNPCIdByPropsId(unsigned propsid)
{
	return 0;
}
