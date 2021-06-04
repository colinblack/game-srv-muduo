#ifndef LOGIC_RECHARGE_ACTIVITY_H_
#define LOGIC_RECHARGE_ACTIVITY_H_

#include "ServerInc.h"



class RechargeActivity : public ActivitySingleton<e_Activity_Recharge>, public CSingleton<RechargeActivity>
{
private:
	friend class CSingleton<RechargeActivity>;
	RechargeActivity(){actid = e_Activity_Recharge;}
	virtual ~RechargeActivity(){}
	int actid;
public:
	void CallDestroy() {Destroy();}

	int AddDoubleCash(unsigned uid, unsigned cash);

	int SetMessage(unsigned uid, ProtoActivity::GameAcitivityAllCPP * msg);
	int ProcessRedPoint(unsigned uid, unsigned index); //小红点
	int LoginCheck(unsigned uid);

private:
	void SetMessage(DataGameActivity & activity, ProtoActivity::GameAcitivityCPP* msg);
	void CheckVersion(DataGameActivity & activity);
	//重置活动数据
	void ResetActivity(DataGameActivity & activity);
};

#endif /*LOGIC_RECHARGE_ACTIVITY_H_*/
