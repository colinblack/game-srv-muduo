/*
 * LogicOrderManager.h
 *
 *  Created on: 2018-3-1
 *      Author: Administrator
 */

#ifndef LOGICORDERMANAGER_H_
#define LOGICORDERMANAGER_H_

#include "Common.h"
#include "Kernel.h"
#include "DataInc.h"
#include "ConfigInc.h"
#include "LogicQueueManager.h"

//订单定时任务
class DataOrderRoutine : public DataRoutineBase
{
public:
	DataOrderRoutine(unsigned uid, unsigned endts, unsigned ud):DataRoutineBase(uid, endts, ud)
	{

	}

	//检查订单id
	virtual void CheckUd(unsigned orderud);

	//获取需要扣的钻石数以及剩余时间
	virtual void GetPriceAndATime(unsigned buildud, int & cash, int & diffts, int &type);

	virtual void SingleRoutineEnd(unsigned orderud, ProtoPush::PushBuildingsCPP * msg);
};

//卡车定时任务
class DataTruckRoutine : public DataRoutineBase
{
public:
	DataTruckRoutine(unsigned uid, unsigned endts, unsigned ud):DataRoutineBase(uid, endts, ud)
	{

	}

	virtual void SingleRoutineEnd(unsigned slotud, ProtoPush::PushBuildingsCPP * msg);
};

class LogicOrderManager : public BattleSingleton, public CSingleton<LogicOrderManager>
{
	enum{
		activiy_table_save_index_6   = 5,//存放广告状态
		activiy_table_save_index_7   = 6,//下一次可观看广告的ts
		activiy_table_save_index_8   = 7,//当天已使用钻石加成次数
		activiy_table_save_index_9   = 8,//钻石特权结束ts

		activiy_table_save_index_14   = 13,//当天已看广告次数
		activiy_table_save_index_15   = 14,//当日剩余广告加成次数

		e_Activity_UserData_1_index_1 = 1,//VIP每日订单加成的使用的次数
	};
private:
	friend class CSingleton<LogicOrderManager>;
	LogicOrderManager(){actid = e_Activity_FriendlyTree;userActId = e_Activity_UserData_1;}
	int actid;
	int userActId;
public:
	virtual void CallDestroy() { Destroy();}
	int NewUser(unsigned uid);
	int CheckLogin(unsigned uid);
	int Process(unsigned uid, ProtoOrder::OrderQueryReq * req, ProtoOrder::OrderResp * resp);
	int Process(unsigned uid, ProtoOrder::TruckQueryReq * req, ProtoOrder::TruckResp * resp);
	int Process(unsigned uid, ProtoOrder::StartOrderReq * req, ProtoOrder::StartOrderResp * resp);
	int Process(unsigned uid, ProtoOrder::DeleteOrderReq * req, ProtoOrder::DeleteOrderResp * resp);
	int Process(unsigned uid, ProtoOrder::RewardOrderReq * req, ProtoOrder::RewardOrderResp * resp);
	int FullMessage(unsigned uid, DataOrder& order, ProtoOrder::OrderCPP *msg);
	int FullLoginMessage(unsigned uid,User::User* reply);

	//----新增订单加成功能
	//获取订单加成信息
	int Process(unsigned uid, ProtoOrder::GetOrderBonusInfoReq * req, ProtoOrder::GetOrderBonusInfoResp * resp);

	//看广告
	int Process(unsigned uid, ProtoOrder::ViewAdReq * req, ProtoOrder::ViewAdResp * resp);

	//花钻购买订单加成
	int Process(unsigned uid, ProtoOrder::BuyOrderBonusReq * req, ProtoOrder::BuyOrderBonusResp * resp);

	//每日重置钻石加成使用次数
	unsigned ResetOrderBonusUsedCnt(unsigned uid);
private:
	int GetRandomOrder(unsigned uid,unsigned soltid,uint32_t level, uint32_t & storage_id, uint32_t & level_id, char order_id [], uint32_t & coin, uint32_t & exp);
	int GetOrderCount(uint32_t level);
	int GetRandomStorageIndex(unsigned soltid);
	int GenerateOrderId(unsigned solt_id,unsigned storage_index,unsigned level_index,char order_id []);

	//获取的当天凌晨ts
	unsigned GetMorningTs();

	//广告加成
	unsigned OrderAdBonus(unsigned uid,unsigned & coin, unsigned &exp);

	//钻石加成
	unsigned OrderDiamond(unsigned uid,unsigned & coin,unsigned & exp);

	//订单加成(返回所有加成后的金币经验奖励)
	unsigned OrderBonus(unsigned uid,unsigned & coin,unsigned &exp);

	//生产设备升星加成
	unsigned BonusByEquipment(unsigned uid,unsigned propsid,float &Kcoin,float &Kexp);
};

#endif /* LOGICORDERMANAGER_H_ */
