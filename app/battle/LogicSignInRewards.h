/*
 * LogicSignInRewards.h
 *
 *  Created on: 2018年5月25日
 *      Author: colin
 */

#ifndef APP_BATTLE_LOGICSIGNINREWARDS_H_
#define APP_BATTLE_LOGICSIGNINREWARDS_H_

#include "ServerInc.h"

class SignInActivity : public ActivitySingleton<e_Activity_SignIn>, public CSingleton<SignInActivity>
{
private:
	friend class CSingleton<SignInActivity>;
	SignInActivity() {actid = e_Activity_SignIn;}
	int actid;
public:
	void CallDestroy() {Destroy();}
	int SetMessage(unsigned uid, ProtoActivity::GameAcitivityAllCPP * msg);
	int Process(unsigned uid, User::SignInRewardsReq* req, User::SignInRewardsResp* resp);
	void CountLoginDaysInActiveity(unsigned uid);
	int ProcessRedPoint(unsigned uid, unsigned index); //小红点
	int LoginCheck(unsigned uid);
private:
	void SetMessage(DataGameActivity & activity, ProtoActivity::GameAcitivityCPP* msg);
	void CheckVersion(DataGameActivity & activity);
	//重置活动数据
	void ResetActivity(DataGameActivity & activity);
};

#endif /* APP_BATTLE_LOGICSIGNINREWARDS_H_ */
