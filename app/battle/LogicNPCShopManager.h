#ifndef LOGIC_NPCSHOP_MANAGER_H
#define LOGIC_NPCSHOP_MANAGER_H

#include "Common.h"
#include "Kernel.h"
#include <bitset>
#include "DataInc.h"
#include "ConfigInc.h"

class LogicNPCShopManager :public BattleSingleton, public CSingleton<LogicNPCShopManager>
{
private:
	friend class CSingleton<LogicNPCShopManager>;
	LogicNPCShopManager(){}

public:
	virtual void CallDestroy() { Destroy();}

	//初始商店信息
	int NewUser(unsigned uid);

	//登录校验
	int CheckLogin(unsigned uid);

	//访问NPC
	int Process(unsigned uid, ProtoNPCUser::RequestNPCUser* req, ProtoNPCUser::NPCUser* resp);

	//查询NPC商店信息
	int Process(unsigned uid, ProtoNPCUser::GetNPCShopReq* req, ProtoNPCUser::GetNPCShopResp* resp);

	//购买
	int Process(unsigned uid, ProtoNPCUser::PurchaseReq* req, ProtoNPCUser::PurchaseResp* resp);

private:
	//生成随机数
	int GetRandom(int start,int end);

};


#endif //LOGIC_NPCSHOP_MANAGER_H
