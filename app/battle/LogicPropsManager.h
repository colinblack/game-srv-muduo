#ifndef LOGIC_PROPS_MANAGER_H
#define LOGIC_PROPS_MANAGER_H

#include "Common.h"
#include "Kernel.h"
#include <bitset>
#include "DataInc.h"
#include "ConfigInc.h"

class LogicPropsManager :public BattleSingleton, public CSingleton<LogicPropsManager>
{
private:
	friend class CSingleton<LogicPropsManager>;
	LogicPropsManager(){}

public:
	virtual void CallDestroy() { Destroy();}

	//花钻购买材料
	int Process(unsigned uid, User::BuyMaterialReq* req, User::BuyMaterialResp* resp);

	int NewUser(unsigned uid);

	int CheckLogin(unsigned uid);

	//消耗道具
	int CostProps(unsigned uid, unsigned propsud, unsigned propsid, unsigned count, string reason, DataCommon::PropsAllChangeCPP * msg);

	//添加道具
	int AddProps(unsigned uid, unsigned propsid, unsigned count, string reason, DataCommon::PropsAllChangeCPP * msg);

	//获取当前粮仓的已用容量
	int GetStorageUsedSpace(unsigned uid, unsigned type) const;

	//是否可叠加
	bool IsAllowOverlay(unsigned propsid);

	//购买加速卡
	int Process(unsigned uid, User::ViewAdGetSpeedUpCardReq* req, User::ViewAdGetSpeedUpCardResp* resp);

private:
	int AddPropsImpl(DataItem & props, string & reason, DataCommon::PropsAllChangeCPP * msg);
};


#endif //LOGIC_PROPS_MANAGER_H
