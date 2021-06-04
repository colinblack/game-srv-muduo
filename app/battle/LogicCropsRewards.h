/*
 * LogicCropsRewards.h
 *
 *  Created on: 2018年5月25日
 *      Author: colin
 */

#ifndef APP_BATTLE_LOGICCROPSREWARDS_H_
#define APP_BATTLE_LOGICCROPSREWARDS_H_

#include "ServerInc.h"
class CropsActivity : public ActivitySingleton<e_Activity_Crops>, public CSingleton<CropsActivity>
{
private:
	friend class CSingleton<CropsActivity>;
	CropsActivity() {actid = e_Activity_Crops;}
	int actid;

public:
	void CallDestroy() {Destroy();}
	int GetExtraCropNumber(unsigned uid);
	int ProcessRedPoint(unsigned uid, unsigned index); //小红点
	int SetMessage(unsigned uid, ProtoActivity::GameAcitivityAllCPP * msg);
	int LoginCheck(unsigned uid);

private:
	void SetMessage(DataGameActivity & activity, ProtoActivity::GameAcitivityCPP* msg);
	void CheckVersion(DataGameActivity & activity);
	//重置活动数据
	void ResetActivity(DataGameActivity & activity);

};


#endif /* APP_BATTLE_LOGICCROPSREWARDS_H_ */
