#ifndef APP_BATTLE_LOGICFUNDACTIVITY_H_
#define APP_BATTLE_LOGICFUNDACTIVITY_H_

#include "ServerInc.h"

class LogicFundActivityManager :public ActivitySingleton<e_Activity_Fund>, public CSingleton<LogicFundActivityManager>
{
private:
	friend class CSingleton<LogicFundActivityManager>;
	LogicFundActivityManager() {actid = e_Activity_Fund;}
	virtual ~LogicFundActivityManager() {}
	int actid;

public:
	enum{
		index_of_buy_ts     = 0,//购买基金ts
		index_of_data       = 1,
	};

	void CallDestroy() {Destroy();}

	//登陆返回改活动数据
	int SetMessage(unsigned uid, ProtoActivity::GameAcitivityAllCPP * msg);

	//购买基金
	int Process(unsigned uid, ProtoActivity::FundPurchaseReq* req, ProtoActivity::FundPurchaseResp* resp);

	//领取基金奖励
	int Process(unsigned uid, ProtoActivity::RewardFundGiftReq* req, ProtoActivity::RewardFundGiftResp* resp);


private:
	//校验版本
	void CheckVersion(DataGameActivity & activity);
	//重置活动数据
	void ResetActivity(DataGameActivity & activity);
	//自动发放未领取的礼包(重置活动时)
	int AutoReward(DataGameActivity & activity);
	//设置消息返回
	int FullMessage(const DataGameActivity & activity,ProtoActivity::GameAcitivityCPP *msg);

};

#endif /* APP_BATTLE_LOGICFUNDACTIVITY_H_ */
