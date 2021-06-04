#include "ServerInc.h"


int LogicNPCShopManager::GetRandom(int start,int end)
{
	int random = 0;
	//在合法的区域中生成随机数
	srand((unsigned)time(NULL));
	random = rand()%(end- start + 1) + start;//[start-end]区间的随机数
	return random;
}

int LogicNPCShopManager::NewUser(unsigned uid)
{
	std::vector<unsigned> shopInfo;
	DataNPCShopManager::Instance()->GetIndexs(uid,shopInfo);
	if(shopInfo.size() == 0)
	{
		/********初始化NPC商店信息*********/
		unsigned shelf_cnt = ShopCfgWrap().GetNPCShopInfoCfg().shelf_cnt();
		for(int index = 1; index <= shelf_cnt; index++)
		{
			unsigned ud = DataNPCShopManager::Instance()->GetNewShelfUd(uid);
			DataNPCShopManager::Instance()->GetData(uid,ud);
		}
	}
	return 0;
}

int LogicNPCShopManager::CheckLogin(unsigned uid)
{
	NewUser(uid);
	return 0;
}

int LogicNPCShopManager::Process(unsigned uid, ProtoNPCUser::RequestNPCUser* req, ProtoNPCUser::NPCUser* resp)
{
	resp->MergeFrom(ConfigManager::Instance()->npcuser.m_config);

	//获取商店状态
	int shop_status = 0;
	std::vector<unsigned> shopInfo;
	DataNPCShopManager::Instance()->GetIndexs(uid,shopInfo);
	for(int i = 0; i < shopInfo.size(); i++)
	{
		DataNPCShop &shop = DataNPCShopManager::Instance()->GetDataByIndex(shopInfo[i]);
		if(shop.sell_flag == 1){
			shop_status = 1;
			break;
		}
	}
	resp->set_shopstatus(shop_status);
	return 0;
}

int LogicNPCShopManager::Process(unsigned uid, ProtoNPCUser::GetNPCShopReq* req, ProtoNPCUser::GetNPCShopResp* resp)
{
	//判定npc商店是否要更新数据
	DBCUserBaseWrap userwrap(uid);
	unsigned npc_shop_update_ts = userwrap.Obj().npc_shop_update_ts;

	//当日首次查询npc商店、则更新商店信息
	if(!Time::IsToday(npc_shop_update_ts))
	{
		//-------------清空商店信息
		std::vector<unsigned> shopInfo;
		DataNPCShopManager::Instance()->GetIndexs(uid,shopInfo);
		std::vector<unsigned>::iterator it =shopInfo.begin();
		for(; it != shopInfo.end(); it++)
		{
			DataNPCShop &shop = DataNPCShopManager::Instance()->GetDataByIndex(*it);
			shop.ResetNPCShopShelf();
			DataNPCShopManager::Instance()->UpdateItem(shop);
		}

		//-------------随机生成商店信息
		unsigned shelf_cnt = ShopCfgWrap().GetNPCShopInfoCfg().shelf_cnt();
		//int count = GetRandom(1,shelf_cnt);

		//每种农作物随机个数区间
		unsigned item_cnt_min = ShopCfgWrap().GetNPCShopInfoCfg().per_shelf_item_cnt(0);
		unsigned item_cnt_max = ShopCfgWrap().GetNPCShopInfoCfg().per_shelf_item_cnt(1);

		//获取可随机的农作物
		vector<unsigned> itemsId;
		ItemCfgWrap().GetUnlockNPCShopItem(1,userwrap.Obj().level,itemsId);//type=1为农作物

		//货架ud初始值
		unsigned shelf_ud = 1;

		//随机生成农作物及数量
		for(int i = 1; i <= shelf_cnt; i++)
		{
			int random_index = GetRandom(1,itemsId.size());
			unsigned propsId = itemsId[random_index - 1];
			unsigned propsCnt = GetRandom(item_cnt_min,item_cnt_max);

			//将随机生成的信息添加到商店信息里
			DataNPCShop &shop = DataNPCShopManager::Instance()->GetData(uid,shelf_ud);
			shop.props_id = propsId;
			shop.props_cnt  = propsCnt;
			DataNPCShopManager::Instance()->UpdateItem(shop);
			shelf_ud ++;
		}

		//------------更新npc商店ts
		DataBase & base = BaseManager::Instance()->Get(uid);
		base.npc_shop_update_ts = Time::GetGlobalTime();
		BaseManager::Instance()->UpdateDatabase(base);
	}

	//获取NPC货架信息
	std::vector<unsigned> shopInfo;
	shopInfo.clear();
	DataNPCShopManager::Instance()->GetIndexs(uid,shopInfo);

	//遍历、返回信息
	std::vector<unsigned>::iterator it =shopInfo.begin();
	for(; it != shopInfo.end(); it++)
	{
		DataNPCShop &shop = DataNPCShopManager::Instance()->GetDataByIndex(*it);
		ProtoNPCUser::NPCShopCPP *msg = resp->add_shop();
		shop.SetMessage(msg);
	}
	return 0;
}

int LogicNPCShopManager::Process(unsigned uid, ProtoNPCUser::PurchaseReq* req, ProtoNPCUser::PurchaseResp* resp)
{
	unsigned shelf_ud = req->ud();//对应的货架ud

	//校验数据是否存在
	bool is_exsit = DataNPCShopManager::Instance()->IsExistItem(uid,shelf_ud);
	if(!is_exsit)
	{
		error_log("param error.uid=%u,shelf_ud=%u",uid,shelf_ud);
		throw std::runtime_error("param error");
	}

	DataNPCShop &shop = DataNPCShopManager::Instance()->GetData(uid,shelf_ud);

	//校验物品是否已出售
	if(shop.sell_flag == 1)
	{
		error_log("param error.item is selled.uid=%u,shelf_ud=%u",uid,shelf_ud);
		throw std::runtime_error("param error");
	}

	//货物货架里的物品信息
	unsigned props_id = shop.props_id;
	unsigned props_cnt = shop.props_cnt;

	if(props_id == 0 || props_cnt == 0)
	{
		error_log("param error.shop is empty");
		throw std::runtime_error("param error");
	}

	//获取购买需要的金币
	int coin = ItemCfgWrap().GetPropsItem(props_id).price().based().coin() * props_cnt;
	//扣除金币
	DBCUserBaseWrap user(uid);
	user.CostCoin(-coin,"buy_npcshop_item");
	DataCommon::CommonItemsCPP *common = resp->mutable_commons();
	DataCommon::BaseItemCPP *base = common->mutable_userbase()->add_baseitem();
	base->set_change(coin);
	base->set_totalvalue(user.GetCoin());
	base->set_type(type_coin);

	//添加物品
	LogicPropsManager::Instance()->AddProps(uid,props_id,props_cnt,"buy_shop_item",resp->mutable_commons()->mutable_props());

	//修改NPC商店信息
	shop.sell_flag = 1;
	DataNPCShopManager::Instance()->UpdateItem(shop);

	//添加返回信息
	shop.SetMessage(resp->mutable_npcshop());
	return 0;
}


