#ifndef APP_BATTLE_LOGIC_RECHARGE_4399_ACTIVITY_H_
#define APP_BATTLE_LOGIC_RECHARGE_4399_ACTIVITY_H_

/*
 * 4399平台 首次奖励翻倍活动
 */

#include "ServerInc.h"

class Recharge4399ActivityManager :public ActivitySingleton<e_Activity_4399_Recharge>, public CSingleton<Recharge4399ActivityManager>
{
private:
	friend class CSingleton<Recharge4399ActivityManager>;
	Recharge4399ActivityManager() {actid = e_Activity_4399_Recharge;}
	virtual ~Recharge4399ActivityManager() {}
	int actid;

public:

	void CallDestroy() {Destroy();}

	//登陆返回改活动数据
	int SetMessage(unsigned uid, ProtoActivity::GameAcitivityAllCPP * msg);

	//领取奖励
	int Process(unsigned uid, ProtoActivity::Reward4399RechargeGiftReq* req, ProtoActivity::Reward4399RechargeGiftResp* resp);

	//处理充值
	int OnCharge(unsigned uid,unsigned cash);

private:
	//校验版本
	void CheckVersion(DataGameActivity & activity);
	//重置活动数据
	void ResetActivity(DataGameActivity & activity);
	//设置消息返回
	int FullMessage(const DataGameActivity & activity,ProtoActivity::GameAcitivityCPP *msg);

};

#endif
