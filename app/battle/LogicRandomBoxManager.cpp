#include "ServerInc.h"

int LogicRandomBoxManager::GetRandom(int start,int end)
{
	int random = 0;

	//在合法的区域中生成随机数
	srand((unsigned)time(NULL));
	random = rand()%(end- start + 1) + start;//[start-end]区间的随机数

	return random;
}

int LogicRandomBoxManager::Process(unsigned uid,ProtoRandomBox::OpenBoxReq *req,ProtoRandomBox::OpenBoxResp *resp)
{
	//校验cd是否已到
	DBCUserBaseWrap userwrap(uid);

	if(Time::GetGlobalTime() < userwrap.Obj().next_random_box_refresh_ts)
	{
		throw std::runtime_error("box_can_not_be_reresh");
	}


	//随机出箱子信息
	const ConfigRandomBox::RandomBoxInfo &cfg = ConfigManager::Instance()->randombox.m_config;

	//新增逻辑、判定用户等级是否能解锁收费的箱子礼包，如果都不能解锁，则只随机出免费礼包
	unsigned user_level = userwrap.Obj().level;
	bool randomFlag = false;
	for(int i = 0; i < cfg.chargeboxgift_size(); i++)
	{
		if(cfg.chargeboxgift(i).unlocklevel() <= user_level)
		{
			randomFlag = true;
			break;
		}
	}

	//根据randomFlag标记判定是否需要从收费礼包去随机
	unsigned boxtype = boxt_of_free;
	if(randomFlag)
	{
		std::vector<unsigned>boxlist;
		for(int i = 0; i < cfg.randombox().boxtype_size(); i++)
		{
			boxlist.push_back(cfg.randombox().boxtype(i).weight());
		}
		int target = 0;
		LogicCommonUtil::TurnLuckTable(boxlist, boxlist.size(), target);
		boxtype = cfg.randombox().boxtype(target).type();
	}

	//根据箱子类型做后续处理
	HandleOpenBox(uid,boxtype,resp);

	//设置返回信息
	unsigned box_refresh_cd = cfg.randombox().cdtime();
	userwrap.Obj().next_random_box_refresh_ts = Time::GetGlobalTime() + box_refresh_cd;
	resp->set_ts(userwrap.Obj().next_random_box_refresh_ts);
	resp->set_type(boxtype);

	//更新base数据
	DataBase & base = BaseManager::Instance()->Get(uid);
	BaseManager::Instance()->UpdateDatabase(base);
	return 0;
}

int LogicRandomBoxManager::Process(unsigned uid,ProtoRandomBox::BuyBoxGiftReq *req,ProtoRandomBox::BuyBoxGiftResp *resp)
{
	if(req->type() != normal_of_get_randombox_gift && req->type() != viewad_of_get_randombox_gift)
	{
		throw std::runtime_error("type_param_error");
	}

	if(m_giftid.count(uid) == 0)
	{
		throw std::runtime_error("data_error");
	}

	const ConfigRandomBox::RandomBoxInfo &cfg = ConfigManager::Instance()->randombox.m_config;
	DBCUserBaseWrap userwrap(uid);
	for(int i = 0; i < cfg.chargeboxgift_size(); i++)
	{
		if(cfg.chargeboxgift(i).giftid() == m_giftid[uid])
		{
			//礼包需要消耗的钻石
			unsigned diamond_cost = 0;
			//从配置中读取数据以计算金币耗钻
			const CommonGiftConfig::CommonModifyItem & common = cfg.chargeboxgift(i).gift();

			//重新构建新的common，用于处理统一消耗
			CommonGiftConfig::CommonModifyItem  common_cost;
			CommonGiftConfig::BaseItem * base_cost = common_cost.mutable_based();

			if(req->type() == normal_of_get_randombox_gift)
			{
				//若礼包中有金币，则计算金币耗钻
				if(common.has_based())
				{
					if(common.based().has_coin())
					{
						unsigned coin = common.based().coin();
						unsigned coin_discount = cfg.randombox().giftboxdiscount();
						float cost  = (float)coin / coin_discount;
						diamond_cost = ceil(cost);
					}
				}

				//加上物品耗钻
				for(int i = 0; i < common.props_size(); i++)
				{
					unsigned itemid = common.props(i).id();
					unsigned itemcnt = common.props(i).count();
					int diamond_base = -ItemCfgWrap().GetPropsItem(itemid).dimond_cost().based().cash();
					diamond_cost += itemcnt * diamond_base;

					//构造PropsItem 用于后面物品增加
					CommonGiftConfig::PropsItem * propsitem = common_cost.add_props();
					propsitem->set_id(itemid);
					propsitem->set_count(itemcnt);
				}

				//校验
				if(diamond_cost > userwrap.Obj().cash)
				{
					error_log("cash_is_not_enough.diamond_cost=%u,cur_diamond=%u",diamond_cost,userwrap.Obj().cash);
					throw std::runtime_error("cash_is_not_enough");
				}

				//扣钻加物品
				float cost = (float)diamond_cost * cfg.randombox().giftdimaonddiscount() / 100;
				diamond_cost = ceil(cost);
				base_cost->set_cash(-diamond_cost);
				base_cost->set_coin(common.based().coin());
				LogicUserManager::Instance()->CommonProcess(uid,common_cost,"BuyRandomBox",resp->mutable_commons());
			}
			else if(req->type() == viewad_of_get_randombox_gift)
			{
				for(int i = 0; i < common.props_size(); i++)
				{
					unsigned itemid = common.props(i).id();
					unsigned itemcnt = common.props(i).count();

					//构造PropsItem 用于后面物品增加
					CommonGiftConfig::PropsItem * propsitem = common_cost.add_props();
					propsitem->set_id(itemid);
					propsitem->set_count(itemcnt);
				}

				//加物品
				base_cost->set_coin(common.based().coin());
				LogicUserManager::Instance()->CommonProcess(uid,common_cost,"ViewAdORShareRandomBox",resp->mutable_commons());

			}

			m_giftid.erase(uid);
			break;
		}
	}
	return 0;
}

int LogicRandomBoxManager::HandleOpenBox(unsigned uid,unsigned boxtype,ProtoRandomBox::OpenBoxResp *resp)
{
	if(boxt_of_free == boxtype)
	{
		HandleOpenFreeBox(uid,resp);
	}
	else if(box_of_charge == boxtype)
	{
		HandleOpenChargeBox(uid,resp);
	}
	return 0;
}

int LogicRandomBoxManager::HandleOpenFreeBox(unsigned uid,ProtoRandomBox::OpenBoxResp *resp)
{
	const ConfigRandomBox::RandomBoxInfo &cfg = ConfigManager::Instance()->randombox.m_config;
	std::vector<unsigned>itemlist;
	itemlist.clear();
	for(int i = 0; i < cfg.freeboxgift_size(); i++)
	{
		itemlist.push_back(cfg.freeboxgift(i).weight());
	}
	int target = 0;
	LogicCommonUtil::TurnLuckTable(itemlist, itemlist.size(), target);

	//构造通用配置信息
	CommonGiftConfig::CommonModifyItem common;
	if(free_box_of_props == cfg.freeboxgift(target).type())
	{
		CommonGiftConfig::PropsItem *items = common.add_props();
		items->set_id(cfg.freeboxgift(target).itemid());
		items->set_count(cfg.freeboxgift(target).value());
	}
	else if(free_box_of_diamond == cfg.freeboxgift(target).type())
	{
		CommonGiftConfig::BaseItem * base = common.mutable_based();
		base->set_cash(cfg.freeboxgift(target).value());
	}
	else if(free_box_of_coin == cfg.freeboxgift(target).type())
	{
		DBCUserBaseWrap userwrap(uid);
		CommonGiftConfig::BaseItem * base = common.mutable_based();
		unsigned coin = cfg.freeboxgift(target).value();
		unsigned coin_base_min = cfg.randombox().freeboxcoinbase(0);
		unsigned coin_base_max = cfg.randombox().freeboxcoinbase(1);
		unsigned coin_base = GetRandom(coin_base_min,coin_base_max);
		coin = coin_base * userwrap.Obj().level;
		base->set_coin(coin);
	}

	//直接添加对应的免费物品
	LogicUserManager::Instance()->CommonProcess(uid,common,"free_random_box",resp->mutable_commons());
	return 0;
}

int LogicRandomBoxManager::HandleOpenChargeBox(unsigned uid,ProtoRandomBox::OpenBoxResp *resp)
{
	DBCUserBaseWrap userwrap(uid);
	unsigned user_level = userwrap.Obj().level;
	const ConfigRandomBox::RandomBoxInfo &cfg = ConfigManager::Instance()->randombox.m_config;
	std::vector<unsigned>weights;
	std::vector<unsigned>giftids;
	weights.clear();
	giftids.clear();

	//选出等级解锁礼包、并依据权重随机
	for(int i = 0; i < cfg.chargeboxgift_size(); i++)
	{
		if(cfg.chargeboxgift(i).unlocklevel() <= user_level)
		{
			weights.push_back(cfg.chargeboxgift(i).weight());
			giftids.push_back(cfg.chargeboxgift(i).giftid());
		}
	}
	int target = 0;
	LogicCommonUtil::TurnLuckTable(weights, weights.size(), target);

	//设置返回
	resp->set_giftid(giftids[target]);
	m_giftid[uid] = giftids[target];
	return 0;
}

