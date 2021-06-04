/*
 * LogicDailyShareRewards.h
 *
 *  Created on: 2018年5月24日
 *      Author: colin
 */

#ifndef APP_BATTLE_LOGICDAILYSHAREREWARDS_H_
#define APP_BATTLE_LOGICDAILYSHAREREWARDS_H_

#include "ServerInc.h"


class DailyShareActivity :public ActivitySingleton<e_Activity_DailyShare>, public CSingleton<DailyShareActivity>
{
private:
	friend class CSingleton<DailyShareActivity>;
	DailyShareActivity() {actid = e_Activity_DailyShare; reward_id = 1;}
	virtual ~DailyShareActivity() {}
	int actid;
	int reward_id; //奖励id

public:
	void CallDestroy() {Destroy();}
	int SetMessage(unsigned uid, ProtoActivity::GameAcitivityAllCPP * msg);
	int LoginCheck(unsigned uid);
	//分享奖励
	int Process(unsigned uid, User::ShareRewardsReq* req, User::ShareRewardsResp* resp);
	int Process(unsigned uid, User::ShareTotalRewardsReq* req, User::ShareTotalRewardsResp* resp);
	int Process(unsigned uid, User::DaliyShareReq* req, User::DaliyShareResp* resp);
	int ProcessRedPoint(unsigned uid, unsigned index); //小红点

private:
	void SetMessage(DataGameActivity & activity, ProtoActivity::GameAcitivityCPP* msg);
	void CheckVersion(DataGameActivity & activity);
	//重置活动数据
	void ResetActivity(DataGameActivity & activity);
	void GetRewardId(DataGameActivity & activity);

};







#endif /* APP_BATTLE_LOGICDAILYSHAREREWARDS_H_ */
