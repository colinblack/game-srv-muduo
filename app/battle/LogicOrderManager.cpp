/*
 * LogicOrderManager.cpp
 *
 *  Created on: 2018-3-1
 *      Author: Administrator
 */
#include "LogicOrderManager.h"
#include "ServerInc.h"
#include "TruckManager.h"

#define INIT_UNLOCK_SLOT_COUNT 2

void DataOrderRoutine::CheckUd(unsigned orderud)
{
	//先调用基类的方法
	DataRoutineBase::CheckUd(orderud);

	if (!DataOrderManager::Instance()->IsExistItem(uid_, orderud))
	{
		error_log("order is not exist. uid=%u,orderid=%u", uid_, orderud);
		throw runtime_error("build_not_exist");
	}

	//获取建筑数据
	DataOrder & order = DataOrderManager::Instance()->GetData(uid_, orderud);

	if(0 == order.end_ts)
	{
		error_log("order not waiting. uid=%u,orderid=%u", uid_, orderud);
		throw runtime_error("order_not_waiting");
	}
}

void DataOrderRoutine::GetPriceAndATime(unsigned orderud, int & cash, int & diffts, int &type)
{
	unsigned nowts = Time::GetGlobalTime();
	diffts = endts_ > nowts ? endts_ - nowts : 0;

	DataOrder & order = DataOrderManager::Instance()->GetData(uid_, orderud);

	const ConfigOrder::Base & base_cfg = OrderCfgWrap().GetOrderCfg();
	const ConfigOrder::Level & level_cfg = base_cfg.storages(order.storage_id - 1).levels(order.level_id -1);

	DataBase & sBase = BaseManager::Instance()->Get(uid_);
	//获取订单配置中，撕单的总价格
	cash = ceil(static_cast<double>(diffts)/base_cfg.parameters(sBase.level - 1).discard_cdtime() * base_cfg.discard_price());
}

void DataOrderRoutine::SingleRoutineEnd(unsigned orderud, ProtoPush::PushBuildingsCPP * msg)
{
	//debug_log("receive order time calback uid:%d slot:%d",uid_, orderud);
	DataOrder & order = DataOrderManager::Instance()->GetData(uid_, orderud);

	order.reset_time();
	DataOrderManager::Instance()->UpdateItem(order);
	LogicOrderManager::Instance()->FullMessage(uid_, order,msg->add_orders());

//	LogicQueueManager::Instance()->FinishQueue(uid_, orderud, routine_type_order);
}

void DataTruckRoutine::SingleRoutineEnd(unsigned slotud, ProtoPush::PushBuildingsCPP * msg)
{
	DataTruck & truck = DataTruckManager::Instance()->GetDataTruck(uid_);

	truck.reset_time();
	truck.state = Truck_Arrival;
	DataTruckManager::Instance()->UpdateTruck(truck);
	truck.SetMessage(msg->mutable_truck());

//	LogicQueueManager::Instance()->FinishQueue(uid_, slotud, routine_type_truck);
}


int LogicOrderManager::NewUser(unsigned uid)
{
	DataOrderManager::Instance()->Init(uid);

/*	for(int slot = 1; slot <= INIT_UNLOCK_SLOT_COUNT; slot++)
	{
		DataBase & sBase = BaseManager::Instance()->Get(uid);
		uint32_t storage_id = 0;
		uint32_t level_id = 0;
		uint32_t order_id = 0;
		uint32_t coin = 0;
		uint32_t exp = 0;
		GetRandomOrder(sBase.level,storage_id, level_id, order_id, coin, exp);
		DataOrder & sOrder = DataOrderManager::Instance()->AddNewOrder(uid, slot, storage_id, level_id, order_id);

		sOrder.coin = coin;
		sOrder.exp = exp;

		DataOrderManager::Instance()->UpdateItem(sOrder);
		debug_log("create a order uid:%d storage_id:%d level_id:%d order_id:%d",uid,storage_id, level_id, order_id);
	}
*/
	return 0;
}

int LogicOrderManager::CheckLogin(unsigned uid)
{
	DataOrderManager::Instance()->Init(uid);
	vector<unsigned> vOrders;
	DataOrderManager::Instance()->GetIndexs(uid, vOrders);
	map<uint32_t, set<unsigned> > mOrders;

	for(int i = 0; i < vOrders.size(); ++i)
	{
		DataOrder & sOrder = DataOrderManager::Instance()->GetDataByIndex(vOrders[i]);
		if (sOrder.end_ts > 0)
		{
			mOrders[sOrder.end_ts].insert(sOrder.id);
		}
	}

	for(map<unsigned, set<unsigned> >::iterator viter = mOrders.begin(); viter != mOrders.end(); ++viter)
	{
		LogicQueueManager::Instance()->JoinRoutine<DataOrderRoutine>(uid, viter->first, routine_type_order, viter->second);
	}

	DataTruckManager::Instance()->Init(uid);

	if(R_SUCCESS == DataTruckManager::Instance()->CheckBuff(uid))
	{
		DataTruck & truck = DataTruckManager::Instance()->Get(uid);

		if(truck.endts > 0)
		{
			LogicQueueManager::Instance()->JoinRoutine<DataTruckRoutine>(uid, truck.endts, routine_type_truck, truck.slot);
		}
	}

	return 0;
}

int LogicOrderManager::Process(unsigned uid, ProtoOrder::OrderQueryReq * req, ProtoOrder::OrderResp * resp)
{
	DataBase & sBase = BaseManager::Instance()->Get(uid);
	if(sBase.level < 2)
	{
		error_log("order level is not enough. uid=%u", uid);
		throw runtime_error("order_level_not_enough");
	}
	vector<unsigned> vOrders;
	DataOrderManager::Instance()->GetIndexs(uid, vOrders);
	uint32_t count = GetOrderCount(sBase.level);//1,2,3,4,5...
	if(vOrders.size() < count)
	{
		uint32_t slot_min = vOrders.size() + 1;

		for(uint32_t i = slot_min; i <= count; i++)
		{
			uint32_t storage_id = 0;
			uint32_t level_id = 0;
			char order_id[ORDER_LENGTH] = {0};
			uint32_t coin = 0;
			uint32_t exp = 0;
			GetRandomOrder(uid,i,sBase.level,storage_id, level_id, order_id, coin, exp);
			DataOrder & sOrder = DataOrderManager::Instance()->AddNewOrder(uid, i, storage_id, level_id, order_id);
			sOrder.coin = coin;
			sOrder.exp = exp;
			debug_log("create a order uid:%d storage_id:%d level_id:%d order_id:%d",uid,storage_id, level_id, order_id);
			DataOrderManager::Instance()->UpdateItem(sOrder);
		}
	}

	//订单配置改后台
	vector<unsigned> indexs;
	DataOrderManager::Instance()->GetIndexs(uid, indexs);
	for(size_t i = 0; i < indexs.size(); ++i)
	{
		DataOrder & order = DataOrderManager::Instance()->GetDataByIndex(indexs[i]);
		FullMessage(uid, order, resp->add_order());
	}

	return 0;
}

int LogicOrderManager::FullMessage(unsigned uid, DataOrder& order, ProtoOrder::OrderCPP *msg)
{
	unsigned storage_id = order.storage_id;
	unsigned level_id = order.level_id;


	const ConfigOrder::Base & base_cfg = OrderCfgWrap().GetOrderCfg();
	unsigned item_size = base_cfg.storages(storage_id - 1).levels(level_id - 1).orders_size();
	for(unsigned i = 0; i < item_size; i++)
	{
		int pos = i / CHAR_BITS;
		int right = i % CHAR_BITS;
		if(1 == ((order.order_id[pos] >> right) & 1))
		{
			unsigned propsid = base_cfg.storages(storage_id - 1).levels(level_id - 1).orders(i).propsid();
			int itemcnt = base_cfg.storages(storage_id - 1).levels(level_id - 1).orders(i).count();
			ProtoOrder::NeedCost * needcost = msg->add_needcost();
			needcost->set_id(propsid);
			needcost->set_count(-itemcnt);
		}
	}
	if(msg->needcost_size() == 0)	// 订单槽位没有物品
	{
		throw runtime_error("order_slot_not_exist_props");
	}

	//获取cd时间配置
	unsigned discard_cdtime = base_cfg.storages(storage_id -1).levels(level_id - 1).discard_cdtime();
	msg->set_discardcdtime(discard_cdtime);

	//添加基本信息
	order.SetMessage(msg);
	unsigned coin = msg->coin();
	unsigned exp  = msg->exp();

	//金币经验加成
	OrderBonus(uid,coin,exp);
	msg->set_coin(coin);
	msg->set_exp(exp);
	return 0;
}

int LogicOrderManager::FullLoginMessage(unsigned uid,User::User* reply)
{
	DataBase & sBase = BaseManager::Instance()->Get(uid);
	vector<unsigned> indexs;
	DataOrderManager::Instance()->GetIndexs(uid, indexs);
	for(size_t i = 0; i < indexs.size(); ++i)
	{
		DataOrder & order = DataOrderManager::Instance()->GetDataByIndex(indexs[i]);
		ProtoOrder::OrderCPP *msg = reply->add_orders();
		try{
			FullMessage(uid, order, msg);
		}
		catch(const std::exception& e)
		{
			error_log("load data error. uid=%u,reason=%s,storage_id=%u,level_id=%u", uid, e.what(),order.storage_id,order.level_id);
			GetRandomOrder(uid,order.id,sBase.level, order.storage_id, order.level_id, order.order_id, order.coin, order.exp);
			FullMessage(uid, order, msg);
			DataOrderManager::Instance()->UpdateItem(order);
//			throw e;
		}


	}
	return 0;
}

int LogicOrderManager::Process(unsigned uid, ProtoOrder::TruckQueryReq * req, ProtoOrder::TruckResp * resp)
{
	DataTruck & truck = DataTruckManager::Instance()->GetDataTruck(uid);
	truck.SetMessage(resp->mutable_truck());
	return 0;
}
int LogicOrderManager::Process(unsigned uid, ProtoOrder::RewardOrderReq * req, ProtoOrder::RewardOrderResp * resp)
{
	DataTruck & truck = DataTruckManager::Instance()->GetDataTruck(uid);

	if(truck.state != Truck_Arrival)
	{
		error_log("truck state invalid. uid=%u", uid);
		throw runtime_error("truck_state_invalid");
	}

	truck.state = Truck_Idle;
	truck.endts = 0;

	CommonGiftConfig::CommonModifyItem reward;
	reward.mutable_based()->set_exp(truck.exp);
	reward.mutable_based()->set_coin(truck.coin);

	LogicUserManager::Instance()->CommonProcess(uid, reward, "OrderReward", resp->mutable_commons());
	truck.SetMessage(resp->mutable_truck());
	resp->set_result(true);

	//将订单领取到的金币添加到任务系统中
	LogicTaskManager::Instance()->AddTaskData(uid,task_of_reward_order_coin,truck.coin);
	//debug_log("reward a order uid:%d storage_id:%d level_id:%d order_id:%d",uid,truck.storage_id, truck.level_id, truck.order_id);

	return 0;
}

int LogicOrderManager::Process(unsigned uid, ProtoOrder::StartOrderReq * req, ProtoOrder::StartOrderResp * resp)
{
	unsigned slot = req->slot();
	if(DataOrderManager::Instance()->IsExistItem(uid,slot) == false)
	{
		error_log("start_order_data_error,uid=%u,slot=%u",uid,slot);
		throw runtime_error("start_order_data_error");
	}
	DataOrder & sOrder = DataOrderManager::Instance()->GetData(uid, slot);

	//const ConfigOrder::Order & order = GetOrderConfig(sOrder.storage_id,sOrder.level_id,sOrder.order_id);

	DataBase & sBase = BaseManager::Instance()->Get(uid);

	DataTruck & truck = DataTruckManager::Instance()->GetDataTruck(uid);

	if(truck.state != Truck_Idle)
	{
		error_log("truck state invalid. uid=%u", uid);
		throw runtime_error("truck_state_invalid");
	}

	CommonGiftConfig::CommonModifyItem common;
	const ConfigOrder::Base & base_cfg = OrderCfgWrap().GetOrderCfg();
	unsigned item_size = base_cfg.storages(sOrder.storage_id - 1).levels(sOrder.level_id - 1).orders_size();
	for(unsigned i = 0; i < item_size; i++)
	{
		int pos = i / CHAR_BITS;
		int right = i % CHAR_BITS;
		if(1 == ((sOrder.order_id[pos] >> right) & 1))
		{
			CommonGiftConfig::PropsItem * propsbase = common.add_props();
			unsigned propsid = base_cfg.storages(sOrder.storage_id - 1).levels(sOrder.level_id - 1).orders(i).propsid();
			int itemcnt = base_cfg.storages(sOrder.storage_id - 1).levels(sOrder.level_id - 1).orders(i).count();
			propsbase->set_id(propsid);
			propsbase->set_count(itemcnt);
		}
	}

	LogicUserManager::Instance()->CommonProcess(uid,common,"start_order",resp->mutable_commons());

	truck.slot = sOrder.id;
	truck.storage_id = sOrder.storage_id;
	truck.level_id = sOrder.level_id;
	truck.order_id = 0;
	truck.coin = sOrder.coin;
	truck.exp = sOrder.exp;
	truck.state = Truck_Transport;
	truck.endts = Time::GetGlobalTime() + 20;

	//是否有加成
	OrderBonus(uid,truck.coin,truck.exp);

	//随机新订单
	GetRandomOrder(uid,sOrder.id,sBase.level, sOrder.storage_id, sOrder.level_id, sOrder.order_id, sOrder.coin, sOrder.exp);

	LogicQueueManager::Instance()->JoinRoutine<DataTruckRoutine>(uid, truck.endts, routine_type_truck, truck.slot);

	DataTruckManager::Instance()->UpdateTruck(truck);

	DataOrderManager::Instance()->UpdateItem(sOrder);

	//订单配置改后台
	FullMessage(uid, sOrder, resp->mutable_order());

	truck.SetMessage(resp->mutable_truck());
	resp->set_result(true);
	//debug_log("start a order uid:%d slot:%d storage_id:%d level_id:%d order_id:%d",uid,slot,sOrder.storage_id, sOrder.level_id, sOrder.order_id);

	//--------订单加成信息
	DataGameActivity& activity = DataGameActivityManager::Instance()->GetUserActivity(uid, actid);
	unsigned viewedAdCnt = activity.actdata[activiy_table_save_index_14];
	unsigned remainAdBonusCnt = activity.actdata[activiy_table_save_index_15];
	unsigned usedDiamondBonusCnt = activity.actdata[activiy_table_save_index_8];
	unsigned diamondBonusEndTs = activity.actdata[activiy_table_save_index_9];
	unsigned remain_cnt = 0;

	//VIP订单加成处理
	const ConfigVIP::VIPCfgInfo &vip_cfg = VIPCfgWrap().GetVIPInfoCfg();
	unsigned viplevel = LogicVIPManager::Instance()->GetVIPLevel(uid);
	if(viplevel > 0) {
		DataGameActivity& activity1 = DataGameActivityManager::Instance()->GetUserActivity(uid, userActId);

		if(activity1.actdata[e_Activity_UserData_1_index_1] < vip_cfg.vipinfo().viporderbonus().bonuscnt(viplevel - 1)) {
			activity1.actdata[e_Activity_UserData_1_index_1] += 1;
			unsigned reamin_vip_bonus_cnt = vip_cfg.vipinfo().viporderbonus().bonuscnt(viplevel - 1) - activity1.actdata[e_Activity_UserData_1_index_1];
			resp->mutable_vipbonus()->set_remainvipbonuscnt(reamin_vip_bonus_cnt);
		}
		DataGameActivityManager::Instance()->UpdateActivity(activity1);
	}

	//剩余广告加成次数减1
	if(activity.actdata[activiy_table_save_index_15] > 0)
		activity.actdata[activiy_table_save_index_15] -= 1;

	if(usedDiamondBonusCnt < base_cfg.orderbonus().diamond_bonus_cnt_daily() && diamondBonusEndTs >= Time::GetGlobalTime())
	{
		activity.actdata[activiy_table_save_index_8] += 1;
		remain_cnt = base_cfg.orderbonus().diamond_bonus_cnt_daily() - activity.actdata[activiy_table_save_index_8];
	}
	DataGameActivityManager::Instance()->UpdateActivity(activity);
	//设置返回
	resp->mutable_adbonus()->set_viewedadcnt(activity.actdata[activiy_table_save_index_14]);
	resp->mutable_adbonus()->set_remianadbonuscnt(activity.actdata[activiy_table_save_index_15]);
	resp->mutable_diamondbounus()->set_diamondbonusendts(diamondBonusEndTs);
	resp->mutable_diamondbounus()->set_remaindiamondbonuscnt(remain_cnt);

	//将发出的订单数添加到任务列表中
	LogicTaskManager::Instance()->AddTaskData(uid, task_of_start_cowcar, 1);
	LogicMissionManager::Instance()->AddMission(uid,mission_of_start_cowcar,1);
	LogicAllianceManager::Instance()->AddRaceOrderProgress(uid, alliance_race_task_of_start_cowcar, 1);
	//更新玩家发出牛车次数
	LogicMailDogManager::Instance()->UpdateMailDogData(uid,update_trunk_cnt_daily,1);
	UserManager::Instance()->OnFairySpeedUpOpen(uid);
	return 0;
}

int LogicOrderManager::Process(unsigned uid, ProtoOrder::DeleteOrderReq * req, ProtoOrder::DeleteOrderResp * resp)
{
	unsigned slot = req->slot();
	if(DataOrderManager::Instance()->IsExistItem(uid,slot) == false)
	{
		error_log("order_not_exsit,uid=%u,slot=%u",uid,slot);
		throw runtime_error("order_not_exsit");
	}
	DataOrder & sOrder = DataOrderManager::Instance()->GetData(uid, slot);
	if(sOrder.storage_id == 0 || sOrder.level_id == 0)
	{
		error_log("delete_order_data_error,uid=%u,storage_id=%u,level_id=%u",uid,sOrder.storage_id,sOrder.level_id);
		throw runtime_error("data_error");
	}

	if(sOrder.end_ts != 0)
	{
		error_log("order not idle. uid=%u,orderid=%u", uid, slot);
		throw runtime_error("order_not_idle");
	}

	const ConfigOrder::Base & base_cfg = OrderCfgWrap().GetOrderCfg();
	const ConfigOrder::Level & level_cfg = base_cfg.storages(sOrder.storage_id - 1).levels(sOrder.level_id -1);
	DataBase & sBase = BaseManager::Instance()->Get(uid);

	GetRandomOrder(uid,slot,sBase.level,sOrder.storage_id,sOrder.level_id, sOrder.order_id, sOrder.coin, sOrder.exp);
	sOrder.end_ts = Time::GetGlobalTime() + base_cfg.parameters(sBase.level - 1).discard_cdtime();
	//debug_log("delete a order uid:%d slot:%d storage_id:%d level_id:%d order_id:%d",uid,slot,sOrder.storage_id, sOrder.level_id, sOrder.order_id);

	LogicQueueManager::Instance()->JoinRoutine<DataOrderRoutine>(uid, sOrder.end_ts, routine_type_order, sOrder.id);
	resp->set_result(true);

	DataOrderManager::Instance()->UpdateItem(sOrder);

	//订单配置改后台,通过storage_id、level_id、order_id读取订单配置
	FullMessage(uid, sOrder, resp->mutable_order());

	return 0;
}

int LogicOrderManager::GetRandomStorageIndex(unsigned soltid)
{
	const ConfigOrder::Base & base_cfg = OrderCfgWrap().GetOrderCfg();

	//更改订单生成方式----------20180528--by-summer
	const ConfigOrder::OrderSolt & ordersolt_cfg = base_cfg.ordersolt(soltid - 1);
	unsigned pos = rand() % base_cfg.ordersolt(soltid - 1).storageid_size();
	return base_cfg.ordersolt(soltid - 1).storageid(pos) - 1;

	/*
	if(base_cfg.storages_size() == 0)
	{
		return -1;
	}
	int sum = 0;
	for(unsigned i = 0; i < base_cfg.storages_size(); i++)
	{
		sum += base_cfg.storages(i).ratio();
	}
	if(sum == 0)
	{
		return rand() % base_cfg.storages_size();
	}
	else
	{
		int randsum = rand() % sum;
		int tmpSum = 0;
		for(size_t pos = 0; pos < base_cfg.storages_size(); ++pos)
		{
			tmpSum += base_cfg.storages(pos).ratio();
			if(tmpSum >= randsum)
			{
				return pos;
			}
		}
		return -1;
	}
	*/
}
int LogicOrderManager::GetRandomOrder(unsigned uid,unsigned soltid,uint32_t level, uint32_t & storage_id, uint32_t & level_id, char order_id [], uint32_t & coin, uint32_t & exp)
{
	const ConfigOrder::Base & base_cfg = OrderCfgWrap().GetOrderCfg();
	int index = GetRandomStorageIndex(soltid);
	if(index < 0)
	{
		error_log("GetRandomOrder_index_error,index=%d",index);
		return 0;
	}
	unsigned level_index = level - 1;
	storage_id = base_cfg.storages(index).storage_id();
	level_id = base_cfg.storages(index).levels(level_index).level_id();

	GenerateOrderId(soltid,index,level_index,order_id);

	uint32_t coin_reward = 0;
	uint32_t exp_reward = 0;

	uint32_t min = base_cfg.parameters(level_index).random_min() * 10;
	uint32_t max = base_cfg.parameters(level_index).random_max() * 10;
	uint32_t rintp = 3;

	if(min < max)
	{
		rintp = rand() % (max - min) + min;
	}
	else
	{
		error_log("order config is error");
	}
	float ratio = (float)rintp / (float)10;

	//获取随机到物品奖励
	unsigned order_size = base_cfg.storages(index).levels(level_index).orders_size();
	for(unsigned i = 0; i < order_size; i++)
	{
		int pos = i / CHAR_BITS;
		int right = i % CHAR_BITS;
		int flag = (order_id[pos] >> right) & 1;
		if(flag == 1)
		{
			unsigned propsid = base_cfg.storages(index).levels(level_index).orders(i).propsid();
			int itemcnt = base_cfg.storages(index).levels(level_index).orders(i).count();
			const ConfigItem::PropItem & sProp = ItemCfgWrap().GetPropsItem(propsid);
			float fcoin = Math::Abs((int)sProp.price().based().coin());
			float fexp = Math::Abs((int)sProp.price().based().coin());  //原始经验=原始价格
			float Kcoin = 1,Kexp = 1;//生产设备加成系数
			BonusByEquipment(uid,propsid,Kcoin,Kexp);
			//float ratio = 3.6;
			float fcount = Math::Abs(itemcnt);  //数量
			float random_ratio = (float)( (float)(rand() % 7 + 2) / (float)10);  //0.2-0.8之间随机
			float coe = sProp.coefficient();  //产品系数
			coin_reward += (uint32_t)ceil( fcoin * ratio * fcount * random_ratio * coe *Kcoin * Kexp);
			exp_reward += (uint32_t)ceil(fexp * ratio * fcount * (1 - random_ratio) * coe *Kcoin *Kexp);

			//生产设备加成

		}
	}

	coin = coin_reward;
	exp = exp_reward;

	return 0;
}

int LogicOrderManager::GenerateOrderId(unsigned solt_id,unsigned storage_index,unsigned level_index,char order_id [])
{
	//获取配置信息
	const ConfigOrder::Base & base_cfg = OrderCfgWrap().GetOrderCfg();
	unsigned item_size = base_cfg.storages(storage_index).levels(level_index).orders_size();
	unsigned order_cnt_max = base_cfg.ordersolt(solt_id - 1).ordermax();

	//先随机出订单物品种类数目
	unsigned item_cnt = 0;
	if(item_size > order_cnt_max )
		item_cnt = rand() % order_cnt_max + 1;
	else
		item_cnt = rand() % item_size + 1;

	//-----------随机出物品
	vector<unsigned>propslist,weightlist;
	propslist.clear();
	weightlist.clear();

	//初始化propslist,weightlist
	for(int i = 0; i < item_size; i++)
	{
		propslist.push_back(base_cfg.storages(storage_index).levels(level_index).orders(i).propsid());
		weightlist.push_back(base_cfg.storages(storage_index).levels(level_index).orders(i).weight());
	}

	//按权重随机
	memset(order_id, 0, ORDER_LENGTH);
	for(int i = 1; i <= item_cnt; i++)
	{
		int target = 0;
		LogicCommonUtil::TurnLuckTable(weightlist, weightlist.size(), target);
		int pos = target / CHAR_BITS;
		int left = target % CHAR_BITS;
		order_id[pos] |= 1 << left;
		weightlist[target] = 0;//将权重赋值为0,可以保证下次按权重随机的时候，不会随到该物品
	}
	return 0;
}

int LogicOrderManager::GetOrderCount(uint32_t level)
{
	if(level < 2)
	{
		return 0;
	}
	else if(level >= 2 && level < 4)
	{
		return 1;
	}
	else if(level >= 4 && level < 6)
	{
		return 2;
	}
	else if(level >= 6 && level < 7)
	{
		return 3;
	}
	else if(level >= 7 && level < 8)
	{
		return 4;
	}
	else if(level >= 8 && level < 9)
	{
		return 5;
	}
	else if(level >= 9 && level < 12)
	{
		return 6;
	}
	else if(level >= 12 && level < 15)
	{
		return 7;
	}
	else if(level >= 15 && level < 18)
	{
		return 8;
	}
	else if(level >= 18 && level < 22)
	{
		return 9;
	}
	else
	{
		return 10;
	}
}


int LogicOrderManager::Process(unsigned uid, ProtoOrder::GetOrderBonusInfoReq * req, ProtoOrder::GetOrderBonusInfoResp * resp)
{
	//获取基本信息
	DataGameActivity& activity = DataGameActivityManager::Instance()->GetUserActivity(uid, actid);
	unsigned viewedAdCnt = activity.actdata[activiy_table_save_index_14];
	unsigned remainAdBonusCnt = activity.actdata[activiy_table_save_index_15];
	unsigned usedDiamondBonusCnt = activity.actdata[activiy_table_save_index_8];
	unsigned diamondBonusEndTs = activity.actdata[activiy_table_save_index_9];

	//设置返回
	const ConfigOrder::Base & base_cfg = OrderCfgWrap().GetOrderCfg();
	resp->mutable_adbonus()->set_viewedadcnt(viewedAdCnt);
	resp->mutable_adbonus()->set_remianadbonuscnt(remainAdBonusCnt);

	unsigned remainDiamondBonusCnt = 0;
	if(diamondBonusEndTs >= Time::GetGlobalTime()) {
		//证明已开通过钻石加成功能
		remainDiamondBonusCnt = base_cfg.orderbonus().diamond_bonus_cnt_daily() - usedDiamondBonusCnt;
	}
	resp->mutable_diamondbounus()->set_diamondbonusendts(diamondBonusEndTs);
	resp->mutable_diamondbounus()->set_remaindiamondbonuscnt(remainDiamondBonusCnt);

	//处理VIP特权
	const ConfigVIP::VIPCfgInfo &vip_cfg = VIPCfgWrap().GetVIPInfoCfg();
	unsigned viplevel = LogicVIPManager::Instance()->GetVIPLevel(uid);
	if(viplevel > 0)
	{
		DataGameActivity& activity1 = DataGameActivityManager::Instance()->GetUserActivity(uid, actid);
		unsigned reamin_vip_bonus_cnt = vip_cfg.vipinfo().viporderbonus().bonuscnt(viplevel - 1) - activity1.actdata[e_Activity_UserData_1_index_1];
		resp->mutable_vipbonus()->set_remainvipbonuscnt(reamin_vip_bonus_cnt);
	}
	return 0;
}

int LogicOrderManager::Process(unsigned uid, ProtoOrder::ViewAdReq * req, ProtoOrder::ViewAdResp * resp)
{
	DataGameActivity& activity = DataGameActivityManager::Instance()->GetUserActivity(uid, actid);
	const ConfigOrder::Base & base_cfg = OrderCfgWrap().GetOrderCfg();
	unsigned viewedAdCnt = activity.actdata[activiy_table_save_index_14];

	/*2018/09/10,按策划需求,取消掉"每日看广告的次数限制"
	if(viewedAdCnt >= base_cfg.orderbonus().ad_bonus_cnt_daily())
	{
		throw std::runtime_error("daily_view_ad_cnt_is_maxed");
	}
	*/

	//校验通过
	activity.actdata[activiy_table_save_index_14] += 1;
	activity.actdata[activiy_table_save_index_15] += 1;
	DataGameActivityManager::Instance()->UpdateActivity(activity);
	resp->mutable_adbonus()->set_viewedadcnt(activity.actdata[activiy_table_save_index_14]);
	resp->mutable_adbonus()->set_remianadbonuscnt(activity.actdata[activiy_table_save_index_15]);
	return 0;
}

int LogicOrderManager::Process(unsigned uid, ProtoOrder::BuyOrderBonusReq * req, ProtoOrder::BuyOrderBonusResp * resp)
{
	//获取配置
	const ConfigOrder::Base & base_cfg = OrderCfgWrap().GetOrderCfg();

	//扣钻
	int cost_diamond = base_cfg.orderbonus().cost_diamond();
	CommonGiftConfig::CommonModifyItem common;
	CommonGiftConfig::BaseItem *baseitem = common.mutable_based();
	baseitem->set_cash(-cost_diamond);
	LogicUserManager::Instance()->CommonProcess(uid,common,"CashOrderBonus",resp->mutable_commons());

	//更新数据
	DataGameActivity& activity = DataGameActivityManager::Instance()->GetUserActivity(uid, actid);
	unsigned usedDiamondBonusCnt = activity.actdata[activiy_table_save_index_8];
	unsigned diamondBonusEndTs = activity.actdata[activiy_table_save_index_9];
	activity.actdata[activiy_table_save_index_8] = 0;
	activity.actdata[activiy_table_save_index_9] = GetMorningTs() + base_cfg.orderbonus().diamond_bonus_day() * 24 * 3600;
	 DataGameActivityManager::Instance()->UpdateActivity(activity);

	//设置返回
	unsigned remain_cnt = base_cfg.orderbonus().diamond_bonus_cnt_daily() - activity.actdata[activiy_table_save_index_8];
	resp->mutable_diamondbounus()->set_diamondbonusendts(activity.actdata[activiy_table_save_index_9]);
	resp->mutable_diamondbounus()->set_remaindiamondbonuscnt(remain_cnt);
	return 0;
}

unsigned LogicOrderManager::GetMorningTs()
{
	//获取当前的整点ts
	unsigned cur_hour_ts = Time::GetGlobalTime();
	time_t nts = cur_hour_ts;
	struct tm ptm;
	if(localtime_r(&nts, &ptm) != NULL)
	{
		cur_hour_ts  -= ptm.tm_hour * 3600;
		cur_hour_ts  -= ptm.tm_min * 60;
		cur_hour_ts  -= ptm.tm_sec;
	}
	return cur_hour_ts;
}

unsigned LogicOrderManager::OrderAdBonus(unsigned uid,unsigned & coin, unsigned &exp)
{
	DataGameActivity& activity = DataGameActivityManager::Instance()->GetUserActivity(uid, actid);
	unsigned remainAdBonusCnt = activity.actdata[activiy_table_save_index_15];
	if(remainAdBonusCnt > 0)
	{
		const ConfigOrder::Base & base_cfg = OrderCfgWrap().GetOrderCfg();
		float percent = base_cfg.orderbonus().adbonus();
		coin = ceil(coin + coin * percent / 100);
		exp = ceil (exp + exp * percent / 100);
	}
	return 0;
}

unsigned LogicOrderManager::OrderDiamond(unsigned uid,unsigned & coin,unsigned & exp)
{
	DataGameActivity& activity = DataGameActivityManager::Instance()->GetUserActivity(uid, actid);
	unsigned diamondBonusEndTs = activity.actdata[activiy_table_save_index_9];
	unsigned usedDiamondBonusCnt = activity.actdata[activiy_table_save_index_8];

	const ConfigOrder::Base & base_cfg = OrderCfgWrap().GetOrderCfg();
	unsigned remainDiamondBonusCnt = base_cfg.orderbonus().diamond_bonus_cnt_daily() - usedDiamondBonusCnt;
	if(diamondBonusEndTs >= Time::GetGlobalTime() && remainDiamondBonusCnt > 0)
	{
		const ConfigOrder::Base & base_cfg = OrderCfgWrap().GetOrderCfg();
		float percent = base_cfg.orderbonus().diamondbonus();
		coin = ceil(coin + coin * percent / 100);
		exp = ceil (exp + exp * percent / 100);
	}
	return 0;
}

unsigned LogicOrderManager::ResetOrderBonusUsedCnt(unsigned uid)
{
	DataGameActivity& activity = DataGameActivityManager::Instance()->GetUserActivity(uid, actid);
	activity.actdata[activiy_table_save_index_8] = 0;
	activity.actdata[activiy_table_save_index_14] = 0;
	activity.actdata[activiy_table_save_index_15] = 0;
	DataGameActivityManager::Instance()->UpdateActivity(activity);

	unsigned viplevel = LogicVIPManager::Instance()->GetVIPLevel(uid);
	if(viplevel > 0) {
		DataGameActivity& activity1 = DataGameActivityManager::Instance()->GetUserActivity(uid, userActId);
		activity1.actdata[e_Activity_UserData_1_index_1] = 0;
		DataGameActivityManager::Instance()->UpdateActivity(activity1);
	}
	return 0;
}

unsigned LogicOrderManager::OrderBonus(unsigned uid,unsigned & coin,unsigned &exp)
{
	//订单双倍活动
	OrderActivity::Instance()->AddCoinAndExp(uid, coin, exp);

	//看广告加成
	OrderAdBonus(uid,coin,exp);

	//花钻加成
	OrderDiamond(uid,coin,exp);

	//VIP特权加成
	coin += ceil(coin * (LogicVIPManager::Instance()->VIPOrderBonus(uid) + LogicFriendWorkerManager::Instance()->GetOrderReardPercent(uid)));
	exp += ceil(exp * (LogicVIPManager::Instance()->VIPOrderBonus(uid) + LogicFriendWorkerManager::Instance()->GetOrderReardPercent(uid)));
}

unsigned LogicOrderManager::BonusByEquipment(unsigned uid,unsigned propsid,float &Kcoin,float &Kexp)
{
	const ConfigItem::PropItem itemcfg = ItemCfgWrap().GetPropsItem(propsid);

	//根据物品id是否需要生产材料来判定其是否为生产设备
	if(!itemcfg.has_material() || !itemcfg.has_equipment())
		return 0;
	unsigned equipmentId = itemcfg.equipment();
	int type = BuildCfgWrap().GetBuildType(equipmentId);

	if(build_type_produce_equipment == type)
	{
		if (DataEquipmentStarManager::Instance()->IsExistItem(uid, equipmentId))
		{
			BuildCfgWrap buildcfgwrap;
			const ConfigBuilding::ProduceEquipment & producecfg = buildcfgwrap.GetProduceCfgById(equipmentId);
			DataEquipmentStar & datastar = DataEquipmentStarManager::Instance()->GetData(uid, equipmentId);
			if(datastar.star == 1)
			{
				//设备星级为1,金币加成
				Kcoin = (float)(100 + producecfg.upgrade_star(datastar.star - 1).value()) / 100;
				Kexp = 1;
			}
			else if(datastar.star == 2)
			{
				//设备星级为2,经验加成
				Kexp = (float)(100 + producecfg.upgrade_star(datastar.star - 1).value()) / 100;
				Kcoin = 1;
			}
		}
	}
	return 0;
}

