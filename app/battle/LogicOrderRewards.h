/*
 * LogicOrderRewards.h
 *
 *  Created on: 2018年5月25日
 *      Author: colin
 */

#ifndef APP_BATTLE_LOGICORDERREWARDS_H_
#define APP_BATTLE_LOGICORDERREWARDS_H_
#include "ServerInc.h"

class OrderActivity: public ActivitySingleton<e_Activity_Order>, public CSingleton<OrderActivity>
{
private:
	friend class CSingleton<OrderActivity>;
	OrderActivity() {actid = e_Activity_Order;}
	int actid;
public:
	void CallDestroy() {Destroy();}
	void AddCoinAndExp(unsigned uid, unsigned& coins, unsigned& exp);
	int ProcessRedPoint(unsigned uid, unsigned index); //小红点
	int SetMessage(unsigned uid, ProtoActivity::GameAcitivityAllCPP * msg);
	int LoginCheck(unsigned uid);

private:
	void SetMessage(DataGameActivity & activity, ProtoActivity::GameAcitivityCPP* msg);
	void CheckVersion(DataGameActivity & activity);
	//重置活动数据
	void ResetActivity(DataGameActivity & activity);
};


#endif /* APP_BATTLE_LOGICORDERREWARDS_H_ */
