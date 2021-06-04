#include "ServerInc.h"
#include "BattleServer.h"

//前端主动获取订单消息 同服
int LogicFriendOrderManager::Process(unsigned uid, ProtoFriendOrder::GetFriendOrderReq *reqmsg, ProtoFriendOrder::GetFriendOrderResp * respmsg)
{
	unsigned Fo_show_max = PER_USER_MAX_FRIEND_ORDER + MAX_FO_BASKET_SIZE;	//最多订单条数
	unsigned Fo_show_cnt = 0;												//实际可以展示的订单条数

	FoInfoItem* foItem = NULL;
	foItem = (FoInfoItem*)malloc(sizeof(FoInfoItem)*Fo_show_max);
	if(foItem == NULL)
	{
		error_log("init is failed");
		throw std::runtime_error("init is failed");
	}

	//获取订单条数
	Fo_show_cnt = GetFoInfo(uid,foItem,Fo_show_max);

	//设置返回信息
	for(unsigned index = 0; index < Fo_show_cnt; ++index)
	{
		SetFoRespMsg(foItem[index],respmsg->add_arrayfoinfo(),index);
	}

	delete [] foItem;
	return 0;
}

int LogicFriendOrderManager::GetFoInfo(unsigned uid,FoInfoItem foItem[],unsigned max_count)
{

	map<unsigned,deque<unsigned> > & foMap = FriendOrderManager::Instance()->GetFoInfoMap();

	map<unsigned,deque<unsigned> >::iterator it_map = foMap.find(uid);
	if(it_map == foMap.end())	//no uid
	{
		return 0;
	}
	if(!it_map->second.size())	//no dynamicInfo
	{
		return 0;
	}

	unsigned info_size = it_map->second.size();
	if(info_size > max_count)
	{
		info_size = max_count;
	}
	for(unsigned j = 0;j < info_size;++j)		//从0开始遍历deque
	{
		unsigned foidx = it_map->second[j];
		if(MAX_FRIEND_ORDER_NUM == foidx)
		{
			FoInfoItem fo_free;
			fo_free.status = STATUS_FO_FREE_BASKET;
			foItem[j].Set(fo_free);
		}
		else
		{
			foItem[j].Set(FriendOrderManager::Instance()->GetFoInfoItem(foidx));
		}

	}
	return info_size;

}

bool LogicFriendOrderManager::SetFoRespMsg(const FoInfoItem& adItem,ProtoFriendOrder::FriendOrderInfo *msg,unsigned index)
{
	msg->set_index(index);

	msg->set_status(adItem.status);
	msg->set_sourceid(adItem.sourceId);
	msg->set_coin(adItem.coin);
	msg->set_count(adItem.count);
	msg->set_deadtime(adItem.deadtime + 60);	//后台deadtime比前端显示要早一分钟
	msg->set_productid(adItem.productId);
	msg->set_senderuid(adItem.senderUid);

	if(adItem.status == STATUS_FO_SELL_OUT)
	{
		msg->set_helperuid(adItem.helperuid);
	}

	return true;
}
//发起订单请求
int LogicFriendOrderManager::Process(unsigned uid,ProtoFriendOrder::SendFriendOrderReq *resqmsg,ProtoFriendOrder::SendFriendOrderResp *respmsg)
{
	//获取前端传参
	unsigned basket = resqmsg->basket();
	unsigned productId = resqmsg->productid();
	unsigned count = resqmsg->count();
	unsigned coin = resqmsg->coin();

	unsigned array_size = resqmsg->arrayuid_size();
	vector<unsigned> vec_uids;
	for(unsigned idx = 0;idx < array_size;++idx)
	{
		vec_uids.push_back(resqmsg->arrayuid(idx));
	}

	//设置源订单信息
	FoInfoItem sourcefo;
	sourcefo.status = STATUS_FO_WAIT_BUY;
	sourcefo.sourceId = basket;
	sourcefo.coin = coin;
	sourcefo.count = count;
	sourcefo.productId = productId;
	sourcefo.owneruid = uid;
	sourcefo.senderUid = uid;
	sourcefo.deadtime = Time::GetGlobalTime() + FO_CD_OVER_TIME;

	//返回栏位号给前端
	respmsg->set_basket(basket);
	//返回新订单给前端
	ProtoFriendOrder::FriendOrderInfo *msg = respmsg->mutable_newsourcefo();
	msg->set_status(sourcefo.status);
	msg->set_senderuid(sourcefo.senderUid);
	msg->set_deadtime(sourcefo.deadtime + 60);
	msg->set_sourceid(sourcefo.sourceId);
	msg->set_productid(sourcefo.productId);
	msg->set_count(sourcefo.count);
	msg->set_coin(sourcefo.coin);
	msg->set_index(sourcefo.sourceId);

	//校验金币数是否足够
	DBCUserBaseWrap userwrap(uid);
	if(userwrap.GetCoin() < coin)
	{
		error_log("coin_is_not_enough.user_coin=%u,cost_coin=%u",userwrap.GetCoin(),coin);
		throw std::runtime_error("coin_is_not_enough");
	}

	//扣除金币并返回数据给前端
	DBCUserBaseWrap user(uid);
	user.CostCoin(coin,"send_friend_order");
	DataCommon::CommonItemsCPP *common = respmsg->mutable_commons();
	DataCommon::BaseItemCPP *base = common->mutable_userbase()->add_baseitem();

	base->set_change(-coin);
	base->set_totalvalue(user.GetCoin());
	base->set_type(type_coin);

	//共享内存中增加一条源订单
	FriendOrderManager::Instance()->AddFoInfo(uid,sourcefo,basket);

	//增加好友订单 考虑跨服和同服
	for(unsigned jdx = 0;jdx < vec_uids.size();++jdx)
	{
		unsigned othuid = vec_uids[jdx];
		sourcefo.owneruid = othuid;
		sourcefo.status = STATUS_FO_UNDEFINED;

		if(CMI->IsNeedConnectByUID(othuid))		//跨服
		{
			AddNormalFoOverServer(uid,othuid,sourcefo);
			AddBuyDyInfoOverServer(uid,othuid,productId,count);
		}
		else									//同服
		{
			FriendOrderManager::Instance()->AddFoInfo(othuid,sourcefo,-1);
			AddBuyDyInfo(uid,othuid,productId,count);
		}
	}

	return 0;
}

bool LogicFriendOrderManager::AddNormalFoOverServer(unsigned senderUid,unsigned receiverUid,FoInfoItem & foitem)
{
	ProtoFriendOrder::SendOtherServerReq *msg = new ProtoFriendOrder::SendOtherServerReq;
	msg->set_coin(foitem.coin);
	msg->set_count(foitem.count);
	msg->set_deadtime(foitem.deadtime);
	msg->set_productid(foitem.productId);
	msg->set_senderuid(foitem.senderUid);
	msg->set_sourceid(foitem.sourceId);
	msg->set_status(foitem.status);
	msg->set_receiveruid(receiverUid);

	return BMI->BattleConnectNoReplyByUID(receiverUid, msg);
}
//跨服 接收好友订单
int LogicFriendOrderManager::Process(ProtoFriendOrder::SendOtherServerReq *resqmsg)
{

	FoInfoItem foitem;
	foitem.coin = resqmsg->coin();
	foitem.count = resqmsg->count();
	foitem.deadtime = resqmsg->deadtime();
	foitem.senderUid = resqmsg->senderuid();
	foitem.productId = resqmsg->productid();
	foitem.sourceId = resqmsg->sourceid();
	foitem.owneruid = resqmsg->receiveruid();

	FriendOrderManager::Instance()->AddFoInfo(foitem.owneruid,foitem,-1);
	return 0;
}

bool LogicFriendOrderManager::CheckClearFoInfo()
{
	return FriendOrderManager::Instance()->CheckClearFo();
}

bool LogicFriendOrderManager::CheckRecycleOldSourceFo()
{
	return FriendOrderManager::Instance()->CheckRecyleSourceFo();
}

//点击好友订单中的某个订单
int LogicFriendOrderManager::Process(unsigned uid,ProtoFriendOrder::ClickFriendOrderReq *resqmsg)
{
	unsigned index = resqmsg->index();
	unsigned status = resqmsg->status();

	map<unsigned,deque<unsigned> > & foMap = FriendOrderManager::Instance()->GetFoInfoMap();

	map<unsigned,deque<unsigned> >::iterator it_map = foMap.find(uid);
	if(it_map == foMap.end())				//no uid
	{
		return 1;
	}
	if(it_map->second.size() <= index)		//no index
	{
		return 1;
	}

	unsigned foidx = it_map->second[index];
	if(foidx < 0 || foidx >= MAX_FRIEND_ORDER_NUM)		//wrong foidx
	{
		return 1;
	}

	FoInfoItem &foitem = FriendOrderManager::Instance()->GetFoInfoItem(foidx);
	unsigned senderUid = foitem.senderUid;
	unsigned sourceId = foitem.sourceId;

	unsigned sourceStatus = STATUS_FO_WAIT_BUY;
	if(CMI->IsNeedConnectByUID(senderUid))		//跨服
	{
		//跨服查询源订单状态
		ProtoFriendOrder::RecallSourceFoReq *resqmsg = new ProtoFriendOrder::RecallSourceFoReq;
		resqmsg->set_myuid(uid);
		resqmsg->set_myindex(index);
		resqmsg->set_sourceid(sourceId);
		resqmsg->set_senderuid(senderUid);

		return BMI->BattleConnectNoReplyByUID(senderUid, resqmsg);
	}
	else										//同服
	{
		sourceStatus = FriendOrderManager::Instance()->GetSourceFoStatus(senderUid,sourceId);
	}

	unsigned newStatus = STATUS_FO_CAN_BUY;
	ProtoFriendOrder::ClickFriendOrderResp *respmsg = new ProtoFriendOrder::ClickFriendOrderResp;
	if(sourceStatus == STATUS_FO_FREE_BASKET || sourceStatus == STATUS_FO_SELL_OUT || sourceStatus == STATUS_FO_FREEZE)
	{
		newStatus = STATUS_FO_OTHER_BOUGHT;
	}
	else if(sourceStatus == STATUS_FO_WAIT_BUY)
	{
		newStatus = STATUS_FO_CAN_BUY;
	}
	else if(sourceStatus == STATUS_FO_OVER_DATE)
	{
		newStatus = STATUS_FO_TIME_OUT;
	}

	//一方面把好友订单的状态改成newStatus
	FriendOrderManager::Instance()->ChangeShmFoStatus(uid,index,newStatus);		//改变共享内存中订单的状态

	//另一方面将newStatus返回给前端显示
	respmsg->set_index(index);
	respmsg->set_newstatus(newStatus);
	LogicManager::Instance()->sendMsg(uid,respmsg);
	return 0;
}
//跨服 更新好友订单状态
int LogicFriendOrderManager::Process(ProtoFriendOrder::ChangeFoStatusReq *resqmsg)
{
	unsigned myuid = resqmsg->myuid();
	unsigned myindex = resqmsg->myindex();
	unsigned newStatus = resqmsg->newstatus();

	FriendOrderManager::Instance()->ChangeShmFoStatus(myuid,myindex,newStatus);		//改变共享内存中订单的状态

	return 0;
}
//跨服 查询源订单状态
int LogicFriendOrderManager::Process(ProtoFriendOrder::RecallSourceFoReq *resqmsg)
{
	unsigned myuid = resqmsg->myuid();
	unsigned myindex = resqmsg->myindex();
	unsigned senderuid = resqmsg->senderuid();
	unsigned index = resqmsg->sourceid();

	map<unsigned,deque<unsigned> > & foMap = FriendOrderManager::Instance()->GetFoInfoMap();

	map<unsigned,deque<unsigned> >::iterator it_map = foMap.find(senderuid);
	if(it_map == foMap.end())				//no uid
	{
		return 1;
	}
	if(it_map->second.size() <= index)		//no index
	{
		return 1;
	}

	unsigned foidx = it_map->second[index];
	if(foidx < 0 || foidx >= MAX_FRIEND_ORDER_NUM)		//wrong foidx
	{
		return 1;
	}

	FoInfoItem &foitem = FriendOrderManager::Instance()->GetFoInfoItem(foidx);
	unsigned senderUid = foitem.senderUid;
	unsigned sourceId = foitem.sourceId;
	if(senderuid != senderUid)
	{
		return 1;										//内存数据与共享内存数据不匹配
	}

	unsigned sourceStatus = FriendOrderManager::Instance()->GetSourceFoStatus(senderUid,sourceId);

	unsigned newStatus = STATUS_FO_CAN_BUY;
	ProtoFriendOrder::ClickFriendOrderResp *respmsg = new ProtoFriendOrder::ClickFriendOrderResp;
	if(sourceStatus == STATUS_FO_FREE_BASKET || sourceStatus == STATUS_FO_SELL_OUT || sourceStatus == STATUS_FO_FREEZE)
	{
		newStatus = STATUS_FO_OTHER_BOUGHT;
	}
	else if(sourceStatus == STATUS_FO_WAIT_BUY)
	{
		newStatus = STATUS_FO_CAN_BUY;
	}
	else if(sourceStatus == STATUS_FO_OVER_DATE)
	{
		newStatus = STATUS_FO_TIME_OUT;
	}

	//一方面跨服返回新的状态,改变相应好友订单的状态
	ProtoFriendOrder::ChangeFoStatusReq *msg = new ProtoFriendOrder::ChangeFoStatusReq;
	msg->set_myuid(myuid);
	msg->set_myindex(myindex);
	msg->set_newstatus(newStatus);
	BMI->BattleConnectNoReplyByUID(myuid, msg);

	//另一方面将newStatus直接返回给前端显示
	respmsg->set_index(myindex);
	respmsg->set_newstatus(newStatus);
	LogicManager::Instance()->sendMsg(myuid,respmsg);

	return 0;

}
//点击“没问题”按钮 购买该条好友订单
int LogicFriendOrderManager::Process(unsigned uid,ProtoFriendOrder::BuyFriendOrderReq *resqmsg)
{
	unsigned index = resqmsg->index();
	unsigned status = resqmsg->status();
	unsigned sourceid = resqmsg->sourceid();
	unsigned senderuid = resqmsg->senderuid();
	unsigned productid = resqmsg->productid();
	unsigned count = resqmsg->count();
	unsigned coin = resqmsg->coin();

	map<unsigned,deque<unsigned> > & foMap = FriendOrderManager::Instance()->GetFoInfoMap();

	map<unsigned,deque<unsigned> >::iterator it_map = foMap.find(uid);
	if(it_map == foMap.end())				//no uid
	{
		return 1;
	}
	if(it_map->second.size() <= index)
	{
		return 1;
	}

	unsigned foidx = it_map->second[index];
	FoInfoItem &foitem = FriendOrderManager::Instance()->GetFoInfoItem(foidx);
	unsigned senderUid = foitem.senderUid;
	unsigned sourceId = foitem.sourceId;
	if(senderuid != senderUid || sourceId != sourceid)					//数据不一致
	{
		return 1;
	}

	unsigned sourceStatus = STATUS_FO_WAIT_BUY;
	if(CMI->IsNeedConnectByUID(senderUid))		//跨服
	{
		//跨服查询源订单是否可以购买
		ProtoFriendOrder::RecallCanBuyFoReq *resqmsg = new ProtoFriendOrder::RecallCanBuyFoReq;
		resqmsg->set_myuid(uid);
		resqmsg->set_myindex(index);
		resqmsg->set_sourceid(sourceId);
		resqmsg->set_senderuid(senderUid);

		return BMI->BattleConnectNoReplyByUID(senderUid, resqmsg);
	}
	else										//同服
	{
		sourceStatus = FriendOrderManager::Instance()->GetSourceFoStatus(senderUid,sourceId);
	}

	if(sourceStatus == STATUS_FO_WAIT_BUY)		//源订单的状态必须是等待购买
	{
		//扣除物品，获得金币
		/*--------进行参数校验-----------*/
		//物品校验
		unsigned props_ud = DataItemManager::Instance()->GetPropsUd(uid,productid);
		if(-1 == props_ud)
		{
			error_log("param error.not exsit item props_id =%u",productid);
			throw std::runtime_error("param error");
		}
		//数量校验
		unsigned pros_ct = DataItemManager::Instance()->GetItemCount(uid,productid);
		if(pros_ct < count)
		{
			error_log("param error.not enough item props_id =%u props_ct=%u",productid,count);
			throw std::runtime_error("param error");
		}
		//---------校验通过
		//物品减少
		ProtoFriendOrder::BuyFriendOrderResp *msg = new ProtoFriendOrder::BuyFriendOrderResp;
		string reason = "friend_Order_help";
		LogicPropsManager::Instance()->CostProps(uid,props_ud,productid,count,reason,msg->mutable_commons()->mutable_props());

		//金币增加
		DBCUserBaseWrap user(uid);
		user.AddCoin(coin,"friend_Order_gain");
		DataCommon::CommonItemsCPP *common = msg->mutable_commons();
		DataCommon::BaseItemCPP *base = common->mutable_userbase()->add_baseitem();
		base->set_change(coin);
		base->set_totalvalue(user.GetCoin());
		base->set_type(type_coin);

		//改变源订单的状态
		FriendOrderManager::Instance()->ChangeShmFoStatus(senderUid,sourceid,STATUS_FO_SELL_OUT);

		//订单数据增加helperuid字段
		FriendOrderManager::Instance()->AddHelperUid(senderUid,sourceid,uid);

		//改变好友订单的状态
		FriendOrderManager::Instance()->ChangeShmFoStatus(uid,index,STATUS_FO_BUY_SUCCESS);

		//给前端推送返回
		msg->set_index(index);
		msg->set_newstatus(STATUS_FO_BUY_SUCCESS);
		LogicManager::Instance()->sendMsg(uid,msg);
	}
	else
	{
		//改变好友订单的状态
		FriendOrderManager::Instance()->ChangeShmFoStatus(uid,index,STATUS_FO_OTHER_BOUGHT);

		//给前端推送返回
		ProtoFriendOrder::BuyFriendOrderResp *msg = new ProtoFriendOrder::BuyFriendOrderResp;
		msg->set_index(index);
		msg->set_newstatus(STATUS_FO_OTHER_BOUGHT);
		LogicManager::Instance()->sendMsg(uid,msg);
	}

	return 0;
}
//跨服 购买该条好友订单
int LogicFriendOrderManager::Process(ProtoFriendOrder::RecallCanBuyFoReq *resqmsg)
{
	unsigned myuid = resqmsg->myuid();
	unsigned myindex = resqmsg->myindex();
	unsigned senderuid = resqmsg->senderuid();
	unsigned sindex = resqmsg->sourceid();

	map<unsigned,deque<unsigned> > & foMap = FriendOrderManager::Instance()->GetFoInfoMap();

	map<unsigned,deque<unsigned> >::iterator it_map = foMap.find(senderuid);
	if(it_map == foMap.end())				//no uid
	{
		return 1;
	}
	if(it_map->second.size() <= sindex)		//no index
	{
		return 1;
	}

	unsigned foidx = it_map->second[sindex];
	if(foidx < 0 || foidx >= MAX_FRIEND_ORDER_NUM)		//wrong foidx
	{
		return 1;
	}

	FoInfoItem &foitem = FriendOrderManager::Instance()->GetFoInfoItem(foidx);
	unsigned senderUid = foitem.senderUid;
	unsigned sourceId = foitem.sourceId;
	unsigned productid = foitem.productId;
	unsigned count = foitem.count;
	unsigned coin = foitem.coin;

	if(senderuid != senderUid || sindex != sourceId)
	{
		return 1;										//内存数据与共享内存数据不匹配
	}

	unsigned sourceStatus = FriendOrderManager::Instance()->GetSourceFoStatus(senderUid,sourceId);
	unsigned buy_result = 0;
	if(sourceStatus == STATUS_FO_WAIT_BUY)		//源订单的状态必须是等待购买
	{
		//源订单可以购买 修改源订单的状态
		FriendOrderManager::Instance()->ChangeShmFoStatus(senderuid,sourceId,STATUS_FO_SELL_OUT);

		//订单数据增加helperuid字段
		FriendOrderManager::Instance()->AddHelperUid(senderuid,sourceId,myuid);

		buy_result = 1;
	}
	else
	{
		//源订单不可购买,本次购买失败
		buy_result = 2;
	}

	ProtoFriendOrder::AnswerWhetherCanBuyReq *msg = new ProtoFriendOrder::AnswerWhetherCanBuyReq;
	msg->set_result(buy_result);
	msg->set_myindex(myindex);
	msg->set_myuid(myuid);
	msg->set_sourceid(sindex);
	msg->set_senderuid(senderuid);

	BMI->BattleConnectNoReplyByUID(myuid, msg);

	return 0;
}
//跨服 处理购买成功与否
int LogicFriendOrderManager::Process(ProtoFriendOrder::AnswerWhetherCanBuyReq *resqmsg)
{
	unsigned buy_result = resqmsg->result();
	unsigned uid = resqmsg->myuid();
	unsigned index = resqmsg->myindex();
	unsigned senderuid = resqmsg->senderuid();
	unsigned sourceid = resqmsg->sourceid();

	map<unsigned,deque<unsigned> > & foMap = FriendOrderManager::Instance()->GetFoInfoMap();

	map<unsigned,deque<unsigned> >::iterator it_map = foMap.find(uid);
	if(it_map == foMap.end())				//no uid
	{
		return 1;
	}
	if(it_map->second.size() <= index)
	{
		return 1;
	}

	unsigned foidx = it_map->second[index];
	FoInfoItem &foitem = FriendOrderManager::Instance()->GetFoInfoItem(foidx);
	unsigned senderUid = foitem.senderUid;
	unsigned sourceId = foitem.sourceId;
	unsigned productid = foitem.productId;
	unsigned coin = foitem.coin;
	unsigned count = foitem.count;

	if(senderuid != senderUid || sourceId != sourceid)					//数据不一致
	{
		return 1;
	}

	if(1 == buy_result)			//购买成功
	{
		//扣除物品，获得金币
		/*--------进行参数校验-----------*/
		//物品校验
		unsigned props_ud = DataItemManager::Instance()->GetPropsUd(uid,productid);
		if((unsigned)-1 == props_ud)
		{
			error_log("param error.not exsit item props_id =%u",productid);
			throw std::runtime_error("param error");
		}
		//数量校验
		unsigned pros_ct = DataItemManager::Instance()->GetItemCount(uid,productid);
		if(pros_ct < count)
		{
			error_log("param error.not enough item props_id =%u props_ct=%u",productid,count);
			throw std::runtime_error("param error");
		}
		//---------校验通过
		//物品减少
		ProtoFriendOrder::BuyFriendOrderResp *msg = new ProtoFriendOrder::BuyFriendOrderResp;
		string reason = "friend_Order_help";
		LogicPropsManager::Instance()->CostProps(uid,props_ud,productid,count,reason,msg->mutable_commons()->mutable_props());

		//金币增加
		DBCUserBaseWrap user(uid);
		user.AddCoin(coin,"friend_Order_gain");
		DataCommon::CommonItemsCPP *common = msg->mutable_commons();
		DataCommon::BaseItemCPP *base = common->mutable_userbase()->add_baseitem();
		base->set_change(coin);
		base->set_totalvalue(user.GetCoin());
		base->set_type(type_coin);

		//改变好友订单的状态
		FriendOrderManager::Instance()->ChangeShmFoStatus(uid,index,STATUS_FO_BUY_SUCCESS);

		//给前端推送返回
		msg->set_index(index);
		msg->set_newstatus(STATUS_FO_BUY_SUCCESS);
		LogicManager::Instance()->sendMsg(uid,msg);
	}
	else if(2 == buy_result)	//购买失败
	{
		//改变好友订单的状态
		FriendOrderManager::Instance()->ChangeShmFoStatus(uid,index,STATUS_FO_OTHER_BOUGHT);

		//给前端推送返回
		ProtoFriendOrder::BuyFriendOrderResp *msg = new ProtoFriendOrder::BuyFriendOrderResp;
		msg->set_index(index);
		msg->set_newstatus(STATUS_FO_OTHER_BOUGHT);
		LogicManager::Instance()->sendMsg(uid,msg);
	}
	else
	{
		return 1;
	}


	return 0;
}

//花费钻石清除冷却时间或者删除正在发送的订单
int LogicFriendOrderManager::Process(unsigned uid,ProtoFriendOrder::CostDiamondReq *resqmsg,ProtoFriendOrder::CostDiamondResp *respmsg)
{
	unsigned basket = resqmsg->basket();
	unsigned status = resqmsg->status();
	unsigned cost_diamond = resqmsg->diamonds();

	map<unsigned,deque<unsigned> > & foMap = FriendOrderManager::Instance()->GetFoInfoMap();

	map<unsigned,deque<unsigned> >::iterator it_map = foMap.find(uid);
	if(it_map == foMap.end())				//no uid
	{
		respmsg->set_basket(basket);
		respmsg->set_newstatus(STATUS_FO_FREE_BASKET);
		return 0;
	}
	if(it_map->second.size() <= basket)
	{
		respmsg->set_basket(basket);
		respmsg->set_newstatus(STATUS_FO_FREE_BASKET);
		return 0;
	}

	unsigned sourceStatus = FriendOrderManager::Instance()->GetSourceFoStatus(uid,basket);
	if(status == STATUS_FO_WAIT_BUY || status == STATUS_FO_FREEZE)			//删除正在发送的订单
	{
		if(sourceStatus != status)
		{
			throw runtime_error("error_status");
		}

		//扣除钻石
		DBCUserBaseWrap user(uid);
		user.CostCash(cost_diamond,"friend_order_unlock_basket");
		DataCommon::CommonItemsCPP *common = respmsg->mutable_commons();
		DataCommon::BaseItemCPP *base = common->mutable_userbase()->add_baseitem();
		base->set_change(-cost_diamond);
		base->set_totalvalue(user.GetCash());
		base->set_type(type_cash);

		if(status == STATUS_FO_WAIT_BUY)	//回收金币
		{
			unsigned foidx = it_map->second[basket];
			FoInfoItem &foitem = FriendOrderManager::Instance()->GetFoInfoItem(foidx);
			unsigned coin = foitem.coin;
			//金币增加
			DBCUserBaseWrap user(uid);
			user.AddCoin(coin,"friend_Order_recyle");
			DataCommon::CommonItemsCPP *common = respmsg->mutable_commons();
			DataCommon::BaseItemCPP *base = common->mutable_userbase()->add_baseitem();
			base->set_change(coin);
			base->set_totalvalue(user.GetCoin());
			base->set_type(type_coin);
		}

		//删除源订单
		if(!FriendOrderManager::Instance()->DeleteOneSourceOrder(uid,basket))
		{
			respmsg->set_basket(basket);
			respmsg->set_newstatus(STATUS_FO_FREE_BASKET);
			return 0;
		}
	}
	else
	{
		throw runtime_error("error_status");
	}

	respmsg->set_basket(basket);
	respmsg->set_newstatus(STATUS_FO_FREE_BASKET);

	return 0;
}
//回收发出的订单 后台给玩家增加物品，并改变该条源订单为冷却状态或者直接删除该条订单
int LogicFriendOrderManager::Process(unsigned uid,ProtoFriendOrder::GetOrderRewardsReq *resqmsg,ProtoFriendOrder::GetOrderRewardsResp *respmsg)
{
	unsigned basket = resqmsg->basket();
	unsigned status = resqmsg->status();
	unsigned produceId = resqmsg->productid();
	unsigned count = resqmsg->count();
	unsigned coin = resqmsg->coin();

	map<unsigned,deque<unsigned> > & foMap = FriendOrderManager::Instance()->GetFoInfoMap();

	map<unsigned,deque<unsigned> >::iterator it_map = foMap.find(uid);
	if(it_map == foMap.end())				//no uid
	{
		return 1;
	}
	if(it_map->second.size() <= basket)
	{
		return 1;
	}

	unsigned sourceStatus = FriendOrderManager::Instance()->GetSourceFoStatus(uid,basket);

	if(status == STATUS_FO_OVER_DATE || status == STATUS_FO_SELL_OUT)			//回收订单
	{
		if(sourceStatus != status)				//状态校验
		{
			return 1;
		}

		//获得金币，扣除物品
		if(status == STATUS_FO_OVER_DATE)		//回收金币
		{
			DBCUserBaseWrap user(uid);
			user.AddCoin(coin,"friend_order_time_out_recycle");
			DataCommon::CommonItemsCPP *common = respmsg->mutable_commons();
			DataCommon::BaseItemCPP *base = common->mutable_userbase()->add_baseitem();
			base->set_change(coin);
			base->set_totalvalue(user.GetCoin());
			base->set_type(type_coin);
		}
		else									//获得物品
		{
			LogicPropsManager::Instance()->AddProps(uid,produceId,count,"friend_order_gain_props",respmsg->mutable_commons()->mutable_props());
		}

	}
	else
	{
		return 1;
	}

	respmsg->set_basket(basket);
	unsigned newStatus = STATUS_FO_FREE_BASKET;

	if(sourceStatus == STATUS_FO_SELL_OUT)		//判断源订单将变成空闲或者冷却
	{
		unsigned foidx = it_map->second[basket];
		if(foidx < 0 || foidx >= MAX_FRIEND_ORDER_NUM)		//wrong foidx
		{
			return 1;
		}
		FoInfoItem &foitem = FriendOrderManager::Instance()->GetFoInfoItem(foidx);
		unsigned now = Time::GetGlobalTime();
		if(now > foitem.deadtime)				//超时,直接删除
		{
			FriendOrderManager::Instance()->DeleteOneSourceOrder(uid,basket);
			newStatus = STATUS_FO_FREE_BASKET;
		}
		else									//未超时，进入冷却状态
		{
			newStatus = STATUS_FO_FREEZE;
			FriendOrderManager::Instance()->ChangeShmFoStatus(uid,basket,newStatus);
			respmsg->set_deadtime(foitem.deadtime + 60);
		}
	}
	else	//源订单已超时 直接删除
	{
		FriendOrderManager::Instance()->DeleteOneSourceOrder(uid,basket);
		newStatus = STATUS_FO_FREE_BASKET;
	}

	respmsg->set_newstatus(newStatus);

	return 0;
}

bool LogicFriendOrderManager::AddBuyDyInfo(unsigned uid,unsigned other_uid,unsigned product_id,unsigned coin)
{
	//uid:访问者,other_uid:被访问者,商品product_id,金币数coin,在好友庄园购买东西会让被访问者增加一条被购买动态
	DynamicInfoAttach *pattach = new DynamicInfoAttach;
	pattach->op_uid = uid;
	pattach->product_id = product_id;
	pattach->coin = coin;
	if(LogicDynamicInfoManager::Instance()->ProduceOneDyInfo(other_uid,TYPE_DY_HELP_BUY,pattach))
	{
		return true;
	}
	return false;
}

bool LogicFriendOrderManager::AddBuyDyInfoOverServer(unsigned uid,unsigned other_uid,unsigned product_id,unsigned coin)
{
	ProtoDynamicInfo::RequestOtherUserMakeDy * msg = new ProtoDynamicInfo::RequestOtherUserMakeDy;
	string ret;
	msg->set_myuid(uid);
	msg->set_othuid(other_uid);
	msg->set_typeid_(TYPE_DY_HELP_BUY);
	msg->set_productid(product_id);
	msg->set_coin(coin);
	msg->set_words(ret);

	return BMI->BattleConnectNoReplyByUID(other_uid, msg);
}
