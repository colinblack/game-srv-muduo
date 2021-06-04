#ifndef LOGIC_FRIENDWORKER_MANAGER_H
#define LOGIC_FRIENDWORKER_MANAGER_H

#include "Common.h"
#include "Kernel.h"
#include <bitset>
#include "DataInc.h"
#include "ConfigInc.h"

class LogicFriendWorkerManager :public BattleSingleton, public CSingleton<LogicFriendWorkerManager>
{
	enum{
		result_set_friendworker_success = 0,
		result_set_friendworker_repeated = 1,
		result_set_friendworker_error = 2,

		pos_of_crops_speedup_slot = 1,//农田加速
		pos_of_order_reward_slot = 2,//订单奖励
		pos_of_animal_speedup_slot = 3,//动物加速
		pos_of_ship_reward_slot = 4,//航运奖励
		pos_of_product_speedup_slot = 5,//设备加速
	};
private:
	friend class CSingleton<LogicFriendWorkerManager>;
	LogicFriendWorkerManager(){};

public:
	virtual void CallDestroy() { Destroy();}

	//设置好友工人
	int Process(unsigned uid, ProtoFriendWorker::SetFriendWorkerReq* req);

	//跨服设置长工
	int Process(ProtoFriendWorker::CSSetFriendWorkerReq* req);

	//处理跨服设置长工信息
	int Process(ProtoFriendWorker::CSSetFriendWorkerResp* req);

	//获取长工信息
	int Process(unsigned uid, ProtoFriendWorker::GetWorkerSpeedUpReq* req, ProtoFriendWorker::GetWorkerSpeedUpResp* resp);

	//选择长工加速
	int Process(unsigned uid, ProtoFriendWorker::SelectWorkerReq* req, ProtoFriendWorker::SelectWorkerResp* resp);

	//感谢长工
	int Process(unsigned uid, ProtoFriendWorker::ThanksWorkerReq* req, ProtoFriendWorker::ThanksWorkerResp* resp);

	//长工农田加速
	float GetCropsSpeedUpPercent(unsigned uid);

	//长工订单加速
	float GetOrderReardPercent(unsigned uid);

	//动物加速
	float GetAnimalSpeedUpPercent(unsigned uid);

	//航运加速
	float GetShipRewardPercent(unsigned uid);

	//设备加速
	float GetProductSpeedUpPercent(unsigned uid);
private:

	//添加好友长工
	int AddFriendWorker(unsigned uid, unsigned inviteduid,unsigned propsid = 0,unsigned sourceType = 0,unsigned sourceFlag = 0);
};


#endif //LOGIC_FRIENDWORKER_MANAGER_H
