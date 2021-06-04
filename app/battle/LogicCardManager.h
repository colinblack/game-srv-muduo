/*
 * LogicRotaryTable.h
 *
 *  Created on: 2018年6月19日
 *      Author: summer
 */

#ifndef APP_BATTLE_LOGICCARD_H_
#define APP_BATTLE_LOGICCARD_H_

class LogicCardManager : public BattleSingleton, public CSingleton<LogicCardManager>
{
private:
	friend class CSingleton<LogicCardManager>;
	LogicCardManager() {actid = e_Activity_FriendlyTree;}
	int actid;
public:
	enum{
		month_card_end_ts_index = 9,//月卡结束ts
	};

	void CallDestroy() {Destroy();}

	//获取玩家月卡终生卡信息
	int Process(unsigned uid, ProtoCard::GetCardReq* req, ProtoCard::GetCardResp* resp);

	//领取月卡奖励
	int Process(unsigned uid, ProtoCard::RewardMonthCardReq* req, ProtoCard::RewardMonthCardResp* resp);

	//领取终生卡奖励
	int Process(unsigned uid, ProtoCard::RewardLifeCardReq* req, ProtoCard::RewardLifeCardResp* resp);

	//处理金币购买
	int HandleCardPurchase(unsigned uid,unsigned itemid);

	//每日重置月卡与终生卡的领取标记
	int ResetCardReward(unsigned uid);

private:
	//获取当天凌晨ts
	unsigned GetCurDateStartTs();
};

#endif /* APP_BATTLE_LOGICCARD_H_ */
