/*
 * LogicActivityTencent.h
 *
 *  Created on: 2018年6月19日
 *      Author: summer
 */

#ifndef APP_BATTLE_LOGICACTIVITYTENCENT_H_
#define APP_BATTLE_LOGICACTIVITYTENCENT_H_

#include "ServerInc.h"

class LogicActivityTencent : public BattleSingleton, public CSingleton<LogicActivityTencent>
{
private:
	friend class CSingleton<LogicActivityTencent>;
	LogicActivityTencent() {actid = e_Activity_Tencent;}
	int actid;
public:
	enum{
		reward_daily_status        = 0,//每日奖励领取状态
		reward_grow_blue_status   = 1,//蓝钻成长奖励
		reward_grow_privilege_status   = 2,//特权礼包成长奖励

		reward_daily_status_id_blue = 0, // 每日奖励领取状态蓝钻ID
		reward_daily_status_id_privilege = 1, // 每日奖励领取状态特权ID
	};

	void CallDestroy() {Destroy();}

	//获取奖励状态
	int Process(unsigned uid,ProtoActivityTencent::RewardStatusReq *req,ProtoActivityTencent::RewardStatusResp *resp);

	//蓝钻每日奖励
	int Process(unsigned uid,ProtoActivityTencent::GetBlueDailyAward *req, ProtoActivityTencent::GetRewardResp *resp);

	//蓝钻成长奖励
	int Process(unsigned uid,ProtoActivityTencent::GetBlueGrowAward *req, ProtoActivityTencent::GetRewardResp *resp);

	//特权每日奖励
	int Process(unsigned uid,ProtoActivityTencent::GetQQgamePrivilegeDailyAward *req, ProtoActivityTencent::GetRewardResp *resp);

	//特权成长奖励
	int Process(unsigned uid,ProtoActivityTencent::GetQQgamePrivilegeGrowAward *req, ProtoActivityTencent::GetRewardResp *resp);

public:
	//重置活动数据
	void ResetActivity(unsigned uid);

private:
	int FillActivityStatus(DataGameActivity& activity, ProtoActivityTencent::RewardStatusResp *resp);
	int getBit(uint32_t flag, uint32_t offset);
	int setBit(uint32_t& flag, uint32_t offset);
};

#endif /* APP_BATTLE_LOGICACTIVITYTENCENT_H_ */
