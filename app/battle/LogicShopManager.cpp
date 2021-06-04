#include "ServerInc.h"

int LogicShopManager::AddShelf(unsigned uid)
{
	//获取最大货架数目
	unsigned shelf_cnt_max = ShopCfgWrap().GetShopInfoCfg().shelf_cnt(1);

	//校验玩家货架数目是否已满
	unsigned cur_shelf_cnt = GetShelfCnt(uid);
	if(cur_shelf_cnt >= shelf_cnt_max)
	{
		error_log("shelf count is maxed.");
		throw std::runtime_error("shelf count is maxed.");
	}

	unsigned ud = DataShopManager::Instance()->GetNewShelfUd(uid);
	DataShopManager::Instance()->GetData(uid,ud);

	return 0;
}

int LogicShopManager::NewUser(unsigned uid)
{
	std::vector<unsigned> shopInfo;
	DataShopManager::Instance()->GetIndexs(uid,shopInfo);
	if(shopInfo.size() == 0)
	{
		/********初始化商店信息*********/

		//获取新手货架数目
		unsigned shelf_cnt = ShopCfgWrap().GetShopInfoCfg().shelf_cnt(0);
		for(int index = 1; index <= shelf_cnt; index++)
		{
			AddShelf(uid);
		}
	}
	return 0;
}

int LogicShopManager::Process(unsigned uid, ProtoShop::GetShopReq* req, ProtoShop::GetShopResp* resp)
{
	//获取玩家货架信息
	std::vector<unsigned> shopInfo;
	DataShopManager::Instance()->GetIndexs(uid,shopInfo);

	//遍历、返回信息
	std::vector<unsigned>::iterator it =shopInfo.begin();
	for(; it != shopInfo.end(); it++)
	{
		DataShop &shop = DataShopManager::Instance()->GetDataByIndex(*it);
		ProtoShop::ShopCPP *msg = resp->add_shop();
		shop.SetMessage(msg);
		if(shop.sell_flag == 1)
		{
			msg->set_buyerfig(shop.fig);
			msg->set_buyername(shop.name);
			msg->set_buyeruid(shop.buyer_uid);
		}
	}
	return 0;
}

int LogicShopManager::Process(unsigned uid, ProtoShop::ShelfPropsReq* req, ProtoShop::ShelfPropsResp* resp)
{
	//获取请求参数
	unsigned shelf_ud    = req->ud();
	unsigned props_id    = req->propsid();
	unsigned props_cnt   = req->propscnt();
	unsigned props_price = req->propsprice();
	unsigned ad_flag     = req->adflag();

	//将物品上架到商店中
	ShelfPros(uid,shelf_ud,props_id,props_cnt,props_price,ad_flag,resp);
	return 0;
}

int LogicShopManager::ShelfPros(unsigned uid,unsigned shelf_ud,unsigned props_id,unsigned props_cnt,unsigned props_price,unsigned ad_flag,ProtoShop::ShelfPropsResp* resp)
{
	//校验
	bool is_exsit = DataShopManager::Instance()->IsExistItem(uid,shelf_ud);
	if(!is_exsit)
	{
		error_log("param error");
		throw std::runtime_error("param error");
	}

	DataShop &shop  = DataShopManager::Instance()->GetData(uid,shelf_ud);
	if(shop.buyer_uid == 0 && shop.props_id == 0 && shop.props_cnt == 0 && shop.props_price == 0 && shop.ad_flag == 0 && shop.sell_flag == 0)
	{
		/*--------进行参数校验-----------*/
		//物品校验
		unsigned props_ud    = DataItemManager::Instance()->GetPropsUd(uid,props_id);
		if(-1 == props_ud)
		{
			error_log("param error.not exsit item props_id =%u",props_id);
			throw std::runtime_error("param error");
		}

		//数量校验
		unsigned sell_cnt_min = ShopCfgWrap().GetShopInfoCfg().sell_cnt(0);
		unsigned sell_cnt_max = ShopCfgWrap().GetShopInfoCfg().sell_cnt(1);
		if(props_cnt < sell_cnt_min || props_cnt > sell_cnt_max)
		{
			error_log("props_cnt param error");
			throw std::runtime_error("param error");
		}

		//价格校验
		int item_init_price     = ItemCfgWrap().GetPropsItem(props_id).price().based().coin();
		float base_price        = ShopCfgWrap().GetShopInfoCfg().item_base_price();
		float sell_price        =  (-item_init_price)*props_cnt*base_price;
		unsigned sell_price_max =  unsigned (sell_price);
		if(sell_price > sell_price_max)
			sell_price_max += 1;//向上取整

		if(props_price < 1 || props_price > sell_price_max)
		{
			error_log("param error.props_price=%u,sell_price_max=%u",props_price,sell_price_max);
			throw std::runtime_error("param error");
		}

		//如果有发广告、校验是否cd
		if(1 == ad_flag)
		{
			DBCUserBaseWrap user(uid);
			if(Time::GetGlobalTime() < user.GetCreateAdTs())
			{
				error_log("param error.");
				throw std::runtime_error("param error");
			}
		}

		//---------校验通过
		//删除item表对应的物品数量
		string reason = "shop_shelf_item";
		LogicPropsManager::Instance()->CostProps(uid,props_ud,props_id,props_cnt,reason,resp->mutable_commons()->mutable_props());

		//更新商店对应的货架信息
		shop.props_id    = props_id;
		shop.props_cnt   = props_cnt;
		shop.props_price = props_price;
		shop.ad_flag     = ad_flag;
		DataShopManager::Instance()->UpdateItem(shop);

		//是否发广告
		DBCUserBaseWrap user(uid);
		if(1 == ad_flag) {
			debug_log("shelf_item_relase_ad,uid=%u,shelf_ud=%u",uid,shelf_ud);
			LogicAdvertiseManager::Instance()->CreateAd(uid,shelf_ud);

			//下一次创建广告的时间
			unsigned ad_cd_ts = AdCfgWrap().GetAdInfoCfg().adcdts();
			user.UpdateCreateAdTs(Time::GetGlobalTime() + ad_cd_ts);
			resp->set_ts(Time::GetGlobalTime() + ad_cd_ts);

			//将发广告添加至任务列表中
			LogicTaskManager::Instance()->AddTaskData(uid,task_of_create_ad,1);
			LogicMissionManager::Instance()->AddMission(uid,mission_of_create_ad,1);
		}

		//设置返回信息
		resp->set_ts(user.GetCreateAdTs());
		shop.SetMessage(resp->mutable_shop());
	}
	else
	{
		error_log("param error.shelf is not empty");
		throw std::runtime_error("shelf is not empty");
	}

	//西山居上报
	if(LogicXsgReportManager::Instance()->IsWXPlatform(uid))
		LogicXsgReportManager::Instance()->XSGShopShelfItemReport(uid,shop.props_id,shop.props_cnt,shop.props_price);
	return 0;
}

int LogicShopManager::GetShelfCnt(unsigned uid)
{
	int count = 0;

	//剔除VIP专属货架
	std::vector<unsigned> shopInfo;
	DataShopManager::Instance()->GetIndexs(uid,shopInfo);
	for(int i = 0; i < shopInfo.size(); i++)
	{
		int flag = DataShopManager::Instance()->GetDataByIndex(shopInfo[i]).vip_shelf_flag;
		if(shelf_of_common == flag)
			count++;
	}

	return count;
}

int LogicShopManager::GetVIPShelfCnt(unsigned uid)
{
	int count = 0;

	//获取VIP专属货架
	std::vector<unsigned> shopInfo;
	DataShopManager::Instance()->GetIndexs(uid,shopInfo);
	for(int i = 0; i < shopInfo.size(); i++)
	{
		int flag = DataShopManager::Instance()->GetDataByIndex(shopInfo[i]).vip_shelf_flag;
		if(shelf_of_vip == flag)
			count++;
	}
	return count;
}

int LogicShopManager::GetShelfCntByInvite(unsigned uid)
{
	int count = 0;

	//获取VIP专属货架
	std::vector<unsigned> shopInfo;
	DataShopManager::Instance()->GetIndexs(uid,shopInfo);
	for(int i = 0; i < shopInfo.size(); i++)
	{
		int flag = DataShopManager::Instance()->GetDataByIndex(shopInfo[i]).vip_shelf_flag;
		if(shelf_of_invite == flag)
			count++;
	}
	return count;
}

int LogicShopManager::Process(unsigned uid, ProtoShop::ShelfUnLockReq* req, ProtoShop::ShelfUnLockResp* resp)
{
	//校验货架是否已解锁至最大数
	unsigned shelf_max = ShopCfgWrap().GetShopInfoCfg().shelf_cnt(1);
	if(GetShelfCnt(uid) > shelf_max)
	{
		error_log("shelf count is maxed");
		throw std::runtime_error("shelf count is maxed");
	}

	/**********钻石消耗校验*****************/
	//配置中最小货架数
	unsigned shelf_min = ShopCfgWrap().GetShopInfoCfg().shelf_cnt(0);

	//获取当前已有的货架数
	unsigned cur_shelf_cnt = GetShelfCnt(uid);

	//按照公式计算消耗的钻石
	unsigned cost_diamond  = (cur_shelf_cnt - shelf_min + 1) * ShopCfgWrap().GetShopInfoCfg().shelf_base_price();

	if(cost_diamond != req->costdiamond())
	{
		error_log("param error. cost_diamond = %u,req->costdimond() = %u",cost_diamond,req->costdiamond());
		throw std::runtime_error("param error");
	}

	//校验通过
	DBCUserBaseWrap user(uid);
	user.CostCash(cost_diamond,"ShopShelfUnlock");
	DataCommon::CommonItemsCPP *common = resp->mutable_commons();
	DataCommon::BaseItemCPP *base = common->mutable_userbase()->add_baseitem();
	base->set_change(-cost_diamond);
	base->set_totalvalue(user.GetCash());
	base->set_type(type_cash);

	//解锁货架
	unsigned ud = DataShopManager::Instance()->GetNewShelfUd(uid);
	DataShop &shop = DataShopManager::Instance()->GetData(uid,ud);
	shop.SetMessage(resp->mutable_shop());

	//开启货架数量添加至任务表中
	LogicTaskManager::Instance()->AddTaskData(uid,task_of_shop_shelf,1);

	return 0;
}

int LogicShopManager::Process(unsigned uid, ProtoShop::ConfirmPaymentReq* req, ProtoShop::ConfirmPaymentResp* resp)
{
	unsigned ud = req->ud();
	unsigned type = 0;
	if(req->has_type())
		type = req->type();
	if(type != type_of_confirm_payment_normal && type != type_of_confirm_payment_viewad)
	{
		throw std::runtime_error("type_param_error");
	}

	bool is_exsit = DataShopManager::Instance()->IsExistItem(uid,ud);
	if(!is_exsit)
	{
		error_log("param error.item is not exsit ud = %u",ud);
		throw std::runtime_error("item is not exsit");
	}

	//获取商店中此货架的信息
	DataShop &shop = DataShopManager::Instance()->GetData(uid,ud);
	if(0 == shop.sell_flag)
	{
		error_log("param error.item don't selled ud = %u",ud);
		throw std::runtime_error("item don't selled");
	}

	//收钱
	unsigned coin = 0;
	if(type_of_confirm_payment_normal == type)
		coin = shop.props_price;
	else if(type_of_confirm_payment_viewad == type)
		coin = shop.props_price * 2;
	DBCUserBaseWrap user(uid);
	user.AddCoin(coin,"shop_item_selled");
	DataCommon::CommonItemsCPP *common = resp->mutable_commons();
	DataCommon::BaseItemCPP *base = common->mutable_userbase()->add_baseitem();
	base->set_change(coin);
	base->set_totalvalue(user.GetCoin());
	base->set_type(type_coin);

	//存储收获到的金币
	LogicShopSellCoinManager::Instance()->SaveSellCoin(uid,Time::GetGlobalTime(),coin);

	//将收获的金币添加至任务表中
	LogicTaskManager::Instance()->AddTaskData(uid,task_of_shop_coin,coin);

	//更新玩家当日商店收获的金币
	LogicMailDogManager::Instance()->UpdateMailDogData(uid,update_shop_income_daily,coin);

	//若果有广告,则删除对应广告
	if(1 == shop.ad_flag) {
		debug_log("confirm_money_delete_ad,uid=%u,shelf_ud=%u",uid,shop.id);
		LogicAdvertiseManager::Instance()->DelAd(shop.uid,shop.id);
	}

	//更新商店中此货架上的信息
	shop.ResetShopShelf();
	DataShopManager::Instance()->UpdateItem(shop);

	//设置返回消息
	shop.SetMessage(resp->mutable_shop());
	return 0;
}

int LogicShopManager::Process(unsigned uid, ProtoShop::PurchaseReq* req)
{
	unsigned other_uid = req->othuid();//对方uid
	unsigned shelf_ud = req->ud();//对应的货架ud
	DBCUserBaseWrap userwrap(uid);
	string buyerfig = userwrap.Obj().fig;
	string buyername    = userwrap.Obj().name;

	if(CMI->IsNeedConnectByUID(other_uid))
	{
		//获取商店货架信息
		ProtoShop::CSPurchaseReq* modifyReq = new ProtoShop::CSPurchaseReq;
		modifyReq->set_othuid(other_uid);
		modifyReq->set_ud(shelf_ud);
		modifyReq->set_propsid(req->propsid());
		modifyReq->set_propscnt(req->propscnt());
		modifyReq->set_propsprice(req->propsprice());
		modifyReq->set_sellflag(req->sellflag());
		modifyReq->set_buyeruid(uid);
		modifyReq->set_buyerfig(buyerfig);
		modifyReq->set_buyername(buyername);

		//购买前校验
		DataShop shop(uid,shelf_ud);
		shop.props_id = req->propsid();
		shop.props_cnt = req->propscnt();
		shop.props_price = req->propsprice();
		shop.sell_flag = req->sellflag();
		BeforeBuyShopItemCheck(uid, shop);

		//校验通过、发送请求数据
		int ret = BMI->BattleConnectNoReplyByUID(other_uid, modifyReq);

		//跨服购买添加动态消息 (若购买失败也会产生动态,故注释该处)
		//AddBuyDyInfoOverServer(uid,other_uid,shop.props_id,shop.props_price);

		return  ret;
	}

	ProtoShop::PurchaseResp* resp = new ProtoShop::PurchaseResp;

	try{
		//加载玩家数据
		OffUserSaveControl offuserctl(other_uid);

		//校验数据是否存在
		bool is_exsit = DataShopManager::Instance()->IsExistItem(other_uid,shelf_ud);
		if(!is_exsit)
		{
			error_log("param error.other_uid=%u,shelf_ud=%u",other_uid,shelf_ud);
			throw std::runtime_error("param error");
		}
		DataShop &shop = DataShopManager::Instance()->GetData(other_uid,shelf_ud);

		//-------购买前校验
		BeforeBuyShopItemCheck(uid, shop);

		//-------处理购买(扣金币、加物品)
		handleShopPurchase(uid,shop,resp);

		//---------修改对方商店信息
		shop.sell_flag = 1;
		shop.buyer_uid = uid;
		strncpy(shop.fig, buyerfig.c_str(), BASE_FIG_LEN-1);
		strncpy(shop.name, buyername.c_str(), BASE_NAME_LEN-1);

		DataShopManager::Instance()->UpdateItem(shop);

		//同服购买添加购买动态消息
		AddBuyDyInfo(uid,other_uid,shop.props_id,shop.props_price);

		//推送消息
		ProtoShop::PushShopMsg *msg = new ProtoShop::PushShopMsg;
		shop.SetMessage(msg->mutable_shop());
		msg->mutable_shop()->set_buyerfig(buyerfig);
		msg->mutable_shop()->set_buyername(buyername);
		LogicManager::Instance()->sendMsg(other_uid,msg);

		//添加返回信息
		resp->set_othuid(other_uid);
		shop.SetMessage(resp->mutable_othshop());

		//西山居上报
		if(LogicXsgReportManager::Instance()->IsWXPlatform(uid)){
			string tradeid;
			tradeid.append("buyer:").append(CTrans::UTOS(shop.buyer_uid)).append("myuid:").append(CTrans::UTOS(shop.uid)).append("ts:").append(CTrans::UTOS(Time::GetGlobalTime()));

			LogicXsgReportManager::Instance()->XSGShopTradeReport(uid,tradeid,"shop",shop.props_price,"jinbi",shop.props_id,shop.props_cnt);
		}
	}catch(const std::exception& e)
	{
		delete resp;
		error_log("purchase error %s", e.what());
		return R_ERROR;
	}

	return LMI->sendMsg(uid, resp)?0:R_ERROR;
}

int LogicShopManager::Process(unsigned uid, ProtoShop::BuyShopItemBySystemReq* req, ProtoShop::BuyShopItemBySystemResp* resp)
{
	unsigned ud = req->ud();
	bool is_exist = DataShopManager::Instance()->IsExistItem(uid,ud);
	if(!is_exist)
	{
		throw std::runtime_error("shelf_ud_is_not_exist");
	}

	DataShop & shop = DataShopManager::Instance()->GetData(uid,ud);
	if(shop.props_cnt == 0 || shop.props_id == 0 || shop.props_price == 0)
	{
		throw std::runtime_error("shelf_data_error");
	}

	shop.sell_flag = 1;
	shop.buyer_uid = 0;

	shop.SetMessage(resp->mutable_shop());

	//推送消息
	ProtoShop::PushShopMsg *msg = new ProtoShop::PushShopMsg;
	shop.SetMessage(msg->mutable_shop());
	LogicManager::Instance()->sendMsg(uid,msg);
	return 0;
}

void LogicShopManager::InviteUnlockShopShelf(unsigned uid)
{
	//获取最大货架数目
	unsigned shelf_cnt_max = ShopCfgWrap().GetShopInfoCfg().invite_unlock_shelf_max();

	//校验玩家货架数目是否已满
	unsigned cur_unlock_shelf_cnt = GetShelfCntByInvite(uid);
	if(cur_unlock_shelf_cnt >= shelf_cnt_max)
	{
		return;
	}

	unsigned ud = DataShopManager::Instance()->GetNewShelfUd(uid);
	DataShop shop = DataShopManager::Instance()->GetData(uid,ud);
	shop.vip_shelf_flag = 2;
	DataShopManager::Instance()->UpdateItem(shop);

	//推送消息
	if(UserManager::Instance()->IsOnline(uid))
	{
		ProtoShop::GetShopResp *msg = new ProtoShop::GetShopResp;
		vector<unsigned>shelfs;
		shelfs.clear();
		DataShopManager::Instance()->GetIndexs(uid,shelfs);
		for(int i = 0; i < shelfs.size(); i++)
		{
			DataShop & shop = DataShopManager::Instance()->GetDataByIndex(shelfs[i]);
			shop.SetMessage(msg->add_shop());
		}
		LMI->sendMsg(uid,msg);
	}
}

int LogicShopManager::BeforeBuyShopItemCheck(unsigned uid,DataShop &shop,bool firstCheck)
{
	//校验购买的物品是否已解锁
	DBCUserBaseWrap userwrap(uid);
	unsigned user_level = userwrap.Obj().level;
	unsigned unlock_level = ItemCfgWrap().GetPropsItem(shop.props_id).unlock_level();
	if(user_level < unlock_level)
		throw std::runtime_error("item is not unlock");

	//校验物品是否已出售
	if(shop.sell_flag == 1 && firstCheck == true)
	{
		error_log("param error.item is selled.shelf_ud=%u",shop.id);
		throw std::runtime_error("param error");
	}

	//货物货架里的物品信息
	unsigned props_id = shop.props_id;
	unsigned props_cnt = shop.props_cnt;
	int coin = shop.props_price;

	if(props_id == 0 || props_cnt == 0 || coin == 0)
	{
		error_log("param error.shop is empty");
		throw std::runtime_error("param error");
	}

	//校验剩余金币是否满足
	if(userwrap.GetCoin() < coin)
	{
		error_log("coin_is_not_enough.user_coin=%u,cost_coin=%u",userwrap.GetCoin(),coin);
		throw std::runtime_error("coin_is_not_enough");
	}

	return 0;
}

int LogicShopManager::handleShopPurchase(unsigned uid,const DataShop &shop,ProtoShop::PurchaseResp* resp)
{
	//扣除金币
	DBCUserBaseWrap user(uid);
	user.CostCoin(shop.props_price,"buy_shop_item");
	DataCommon::CommonItemsCPP *common = resp->mutable_commons();
	DataCommon::BaseItemCPP *base = common->mutable_userbase()->add_baseitem();

	int coin = shop.props_price;
	base->set_change(-coin);
	base->set_totalvalue(user.GetCoin());
	base->set_type(type_coin);

	//添加物品
	LogicPropsManager::Instance()->AddProps(uid,shop.props_id,shop.props_cnt,"buy_shop_item",resp->mutable_commons()->mutable_props());

	return 0;
}

int LogicShopManager::Process(ProtoShop::CSPurchaseReq* req)
{
	unsigned other_uid = req->othuid();
	unsigned shelf_ud  = req->ud();

	//加载玩家数据
	OffUserSaveControl offuserctl(other_uid);

	//校验数据是否存在
	bool is_exsit = DataShopManager::Instance()->IsExistItem(other_uid,shelf_ud);
	if(!is_exsit)
	{
		error_log("shopinfo_is_not_exsit.other_uid=%u,shelf_ud=%u",other_uid,shelf_ud);
		throw std::runtime_error("shopinfo_is_not_exsit");
	}
	DataShop &shop = DataShopManager::Instance()->GetData(other_uid,shelf_ud);

	//校验是否已购买
	if(shop.sell_flag == 1)
	{
		error_log("item_is_selled.item is selled.other_uid=%u,shelf_ud=%u",other_uid,shop.id);
		throw std::runtime_error("item_is_selled");
	}

	//校验前端参数是否正确
	unsigned props_id = req->propsid();
	unsigned props_cnt = req->propscnt();
	unsigned props_price = req->propsprice();
	unsigned sell_flag = req->sellflag();
	if(props_id != shop.props_id || props_cnt != shop.props_cnt || props_price != shop.props_price || sell_flag != shop.sell_flag)
	{
		error_log("param_error.checked_failed");
		throw std::runtime_error("param_error.checked_failed");
	}

	//修改商店信息
	shop.sell_flag = 1;
	shop.buyer_uid = req->buyeruid();
	strncpy(shop.fig, req->buyerfig().c_str(), BASE_FIG_LEN-1);
	strncpy(shop.name, req->buyername().c_str(), BASE_NAME_LEN-1);
	DataShopManager::Instance()->UpdateItem(shop);

	//消息推送
	ProtoShop::PushShopMsg *msg = new ProtoShop::PushShopMsg;
	shop.SetMessage(msg->mutable_shop());
	msg->mutable_shop()->set_buyerfig(req->buyerfig());
	msg->mutable_shop()->set_buyername(req->buyername());
	LogicManager::Instance()->sendMsg(other_uid,msg);

	//产生动态 同服
	AddBuyDyInfo(req->buyeruid(),other_uid,props_id,props_price);

	//发送消息给请求者
	ProtoShop::CSPurchaseResp* resp = new ProtoShop::CSPurchaseResp;
	shop.SetMessage(resp->mutable_visitedshop());
	resp->set_visituid(req->buyeruid());
	resp->set_visiteduid(req->othuid());
	BMI->BattleConnectNoReplyByUID(req->buyeruid(), resp);

	return 0;
}

int LogicShopManager::Process(ProtoShop::CSPurchaseResp* req)
{
	//处理前再次校验是否可以购买
	DataShop shopInfo;
	shopInfo.FromMessage(req->mutable_visitedshop());
	BeforeBuyShopItemCheck(req->visituid(), shopInfo,false);


	ProtoShop::PurchaseResp * resp = new ProtoShop::PurchaseResp;

	//-------处理购买(扣金币、加物品)
	try
	{
		handleShopPurchase(req->visituid(),shopInfo,resp);
	}
	catch(const std::exception &e)
	{
		delete resp;
		error_log("parchase failed:%s",e.what());
		return R_ERROR;
	}
	shopInfo.SetMessage(resp->mutable_othshop());
	resp->set_othuid(req->visiteduid());

	//-----推送信息返回
	LMI->sendMsg(req->visituid(),resp,false);

	//西山居上报
	if(LogicXsgReportManager::Instance()->IsWXPlatform(shopInfo.uid)){
		string tradeid;
		tradeid.append("buyer:").append(CTrans::UTOS(shopInfo.buyer_uid)).append("myuid:").append(CTrans::UTOS(shopInfo.uid)).append("ts:").append(CTrans::UTOS(Time::GetGlobalTime()));

		LogicXsgReportManager::Instance()->XSGShopTradeReport(shopInfo.uid,tradeid,"shop",shopInfo.props_price,"jinbi",shopInfo.props_id,shopInfo.props_cnt);
	}
	return 0;
}

int LogicShopManager::Process(unsigned uid, ProtoShop::VisitOtherShopReq* req)
{
	unsigned othuid = req->othuid();
	if(CMI->IsNeedConnectByUID(othuid))
	{
		ProtoShop::CSVisitOtherShopReq* m = new ProtoShop::CSVisitOtherShopReq;
		m->set_visituid(uid);
		m->set_visiteduid(othuid);
		return BMI->BattleConnectNoReplyByUID(othuid, m);
	}

	ProtoShop::VisitOtherShopResp* resp = new ProtoShop::VisitOtherShopResp;
	int ret = VisitOtherShop(othuid, resp);
	if(ret)
	{
		delete resp;
		return ret;
	}
	return LMI->sendMsg(uid, resp)?0:R_ERROR;

}

int LogicShopManager::Process(ProtoShop::CSVisitOtherShopReq* req)
{
	//查询自己的商店信息
	ProtoShop::CSVisitOtherShopResp* resp = new ProtoShop::CSVisitOtherShopResp;
	resp->set_visituid(req->visituid());
	VisitOtherShop(req->visiteduid(),resp->mutable_visitedshopresp());

	return BMI->BattleConnectNoReplyByUID(req->visituid(), resp);
}

int LogicShopManager::Process(ProtoShop::CSVisitOtherShopResp* resp)
{
	return LMI->sendMsg(resp->visituid(), resp->mutable_visitedshopresp(), false)?0:R_ERROR;
}

int LogicShopManager::VisitOtherShop(unsigned othuid, ProtoShop::VisitOtherShopResp * resp)
{
	//加载玩家数据
	OffUserSaveControl offuserctl(othuid);

	//获取玩家的商店数据
	//获取玩家货架信息
	std::vector<unsigned> indexs;
	DataShopManager::Instance()->GetIndexs(othuid, indexs);

	//遍历、返回信息
	for(int i = 0; i < indexs.size(); ++i)
	{
		DataShop &shop = DataShopManager::Instance()->GetDataByIndex(indexs[i]);
		ProtoShop::ShopCPP *msg = resp->add_othshop();

		shop.SetMessage(msg);

		if(shop.sell_flag == 1)
		{
			//添加购买者头像、名字信息
			string headUrl = ResourceManager::Instance()->GetHeadUrl(shop.buyer_uid);
			string name    = ResourceManager::Instance()->GetUserName(shop.buyer_uid);
			msg->set_buyername(name);
			msg->set_buyerfig(headUrl);
		}
	}

	resp->set_othuid(othuid);

	return 0;
}

int LogicShopManager::Process(unsigned uid, ProtoShop::ModifyShelfInfoReq* req, ProtoShop::ModifyShelfInfoResp* resp)
{

	//校验对应的货架信息是否存在
	unsigned shelf_ud  = req->ud();
	bool is_exsit    = DataShopManager::Instance()->IsExistItem(uid,shelf_ud);
	if(!is_exsit)
	{
		error_log("param error");
		throw std::runtime_error("param error");
	}

	//获取商店基本信息
	DataShop &shop   = DataShopManager::Instance()->GetData(uid,shelf_ud);

	//发广告
	if(1 == req->flag())
	{
		//校验是否已发过广告
		if(1 == shop.ad_flag)
		{
			error_log("data error");
			throw std::runtime_error("data error");
		}

		//校验广告cd
		DBCUserBaseWrap user(uid);
		if(Time::GetGlobalTime()  < user.GetCreateAdTs())
		{
			error_log("can't create ad");
			throw std::runtime_error("can't create ad");
		}


		//更新广告标志位、
		shop.ad_flag  = 1;
		DataShopManager::Instance()->UpdateItem(shop);

		//创建广告
		debug_log("modify_shelf_relase_ad,uid=%u,shelf_ud=%u",uid,shelf_ud);
		LogicAdvertiseManager::Instance()->CreateAd(uid,shelf_ud);

		//下一次创建广告的时间
		unsigned ad_cd_ts = AdCfgWrap().GetAdInfoCfg().adcdts();
		user.UpdateCreateAdTs(Time::GetGlobalTime() + ad_cd_ts);
		resp->set_ts(Time::GetGlobalTime() + ad_cd_ts);

		//将发广告添加至任务列表中
		LogicTaskManager::Instance()->AddTaskData(uid,task_of_create_ad,1);
		LogicMissionManager::Instance()->AddMission(uid,mission_of_create_ad,1);
	}
	else if(2 == req->flag())
	{
		//-----删除订单
		//扣除钻石
		DBCUserBaseWrap user(uid);
		unsigned cash  = AdCfgWrap().GetAdInfoCfg().delordercost();
		user.CostCash(cash,"delete_shop_item");
		DataCommon::CommonItemsCPP *common = resp->mutable_commons();
		DataCommon::BaseItemCPP *base = common->mutable_userbase()->add_baseitem();
		base->set_change(-cash);
		base->set_totalvalue(user.GetCash());
		base->set_type(type_cash);

		//如果有广告，则删除对应的广告信息
		if(1 == shop.ad_flag) {
			debug_log("modify_shelf_delete_ad,uid=%u,shelf_ud=%u",uid,shelf_ud);
			LogicAdvertiseManager::Instance()->DelAd(shop.uid,shop.id);
		}

		//商店货架信息重置
		shop.ResetShopShelf();
		DataShopManager::Instance()->UpdateItem(shop);

		//返回下一次发广告的ts
		resp->set_ts(user.GetCreateAdTs());
	}
	else
	{
		error_log("param error.flag = %u",req->flag());
		throw std::runtime_error("param error");
	}

	//返回货架信息
	shop.SetMessage(resp->mutable_shop());
	return 0;
}


int LogicShopManager::Process(unsigned uid, ProtoShop::ViewAdRecycleItemReq* req, ProtoShop::ViewAdRecycleItemResp* resp)
{
	//校验对应的货架信息是否存在
	unsigned shelf_ud  = req->ud();
	bool is_exsit    = DataShopManager::Instance()->IsExistItem(uid,shelf_ud);
	if(!is_exsit)
	{
		error_log("param error");
		throw std::runtime_error("param error");
	}

	//获取商店基本信息
	DataShop &shop   = DataShopManager::Instance()->GetData(uid,shelf_ud);

	if(1 == shop.sell_flag) {
		throw std::runtime_error("shop_item_is_already_selled");
	}

	if(shop.props_id == 0 || shop.props_cnt == 0)
	{
		throw std::runtime_error("shop_item_is_not_exsit");
	}
	//判定货仓、粮仓剩余空间是否足够
	const ConfigItem::PropItem itemcfg = ItemCfgWrap().GetPropsItem(shop.props_id);
	unsigned location = itemcfg.location();
	int restspace = LogicBuildManager::Instance()->GetStorageRestSpace(uid, location);
	if(shop.props_cnt > restspace)
	{
		throw std::runtime_error("warehouse_space_is_not_enough");
	}

	//获取物品信息、进行回补操作
	CommonGiftConfig::CommonModifyItem common;
	CommonGiftConfig::PropsItem * propsitem = common.add_props();
	propsitem->set_id(shop.props_id);
	propsitem->set_count(shop.props_cnt);
	LogicUserManager::Instance()->CommonProcess(uid,common,"view_ad_recycle_item",resp->mutable_commons());

	//如果有广告,则删除广告
	if(1 == shop.ad_flag) {
		LogicAdvertiseManager::Instance()->DelAd(shop.uid,shop.id);
	}

	//重置商店数据
	shop.ResetShopShelf();
	DataShopManager::Instance()->UpdateItem(shop);
	shop.SetMessage(resp->mutable_shop());
	return 0;
}

int LogicShopManager::UpdateAdFlag(unsigned uid,unsigned shelf_ud)
{

	//在此处理前有load档，故无需再重复load档。直接处理相关逻辑
	bool is_exsit = DataShopManager::Instance()->IsExistItem(uid,shelf_ud);
	if(!is_exsit)
	{
		error_log("shop_data_is_not_exsit.update ad flag failed uid =%u,shelf_ud=%u",uid,shelf_ud);
		throw std::runtime_error("shop_data_is_not_exsit");
	}

	//1.如果广告时间到,若货架物品无人购买,则选用田园老人来购买,并更新广告标记
	//2.如果货架物品已出售,则更新广告标记

	DataShop &shop = DataShopManager::Instance()->GetData(uid,shelf_ud);
	if(shop.sell_flag == 0)
	{
		shop.sell_flag = 1;
		shop.buyer_uid = 0;
		//判断用户是否在线，如果在线，推送消息
		bool is_online = UserManager::Instance()->IsOnline(uid);
		if(is_online)
		{
			ProtoShop::PushShopMsg *msg = new ProtoShop::PushShopMsg;
			shop.SetMessage(msg->mutable_shop());
			LogicManager::Instance()->sendMsg(uid,msg);
		}
	}

	shop.ad_flag   = 0;
	DataShopManager::Instance()->UpdateItem(shop);
	return 0;
}

bool LogicShopManager::AddBuyDyInfo(unsigned uid,unsigned other_uid,unsigned product_id,unsigned coin)
{
	//uid:访问者,other_uid:被访问者,商品product_id,金币数coin,在好友庄园购买东西会让被访问者增加一条被购买动态
	DynamicInfoAttach *pattach = new DynamicInfoAttach;
	pattach->op_uid = uid;
	pattach->product_id = product_id;
	pattach->coin = coin;
	if(LogicDynamicInfoManager::Instance()->ProduceOneDyInfo(other_uid,TYPE_DY_BUY,pattach))
	{
		return true;
	}
	return false;
}

bool LogicShopManager::AddBuyDyInfoOverServer(unsigned uid,unsigned other_uid,unsigned product_id,unsigned coin)
{
	ProtoDynamicInfo::RequestOtherUserMakeDy * msg = new ProtoDynamicInfo::RequestOtherUserMakeDy;
	string ret;
	msg->set_myuid(uid);
	msg->set_othuid(other_uid);
	msg->set_typeid_(TYPE_DY_BUY);
	msg->set_productid(product_id);
	msg->set_coin(coin);
	msg->set_words(ret);

	return BMI->BattleConnectNoReplyByUID(other_uid, msg);
}
