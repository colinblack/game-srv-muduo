#ifndef APP_BATTLE_LOGIC_DAILY_4399_ACTIVITY_H_
#define APP_BATTLE_LOGIC_DAILY_4399_ACTIVITY_H_

/*
 * 4399平台 每日充值活动
 */

#include "ServerInc.h"

class Daily4399ActivityManager :public ActivitySingleton<e_Activity_4399_Daily>, public CSingleton<Daily4399ActivityManager>
{
private:
	friend class CSingleton<Daily4399ActivityManager>;
	Daily4399ActivityManager() {actid = e_Activity_4399_Daily;}
	virtual ~Daily4399ActivityManager() {}
	int actid;

public:

	enum{
			e_card_month = 1,
			e_card_life  = 2,
	};

	void CallDestroy() {Destroy();}

	//登陆返回改活动数据
	int SetMessage(unsigned uid, ProtoActivity::GameAcitivityAllCPP * msg);

	//领取奖励
	int Process(unsigned uid, ProtoActivity::Reward4399DailyGiftReq* req, ProtoActivity::Reward4399DailyGiftResp* resp);

	//使用月卡令或终身卡令
	int Process(unsigned uid, ProtoActivity::UseCardReq* req, ProtoActivity::UseCardResp* resp);

	//处理充值
	int OnCharge(unsigned uid,unsigned cash);

	//跨天登录重置活动数据
	void CheckLogin(unsigned uid, unsigned last_offline_time);

	//每日0点重置活动数据
	void CheckDaily();

private:

	//活动数据是否为空
	bool IsDartyActData(DataGameActivity & activity);

	//校验版本
	void CheckVersion(DataGameActivity & activity);
	//重置活动数据
	void ResetActivity(DataGameActivity & activity);
	//设置消息返回
	int FullMessage(const DataGameActivity & activity,ProtoActivity::GameAcitivityCPP *msg);

};

#endif
